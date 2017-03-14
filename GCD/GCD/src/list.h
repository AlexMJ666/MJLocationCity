/*
* File:   list.h
* Author: Fcten
*
* Created on 2015��2��15��, ����9:24
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
* ����ͨ�� list_t ָ���������ṹ���ָ��
* @ptr:                list_t �ṹ��ָ��
* @type:               ��Ҫ��ȡ�������ṹ������
* @member:         list_t ��Ա�������ṹ���е�����
*/
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
* ���� list_t ��ʼ���������������ٵ���
* @ptr:     list_t �ṹ��ָ��
*/
#define list_init(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
	} while (0)

/* �� list_t �ṹ����б��� */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/* ��ȡ����ĵ�һ��Ԫ�� */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/* ��ȡ��������һ��Ԫ�� */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/* ��ȡ�������һ��Ԫ�� */
#define list_next_entry(pos, type, member) \
	list_entry((pos)->member.next, type, member)

/* ��ȡ�������һ��Ԫ�� */
#define list_prev_entry(pos, type, member) \
	list_entry((pos)->member.prev, type, member)

/**
* �� list_t �ṹ����б��������������ṹ���ָ��
* @pos:                ���ڱ����������ṹ����ʱָ��
* @head:               ����ͷ�е� list_t ָ��
* @member:         list_t ��Ա�������ṹ���е�����
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

