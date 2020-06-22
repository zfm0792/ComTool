
#include <stdio.h>
#include <assert.h>
#define __list_c__

#include "list.h"

struct _my_list _inner_list = {
	list_is_empty,
	list_init,
	list_insert_head,
	list_insert_tail,
	list_remove_head,
	list_remove_tail,
	list_remove
};

struct _my_list* list = &_inner_list;

// 判断双向链表是否为空
// 返回值: 非零表示为空   零表示不空
int list_is_empty(list_s* phead)
{
	assert(phead != NULL);
	return phead->next == phead;
}

// 初始化双向链表
void list_init(list_s* phead)
{
	assert(phead != NULL);
	// 前后指针都指向自己
	phead->next = phead;
	phead->prior = phead;
}
// 从双向链表头部插入节点
void list_insert_head(list_s* phead, list_s* plist)
{
	assert(phead != NULL && plist != NULL);
	// 得到第一个节点的指针
	list_s *first = phead->next;
	// 
	phead->next = plist;
	plist->prior = phead;
	plist->next = first;
	first->prior = plist;
}
// 从双向链表尾部插入节点
void list_insert_tail(list_s* phead, list_s* plist)
{
	// 得到最后一个节点
	list_s* last = phead->prior;
	//
	last->next = plist;
	plist->next = phead;
	plist->prior = last;
	phead->prior = plist;
}
// 双向链表移除头部节点
// 返回被移除的节点
list_s* list_remove_head(list_s* phead)
{
	list_s* second = NULL;
	list_s* removed = NULL;
	if (list_is_empty(phead))
		return NULL;
	// 第二个节点
	second = phead->next->next;
	// 移除的节点
	removed = phead->next;

	second->prior = phead;
	phead->next = second;

	return removed;
}
// 双向链表移除尾部节点
// 返回被移除的节点
list_s* list_remove_tail(list_s* phead)
{
	list_s* second_last = NULL;
	list_s* removed = NULL;
	if (list_is_empty(phead))
		return NULL;
	// 第二个节点
	second_last = phead->prior->prior;
	// 移除的节点
	removed = phead->prior;

	second_last->next = phead;
	phead->prior = second_last;
	return removed;
}

// 移除节点指针为p的节点  并不释放节点内存
int list_remove(list_s* phead, list_s* p)
{
	if (!list_is_empty(phead)) {
		// 非空
		list_s* node = NULL;
		// 遍历找到节点p
		for (node = phead->next; node != phead; node = node->next) {
			if (node == p) {
				// 移除节点
				node->prior->next = node->next;
				node->next->prior = node->prior;
				return 1; // 删除节点成功
			}
		}
		return 0; // 删除节点不存在
	}
	else
	{
		return 2;// 链表空
	}
}

#if 0
#define array_size(Array) (sizeof(Array) / sizeof(Array[0]))
// 调试使用
typedef struct
{
	int num;
	list_s list_entry;
}my_data;

int main(int argc, char** argv)
{
	list_s list_head;
	my_data md[16];
	int it1, it2;
	struct {
		void(*insert)(list_s*, list_s*);
		list_s *(*remove)(list_s*);
		char* msg;
	}func_ptr[] = {
		{list->insert_head,list->remove_head,"头头"},
		{list->insert_head,list->remove_tail,"头尾"},
		{list->insert_tail,list->remove_head,"尾头"},
		{list->insert_tail,list->remove_tail,"尾尾"}
	};

	for (it2 = 0; it2 < array_size(func_ptr); it2++) {
		printf("\n测试第%d种方式,方式:%s:\n", it2 + 1, func_ptr[it2].msg);
		//必须进行初始化操作
		list->init(&list_head);
		//测试数据插入双向链表,采用2种方式之一
		for (it1 = 0; it1 < array_size(md); it1++) {
			md[it1].num = it1;
			func_ptr[it2].insert(&list_head, &md[it1].list_entry);
		}
		//测试数据脱离双向链表,采用2种方式之一
		while (!list->is_empty(&list_head)) {
			//得到被移除的链表结点指针
			list_s* plist = func_ptr[it2].remove(&list_head);
			//通过链表结点指针得到包含该链表结构的结构体的指针
			my_data* pmd = list_data(plist, my_data, list_entry);
			//输出相关数据,验证正确性
			printf("md[0x%08X].num = %d\n", pmd, pmd->num);
		}
	}
}
#endif