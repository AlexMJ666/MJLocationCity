/*
* File:   list.h
* Author: Fcten
*
* Created on 2015年2月15日, 上午9:24
*/

#ifndef __LIST_H__
#define	__LIST_H__

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct list_s {
	struct list_s *next, *prev;
} list_t;

/**
* 用于通过 list_t 指针获得完整结构体的指针
* @ptr:                list_t 结构体指针
* @type:               想要获取的完整结构体类型
* @member:         list_t 成员在完整结构体中的名称
*/
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
* 用于 list_t 初始化，必须先声明再调用
* @ptr:     list_t 结构体指针
*/
#define list_init(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
	} while (0)

/* 对 list_t 结构体进行遍历 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/* 获取链表的第一个元素 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/* 获取链表的最后一个元素 */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/* 获取链表的下一个元素 */
#define list_next_entry(pos, type, member) \
	list_entry((pos)->member.next, type, member)

/* 获取链表的上一个元素 */
#define list_prev_entry(pos, type, member) \
	list_entry((pos)->member.prev, type, member)

/**
* 对 list_t 结构体进行遍历并返回完整结构体的指针
* @pos:                用于遍历的完整结构体临时指针
* @head:               链表头中的 list_t 指针
* @member:         list_t 成员在完整结构体中的名称
*/
#define list_for_each_entry(pos, type, head, member)			\
	for (pos = list_first_entry(head, type, member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, type, member))

static inline void __list_add(list_t *list, list_t *prev, list_t *next) {
	next->prev = list;
	list->next = next;
	list->prev = prev;
	prev->next = list;
}

static inline void __list_del(list_t *prev, list_t *next) {
	next->prev = prev;
	prev->next = next;
}

static inline void list_add(list_t *list, list_t *head) {
	__list_add(list, head, head->next);
}

static inline void list_add_tail(list_t *list, list_t *head) {
	__list_add(list, head->prev, head);
}

static inline void list_del(list_t *entry) {
	__list_del(entry->prev, entry->next);
}

static inline int list_empty(const list_t *head) {
	return head->next == head;
}

static inline void list_move_tail(list_t *list, list_t *head) {
	list_del(list);
	list_add_tail(list, head);
}

#ifdef	__cplusplus
}
#endif

#endif	/* __LIST_H__ */

