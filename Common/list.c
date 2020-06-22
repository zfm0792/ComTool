
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

// �ж�˫�������Ƿ�Ϊ��
// ����ֵ: �����ʾΪ��   ���ʾ����
int list_is_empty(list_s* phead)
{
	assert(phead != NULL);
	return phead->next == phead;
}

// ��ʼ��˫������
void list_init(list_s* phead)
{
	assert(phead != NULL);
	// ǰ��ָ�붼ָ���Լ�
	phead->next = phead;
	phead->prior = phead;
}
// ��˫������ͷ������ڵ�
void list_insert_head(list_s* phead, list_s* plist)
{
	assert(phead != NULL && plist != NULL);
	// �õ���һ���ڵ��ָ��
	list_s *first = phead->next;
	// 
	phead->next = plist;
	plist->prior = phead;
	plist->next = first;
	first->prior = plist;
}
// ��˫������β������ڵ�
void list_insert_tail(list_s* phead, list_s* plist)
{
	// �õ����һ���ڵ�
	list_s* last = phead->prior;
	//
	last->next = plist;
	plist->next = phead;
	plist->prior = last;
	phead->prior = plist;
}
// ˫�������Ƴ�ͷ���ڵ�
// ���ر��Ƴ��Ľڵ�
list_s* list_remove_head(list_s* phead)
{
	list_s* second = NULL;
	list_s* removed = NULL;
	if (list_is_empty(phead))
		return NULL;
	// �ڶ����ڵ�
	second = phead->next->next;
	// �Ƴ��Ľڵ�
	removed = phead->next;

	second->prior = phead;
	phead->next = second;

	return removed;
}
// ˫�������Ƴ�β���ڵ�
// ���ر��Ƴ��Ľڵ�
list_s* list_remove_tail(list_s* phead)
{
	list_s* second_last = NULL;
	list_s* removed = NULL;
	if (list_is_empty(phead))
		return NULL;
	// �ڶ����ڵ�
	second_last = phead->prior->prior;
	// �Ƴ��Ľڵ�
	removed = phead->prior;

	second_last->next = phead;
	phead->prior = second_last;
	return removed;
}

// �Ƴ��ڵ�ָ��Ϊp�Ľڵ�  �����ͷŽڵ��ڴ�
int list_remove(list_s* phead, list_s* p)
{
	if (!list_is_empty(phead)) {
		// �ǿ�
		list_s* node = NULL;
		// �����ҵ��ڵ�p
		for (node = phead->next; node != phead; node = node->next) {
			if (node == p) {
				// �Ƴ��ڵ�
				node->prior->next = node->next;
				node->next->prior = node->prior;
				return 1; // ɾ���ڵ�ɹ�
			}
		}
		return 0; // ɾ���ڵ㲻����
	}
	else
	{
		return 2;// �����
	}
}

#if 0
#define array_size(Array) (sizeof(Array) / sizeof(Array[0]))
// ����ʹ��
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
		{list->insert_head,list->remove_head,"ͷͷ"},
		{list->insert_head,list->remove_tail,"ͷβ"},
		{list->insert_tail,list->remove_head,"βͷ"},
		{list->insert_tail,list->remove_tail,"ββ"}
	};

	for (it2 = 0; it2 < array_size(func_ptr); it2++) {
		printf("\n���Ե�%d�ַ�ʽ,��ʽ:%s:\n", it2 + 1, func_ptr[it2].msg);
		//������г�ʼ������
		list->init(&list_head);
		//�������ݲ���˫������,����2�ַ�ʽ֮һ
		for (it1 = 0; it1 < array_size(md); it1++) {
			md[it1].num = it1;
			func_ptr[it2].insert(&list_head, &md[it1].list_entry);
		}
		//������������˫������,����2�ַ�ʽ֮һ
		while (!list->is_empty(&list_head)) {
			//�õ����Ƴ���������ָ��
			list_s* plist = func_ptr[it2].remove(&list_head);
			//ͨ��������ָ��õ�����������ṹ�Ľṹ���ָ��
			my_data* pmd = list_data(plist, my_data, list_entry);
			//����������,��֤��ȷ��
			printf("md[0x%08X].num = %d\n", pmd, pmd->num);
		}
	}
}
#endif