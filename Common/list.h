#ifndef __LIST_H__
#define __LIST_H__

/*
 *  双向链表的基本操作
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

// 根据给定的结构类型和一个结构体成员变量的地址来求结构体的基地址
// addr:  结构体中某个成员变量的地址
// type:  结构体的原型
// member: 结构体的某个成员
// CONTAINING_RECORD containing_record
// 结构体的地址 + 成员变量的偏移 = 成员变量的地址
// (unsigned long)&(((type*)0)->member))):成员变量的偏移
// (((unsigned char*)addr) 成员变量的地址
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