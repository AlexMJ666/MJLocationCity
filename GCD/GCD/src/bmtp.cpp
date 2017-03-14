// CBMTP.cpp : Defines the exported functions for the DLL application.
//

#define BMTP_EXPORTS

#include "stdafx.h"
#include "list.h"
#include "bmtp.h"

#define BMTP_CONN    0x10
#define BMTP_CONNACK 0x20
#define BMTP_PUB     0x30
#define BMTP_PUBACK  0x40
#define BMTP_SUB     0x50
#define BMTP_SUBACK  0x60
#define BMTP_PING    0xC0
#define BMTP_PINGACK 0xD0
#define BMTP_DISCONN 0xE0

#define bmtp_cmd(b)     (b&240)
#define bmtp_version(b) (b&15)
#define bmtp_status(b)  (b&15)
#define bmtp_dup(b)     (b&8)
#define bmtp_qos(b)     (b&3)

#define BMTP_QOS_AT_MOST_ONCE  0
#define BMTP_QOS_AT_LEAST_ONCE 1
#define BMTP_QOS_EXACTLY_ONCE  2
#define BMTP_DUP               8

// This is an example of an exported variable
//BMTP_API int nBMTP=0;

// This is an example of an exported function.
//BMTP_API int fnBMTP(void)
//{
//    return 42;
//}

// This is the constructor of a class that has been exported.
// see BMTP.h for the class definition
//CBMTP::CBMTP()
//{
//    return;
//}

#ifdef WIN32

#define bmtp_errno                  GetLastError()
#define bmtp_set_errno(err)         SetLastError(err)
#define bmtp_socket_errno           WSAGetLastError()
#define bmtp_set_socket_errno(err)  WSASetLastError(err)

#define BMTP_EAGAIN                 WSAEWOULDBLOCK

#else

#define bmtp_errno                  errno
#define bmtp_set_errno(err)         errno = err
#define bmtp_socket_errno           errno
#define bmtp_set_socket_errno(err)  errno = err

#if (__hpux__)
#define BMTP_EAGAIN        EWOULDBLOCK
#else
#define BMTP_EAGAIN        EAGAIN
#endif

#endif

enum {
    STATE_CONNECTED,
    STATE_RECV_HEADER,
    STATE_RECV_SID,
    STATE_RECV_CID,
    STATE_RECV_PAYLOAD_LENGTH,
    STATE_RECV_PAYLOAD
};

void bmtp_close(struct bmtp_context *bmtp);
void bmtp_reconnect(struct bmtp_context *bmtp);

int bmtp_send_conn(struct bmtp_context *bmtp);
int bmtp_send_puback(struct bmtp_context *bmtp, unsigned char sid);
int bmtp_send_ping(struct bmtp_context *bmtp);
int bmtp_send_disconn(struct bmtp_context *bmtp);
int bmtp_send_pub(struct bmtp_context *bmtp, unsigned char *msg, int len, unsigned char qos, unsigned char dup);

void bmtp_on_open( BMTP * ) {}
void bmtp_on_close( BMTP * ) {}
void bmtp_on_error( BMTP *, int err_no ) {}
void bmtp_on_message( BMTP *, const char* data, int len ) {}

struct bmtp_msg {
    list_t head;

    unsigned int  len;
    unsigned int  offset;
    unsigned char *msg;
};

struct bmtp_context {
    struct event_base *base;
    struct event *event_write;
    struct event *event_read;
    struct sockaddr_in server_addr;

    unsigned int  recv_length;
    unsigned int  recv_offset;
    unsigned char *recv_buf;
    
    unsigned int  state;
    unsigned char header;
    unsigned char sid;
    unsigned long long int cid;
    unsigned int  payload_length;
    unsigned char *payload;
    
    unsigned int  last_sid : 8;
    unsigned int  usable_sids : 8;
    unsigned int  page[8];

    struct bmtp_msg wait_queue;
    struct bmtp_msg send_queue;
    struct bmtp_msg ack_queue;

    void( *on_open )( BMTP* );
    void( *on_close )( BMTP* );
    void( *on_error )( BMTP*, int );
    void( *on_message )( BMTP*, const char*, int );

    // 用于 send_queue 和 wait_queue 加锁
    // ack_queue 不需要加锁，因为只有单线程访问
#ifdef WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t cs;
#endif
};

#ifdef WIN32
void bmtp_mutex_init( struct bmtp_context *bmtp ) {
    InitializeCriticalSection( &bmtp->cs );
}

void bmtp_mutex_lock( struct bmtp_context *bmtp ) {
    EnterCriticalSection( &bmtp->cs );
}

void bmtp_mutex_unlock( struct bmtp_context *bmtp ) {
    LeaveCriticalSection( &bmtp->cs );
}
#else
void bmtp_mutex_init( struct bmtp_context *bmtp ) {
    pthread_mutex_init( &bmtp->cs, NULL );
}

void bmtp_mutex_lock( struct bmtp_context *bmtp ) {
    pthread_mutex_lock( &bmtp->cs );
}

void bmtp_mutex_unlock( struct bmtp_context *bmtp ) {
    pthread_mutex_unlock( &bmtp->cs );
}
#endif

// 分配一个流标识符 (1-255)
// 如果分配失败， 返回 0
int bmtp_sid_alloc(struct bmtp_context *bmtp) {
    if (!bmtp->usable_sids) {
        return 0;
    }

    unsigned char offset = bmtp->last_sid + 1;

    unsigned int *addr = bmtp->page;
    unsigned int *p;
    unsigned int mask;

    while (1) {
        p = addr + (offset >> 5);
        mask = 1U << (offset & 31);

        if (!((*p) & mask)) {
            *p |= mask;
            bmtp->usable_sids--;
            bmtp->last_sid = offset;
            return offset;
        }

        while (!++offset);
    }

    return 0;
}

// 释放一个流标识符，使其可以被重新分配
void bmtp_sid_free(struct bmtp_context *bmtp, unsigned int sid) {
    unsigned char offset = sid;

    unsigned int *addr = bmtp->page;
    unsigned int *p = addr + (offset >> 5);
    unsigned int mask = 1U << (offset & 31);

    *p &= ~mask;

    bmtp->usable_sids++;
}

int bmtp_on_connack(struct bmtp_context *bmtp) {
    bmtp->on_open( bmtp );
    return 0;
}

int bmtp_on_pub(struct bmtp_context *bmtp) {

    if (bmtp_qos(bmtp->header) > 0) {
        bmtp_send_puback(bmtp, bmtp->sid);
    }

    bmtp->on_message(bmtp, (char *)bmtp->payload, bmtp->payload_length);

    return 0;
}

int bmtp_on_puback(struct bmtp_context *bmtp) {
    // 搜索 ack_queue
    struct bmtp_msg *msg_node;
    list_for_each_entry(msg_node, struct bmtp_msg, &bmtp->ack_queue.head, head) {
        if (bmtp_cmd(msg_node->msg[0]) == BMTP_PUB) {
            if (msg_node->msg[1] == bmtp->sid) {
                list_del(&msg_node->head);
                free(msg_node->msg);
                free(msg_node);
                break;
            }
        }
    }

    bmtp_mutex_lock( bmtp );

    if (list_empty(&bmtp->wait_queue.head)) {
        bmtp_sid_free(bmtp, bmtp->sid);
    }
    else {
        msg_node = list_first_entry(&bmtp->wait_queue.head, struct bmtp_msg, head);
        msg_node->msg[1] = bmtp->sid;
        list_del(&msg_node->head);
        list_add_tail(&msg_node->head, &bmtp->send_queue.head);

        struct timeval tv = { 15, 0 };
        event_add(bmtp->event_write, &tv);
    }

    bmtp_mutex_unlock( bmtp );

    return 0;
}

int bmtp_on_suback(struct bmtp_context *bmtp) {
    // 搜索 ack_queue
    struct bmtp_msg *msg_node;
    unsigned long long int cid;
    list_for_each_entry(msg_node, struct bmtp_msg, &bmtp->ack_queue.head, head) {
        if (bmtp_cmd(msg_node->msg[0]) == BMTP_SUB) {
            cid = (msg_node->msg[1] << 24) + (msg_node->msg[2] << 16) + (msg_node->msg[3] << 8) + msg_node->msg[4];
            if (cid == bmtp->cid) {
                list_del(&msg_node->head);
                free(msg_node->msg);
                free(msg_node);
                break;
            }
        }
    }

    return 0;
}

int bmtp_on_pingack(struct bmtp_context *bmtp) {
    // nothing to do
    return 0;
}

int bmtp_add_to_send_queue(struct bmtp_context *bmtp, const unsigned char * msg, int len) {
    struct bmtp_msg *msg_node = (struct bmtp_msg *)calloc(1, sizeof(struct bmtp_msg));
    if (!msg_node) {
        bmtp->on_error( bmtp, BMTP_OUT_OF_MEMORY );
        return 1;
    }

    msg_node->msg =(unsigned char *)malloc(len);
    if (!msg_node->msg) {
        free(msg_node);
        bmtp->on_error( bmtp, BMTP_OUT_OF_MEMORY );
        return 1;
    }
    msg_node->len = len;
    memcpy(msg_node->msg, msg, len);

    bmtp_mutex_lock( bmtp );

    if( bmtp_cmd( msg[0] ) == BMTP_PUB &&
        bmtp_qos( msg[0] ) > 0 &&
        msg[1] == 0 ) {
        list_add_tail(&msg_node->head, &bmtp->wait_queue.head);
    } else {
        if( bmtp_cmd( msg[0] ) == BMTP_CONN ) {
            list_add( &msg_node->head, &bmtp->send_queue.head );
        } else {
            list_add_tail( &msg_node->head, &bmtp->send_queue.head );
        }

        if( bmtp->event_write ) {
            struct timeval tv = { 15, 0 };
            event_add( bmtp->event_write, &tv );
        }
    }

    bmtp_mutex_unlock( bmtp );

    return 0;
}

int bmtp_send_conn(struct bmtp_context *bmtp) {
    unsigned char conn[] = { BMTP_CONN + 0x1, 'B', 'M', 'T', 'P' };

    return bmtp_add_to_send_queue(bmtp, conn, sizeof(conn));
}

int bmtp_send_puback(struct bmtp_context *bmtp, unsigned char sid) {
    unsigned char puback[] = { BMTP_PUBACK, sid };

    return bmtp_add_to_send_queue(bmtp, puback, sizeof(puback));
}

int bmtp_send_ping(struct bmtp_context *bmtp) {
    unsigned char ping[] = { BMTP_PING };

    return bmtp_add_to_send_queue(bmtp, ping, sizeof(ping));
}

int bmtp_send_disconn(struct bmtp_context *bmtp) {
    unsigned char disconn[] = { BMTP_DISCONN };

    return bmtp_add_to_send_queue(bmtp, disconn, sizeof(disconn));
}

int bmtp_send_sub(struct bmtp_context *bmtp, unsigned long long int channel_id) {
    unsigned char sub[] = { BMTP_SUB, static_cast<unsigned char>(channel_id >> 24), static_cast<unsigned char>(channel_id >> 16), static_cast<unsigned char>(channel_id >> 8), static_cast<unsigned char>(channel_id) };

    return bmtp_add_to_send_queue(bmtp, sub, sizeof(sub));
}

int bmtp_send_pub(struct bmtp_context *bmtp, unsigned char *msg, int len, unsigned char qos, unsigned char dup) {
    if (qos > 0) {
        int sid = bmtp_sid_alloc(bmtp);

        if (len > 64 * 1024 - 4) {
            len = 64 * 1024 - 4;
        }

        unsigned char pub[64 * 1024] = { static_cast<unsigned char>(BMTP_PUB + qos + dup), static_cast<unsigned char>(sid), static_cast<unsigned char>(len >> 8), static_cast<unsigned char>(len)
        };
        memcpy(pub + 4, msg, len);
        return bmtp_add_to_send_queue(bmtp, pub, len + 4);
    }
    else {
        if (len > 64 * 1024 - 3) {
            len = 64 * 1024 - 3;
        }

        unsigned char pub[64 * 1024] = { static_cast<unsigned char>(BMTP_PUB + qos + dup), static_cast<unsigned char>(len >> 8), static_cast<unsigned char>(len) };
        memcpy(pub + 3, msg, len);
        return bmtp_add_to_send_queue(bmtp, pub, len + 3);
    }
}

void bmtp_on_recv(struct bmtp_context *bmtp) {
    while (1) {
        switch (bmtp->state) {
        case STATE_CONNECTED:
            bmtp->sid = 0;
            bmtp->payload = NULL;
            bmtp->payload_length = 0;
            bmtp->state = STATE_RECV_HEADER;
            break;
        case STATE_RECV_HEADER:
            if (bmtp->recv_offset + 1 > bmtp->recv_length) {
                return;
            }

            bmtp->header = bmtp->recv_buf[bmtp->recv_offset++];

            switch (bmtp_cmd(bmtp->header)) {
            case BMTP_CONNACK:
            case BMTP_PINGACK:
                bmtp->state = STATE_RECV_PAYLOAD;
                break;
            case BMTP_PUB:
                if (bmtp_qos(bmtp->header) == 0) {
                    bmtp->state = STATE_RECV_PAYLOAD_LENGTH;
                }
                else {
                    bmtp->state = STATE_RECV_SID;
                }
                break;
            case BMTP_PUBACK:
                bmtp->state = STATE_RECV_SID;
                break;
            case BMTP_SUBACK:
                bmtp->state = STATE_RECV_CID;
                break;
            default:
                bmtp->on_error( bmtp, 1/*数据流异常*/ );
                return;
            }
            break;
        case STATE_RECV_SID:
            if (bmtp->recv_offset + 1 > bmtp->recv_length) {
                return;
            }

            switch (bmtp_cmd(bmtp->header)) {
            case BMTP_PUB:
                bmtp->sid = bmtp->recv_buf[bmtp->recv_offset++];
                bmtp->state = STATE_RECV_PAYLOAD_LENGTH;
                break;
            case BMTP_PUBACK:
                bmtp->sid = bmtp->recv_buf[bmtp->recv_offset++];
                bmtp->state = STATE_RECV_PAYLOAD;
                break;
            default:
                bmtp->on_error( bmtp, 1/*数据流异常*/ );
                return;
            }
            break;
        case STATE_RECV_CID:
            if (bmtp->recv_offset + 4 > bmtp->recv_length) {
                return;
            }

            bmtp->cid  = bmtp->recv_buf[bmtp->recv_offset++] << 24;
            bmtp->cid += bmtp->recv_buf[bmtp->recv_offset++] << 16;
            bmtp->cid += bmtp->recv_buf[bmtp->recv_offset++] << 8;
            bmtp->cid += bmtp->recv_buf[bmtp->recv_offset++];
            bmtp->state = STATE_RECV_PAYLOAD;
            break;
        case STATE_RECV_PAYLOAD_LENGTH:
            if (bmtp->recv_offset + 2 > bmtp->recv_length) {
                return;
            }

            bmtp->payload_length  = bmtp->recv_buf[bmtp->recv_offset++] << 8;
            bmtp->payload_length += bmtp->recv_buf[bmtp->recv_offset++];
            bmtp->state = STATE_RECV_PAYLOAD;
            break;
        case STATE_RECV_PAYLOAD:
            if (bmtp->recv_offset + bmtp->payload_length > bmtp->recv_length) {
                return;
            }

            bmtp->payload = bmtp->recv_buf + bmtp->recv_offset;
            bmtp->recv_offset += bmtp->payload_length;

            switch (bmtp_cmd(bmtp->header)) {
            case BMTP_CONNACK:
                if (bmtp_on_connack(bmtp) != 0) {
                    bmtp->on_error( bmtp, 1/*on_connack异常*/ );
                    return;
                }
                break;
            case BMTP_PUB:
                if (bmtp_on_pub(bmtp) != 0) {
                    bmtp->on_error( bmtp, 1/*on_pub异常*/ );
                    return;
                }
                break;
            case BMTP_PUBACK:
                if (bmtp_on_puback(bmtp) != 0) {
                    bmtp->on_error( bmtp, 1/*on_puback异常*/ );
                    return;
                }
                break;
            case BMTP_SUBACK:
                if (bmtp_on_suback(bmtp) != 0) {
                    bmtp->on_error( bmtp, 1/*on_suback异常*/ );
                    return;
                }
                break;
            case BMTP_PINGACK:
                if (bmtp_on_pingack(bmtp) != 0) {
                    bmtp->on_error( bmtp, 1/*on_pingack异常*/ );
                    return;
                }
                break;
            default:
                bmtp->on_error( bmtp, 1/*数据流异常*/ );
                return;
            }

            if (bmtp->recv_offset >= 4096) {
                memcpy(bmtp->recv_buf, bmtp->recv_buf + bmtp->recv_offset, bmtp->recv_length - bmtp->recv_offset);
                bmtp->recv_buf = (unsigned char *)realloc(bmtp->recv_buf, bmtp->recv_length - bmtp->recv_offset);
                bmtp->recv_length -= bmtp->recv_offset;
                bmtp->recv_offset = 0;
            }

            bmtp->state = STATE_CONNECTED;
            break;
        default:
            bmtp->on_error( bmtp, 1/*无法识别的状态*/ );
            return;
        }
    }
}

void bmtp_write_cb(evutil_socket_t sock, short flags, void * args) {
    struct bmtp_context *bmtp = (struct bmtp_context *)args;

    switch (flags) {
    case EV_TIMEOUT:
        break;
    case EV_WRITE:
        while (!list_empty(&bmtp->send_queue.head)) {
            struct bmtp_msg *msg_node = list_first_entry(&bmtp->send_queue.head, struct bmtp_msg, head);
            int ret = send(sock, (char *)msg_node->msg + msg_node->offset, msg_node->len - msg_node->offset, 0);
            if (ret == 0) {
                /*connection closed*/
                msg_node->offset = 0;
                break;
            }
            else if (ret < 0) {
                if (bmtp_socket_errno == BMTP_EAGAIN) {
                    struct timeval tv = { 15, 0 };
                    event_add(bmtp->event_write, &tv);
                }
                break;
            }
            else {
                if (ret + msg_node->offset >= msg_node->len) {
                    bmtp_mutex_lock( bmtp );
                    list_del(&msg_node->head);
                    bmtp_mutex_unlock( bmtp );
                    if ((bmtp_cmd(msg_node->msg[0]) == BMTP_PUB && bmtp_qos(msg_node->msg[0]) > 0) ||
                        (bmtp_cmd(msg_node->msg[0]) == BMTP_SUB)) {
                        list_add_tail(&msg_node->head, &bmtp->ack_queue.head);
                    }
                    else {
                        free(msg_node->msg);
                        free(msg_node);
                    }
                }
                else {
                    msg_node->offset += ret;
                }
            }
        }
        break;
    }
}

void bmtp_read_cb(evutil_socket_t sock, short flags, void * args) {
    struct bmtp_context *bmtp = (struct bmtp_context *)args;
    char recv_buf[10240];
    struct timeval tv = { 30, 0 };
    int ret;

    switch (flags) {
    case EV_TIMEOUT:
        bmtp_send_ping(bmtp);
        event_add(bmtp->event_read, &tv);
        break;
    case EV_READ:
        while (1) {
            ret = recv(sock, recv_buf, 10240, 0);
            if (ret == 0) {
                /*connection closed*/
                break;
            }
            else if (ret < 0) {
                if (bmtp_socket_errno == BMTP_EAGAIN) {
                    event_add(bmtp->event_read, &tv);
                }
                break;
            }
            else {
                void *p = realloc(bmtp->recv_buf, bmtp->recv_length + ret);
                if (p == NULL) {
                    bmtp->on_error( bmtp, BMTP_OUT_OF_MEMORY );
                    break;
                }
                else {
                    bmtp->recv_buf = (unsigned char *)p;
                    memcpy(bmtp->recv_buf + bmtp->recv_length, recv_buf, ret);
                    bmtp->recv_length += ret;

                    bmtp_on_recv(bmtp);
                }
            }
        }
        break;
    }
}

#ifdef WIN32
unsigned int __stdcall bmtp_main_thread(LPVOID lpParam) {
#else
void* bmtp_main_thread( void* lpParam ) {
#endif
    struct bmtp_context *bmtp = (struct bmtp_context*)lpParam;

    event_base_dispatch(bmtp->base);

    bmtp_close(bmtp);

#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    bmtp_reconnect(bmtp);

    return 0;
}

void bmtp_reconnect(struct bmtp_context *bmtp) {
#ifdef WIN32
    WSADATA wsaData;
    while (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        Sleep(1000);
    }
#endif
    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    while (connect(sock, (struct sockaddr*)&bmtp->server_addr, sizeof(bmtp->server_addr)) == -1) {
#ifdef WIN32
        Sleep(3000);
#else
        sleep(3);
#endif
    }
    evutil_make_socket_nonblocking(sock);

    bmtp->base = event_base_new();
    bmtp->event_write = event_new(bmtp->base, sock, EV_WRITE, bmtp_write_cb, (void*)bmtp);
    bmtp->event_read = event_new(bmtp->base, sock, EV_READ, bmtp_read_cb, (void*)bmtp);

    //list_init(&bmtp->wait_queue.head);
    //list_init(&bmtp->send_queue.head);
    //list_init(&bmtp->ack_queue.head);

    bmtp_open(bmtp);
}


void bmtp_close(struct bmtp_context *bmtp) {
    if (bmtp->recv_buf) {
        free(bmtp->recv_buf);
        bmtp->recv_buf = NULL;
        bmtp->recv_length = 0;
        bmtp->recv_offset = 0;
    }

    if (bmtp->event_read) {
        event_del(bmtp->event_read);
        bmtp->event_read = NULL;
    }

    if (bmtp->event_write) {
        event_del(bmtp->event_write);
        bmtp->event_write = NULL;
    }

    if (bmtp->base) {
        event_base_free(bmtp->base);
        bmtp->base = NULL;
    }

    // 把 ack_queue 中的消息全部放到 send_queue 末尾
    while (!list_empty(&bmtp->ack_queue.head)) {
        struct bmtp_msg *msg_node = list_first_entry(&bmtp->ack_queue.head, struct bmtp_msg, head);
        list_del(&msg_node->head);
        list_add_tail(&msg_node->head, &bmtp->send_queue.head);
    }

    bmtp->on_close( bmtp );
}

BMTP_API int bmtp_init() {
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
        return 1;
    }
#endif

    return 0;
}

BMTP_API void bmtp_cleanup() {
#ifdef WIN32
    WSACleanup();
#endif
}

BMTP_API BMTP* bmtp_new(const char *ip, int port) {
    struct bmtp_context *bmtp = (struct bmtp_context*)calloc(1, sizeof(struct bmtp_context));

    bmtp->server_addr.sin_family = AF_INET;
    bmtp->server_addr.sin_port = htons(port);
#ifdef WIN32
    bmtp->server_addr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
    bmtp->server_addr.sin_addr.s_addr = inet_addr(ip);
#endif
    memset(bmtp->server_addr.sin_zero, 0x00, 8);

    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect(sock, (struct sockaddr*)&bmtp->server_addr, sizeof(bmtp->server_addr)) == -1) {
        return NULL;
    }
    evutil_make_socket_nonblocking(sock);

#ifdef WIN32
    evthread_use_windows_threads();
#else  
    evthread_use_pthreads();
#endif

    bmtp->base = event_base_new();
    bmtp->event_write = event_new(bmtp->base, sock, EV_TIMEOUT | EV_WRITE, bmtp_write_cb, (void*)bmtp);
    bmtp->event_read = event_new(bmtp->base, sock, EV_TIMEOUT | EV_READ, bmtp_read_cb, (void*)bmtp);
    
    bmtp->on_close   = bmtp_on_close;
    bmtp->on_error   = bmtp_on_error;
    bmtp->on_open    = bmtp_on_open;
    bmtp->on_message = bmtp_on_message;

    bmtp->usable_sids = 255;

    list_init(&bmtp->wait_queue.head);
    list_init(&bmtp->send_queue.head);
    list_init(&bmtp->ack_queue.head);

    bmtp_mutex_init( bmtp );

    return bmtp;
}

BMTP_API int bmtp_set_on_open( BMTP *p, void( *on_open )( BMTP * ) ) {
    struct bmtp_context *bmtp = (struct bmtp_context*)p;

    bmtp->on_open = on_open;

    return 0;
}

BMTP_API int bmtp_set_on_close( BMTP *p, void( *on_close )( BMTP * ) ) {
    struct bmtp_context *bmtp = (struct bmtp_context*)p;

    bmtp->on_close = on_close;

    return 0;
}

BMTP_API int bmtp_set_on_error( BMTP *p, void( *on_error )( BMTP *, int err_no ) ) {
    struct bmtp_context *bmtp = (struct bmtp_context*)p;

    bmtp->on_error = on_error;

    return 0;
}

BMTP_API int bmtp_set_on_message( BMTP *p, void( *on_message )( BMTP *, const char *data, int len ) ) {
    struct bmtp_context *bmtp = (struct bmtp_context*)p;

    bmtp->on_message = on_message;

    return 0;
}

BMTP_API int bmtp_open(BMTP *p) {
    struct bmtp_context *bmtp = (struct bmtp_context*)p;

    bmtp_send_conn(bmtp);

    struct timeval tv = { 15, 0 };
    event_add(bmtp->event_write, &tv);
    tv.tv_sec = 30;
    event_add(bmtp->event_read, &tv);

#ifdef WIN32
    _beginthreadex(NULL, 0, bmtp_main_thread, LPVOID(bmtp), 0, NULL);
#else
    pthread_t tid;
    pthread_create(&tid, 0, bmtp_main_thread, bmtp);
#endif

    return 0;
}

BMTP_API int bmtp_pub(BMTP *p, const char *data, int len, int qos) {
    struct bmtp_context *bmtp = (struct bmtp_context*)p;

    return bmtp_send_pub( bmtp, (unsigned char *)data, len, qos, 0 );
}

BMTP_API int bmtp_sub(BMTP*p, unsigned long long int channel_id) {
    struct bmtp_context *bmtp = (struct bmtp_context*)p;

    return bmtp_send_sub(bmtp, channel_id);
}

BMTP_API int bmtp_debug( BMTP*p ) {
    struct bmtp_context *bmtp = ( struct bmtp_context* )p;

    printf( "%d %d\n", bmtp->recv_length, bmtp->recv_offset );

    unsigned int i = 0;
    while( i < bmtp->recv_length ) {
        printf( "%x ", bmtp->recv_buf[i] );
        i++;
    }
    return 0;
}