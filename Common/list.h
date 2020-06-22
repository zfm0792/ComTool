#ifndef __LIST_H__
#define __LIST_H__

/*
 *  ˫������Ļ�������
 *
*/

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct _list_s {
		struct _list_s* prior;
		struct _list_s* next;
	}list_s;

	struct _my_list {
		int(*is_empty)(list_s* phead);
		void(*init)(list_s* phead);
		void(*insert_head)(list_s* phead, list_s *plist);
		void(*insert_tail)(list_s* phead, list_s *plist);
		list_s* (*remove_head)(list_s* phead);
		list_s* (*remove_tail)(list_s *phead);
		int(*remove)(list_s* phead, list_s* p);
	};

// ���ݸ����Ľṹ���ͺ�һ���ṹ���Ա�����ĵ�ַ����ṹ��Ļ���ַ
// addr:  �ṹ����ĳ����Ա�����ĵ�ַ
// type:  �ṹ���ԭ��
// member: �ṹ���ĳ����Ա
// CONTAINING_RECORD containing_record
// �ṹ��ĵ�ַ + ��Ա������ƫ�� = ��Ա�����ĵ�ַ
// (unsigned long)&(((type*)0)->member))):��Ա������ƫ��
// (((unsigned char*)addr) ��Ա�����ĵ�ַ
#define list_data(addr,type,member) \
	((type*)(((unsigned char*)addr)-(unsigned long)&(((type*)0)->member)))

#ifdef __list_c__
#undef __list_c__

	static int list_is_empty(list_s* phead);
	static void list_init(list_s* phead);
	static void list_insert_head(list_s* phead, list_s* plist);
	static void list_insert_tail(list_s* phead, list_s* plist);
	static list_s* list_remove_head(list_s* phead);
	static list_s* list_remove_tail(list_s* phead);
	static int list_remove(list_s* phead, list_s* p);

#else

	extern struct _my_list* list;

#endif //!__list_c__

#ifdef __cplusplus
}
#endif // !cplusplus

#endif // !__LIST_H__