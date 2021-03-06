#define __MEMORY_C__
#include <windows.h>

#include "memoryManage.h"
#include "list.h"
#include "utils.h"
#include "debug.h"
#include "msg.h"
#include "about.h"

static char* __THIS_FILE__ = __FILE__;

void init_memory(void)
{
	memory.manage_mem = manage_mem;
	memory.get_mem_debug = get_mem_debug;
	memory.free_mem = free_mem;
}
#pragma pack(push,1)
typedef struct {
	unsigned char sign_head[2];
	char* file;
	int line;
	size_t size;
	list_s entry;
}common_mem_context;

typedef struct {
	unsigned char sign_tail[2];
}common_mem_context_tail;
#pragma pack(pop)

// 内存分配管理
void manage_mem(int what, void* pv)
{
	static list_s list_head;
	static CRITICAL_SECTION critical_section;

	if (what == MANMEM_INTSERT || what == MANMEM_REMOVE)
	{
		__try {
			EnterCriticalSection(&critical_section);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			MessageBox(NULL, "内存未正确初始化", NULL, MB_ICONERROR);
		}
	}

	switch (what)
	{
	case MANMEM_INITIALIZE:
		list->init(&list_head); // 初始化链表头结点
		InitializeCriticalSection(&critical_section); //初始化临界区
		break;

	case MANMEM_INTSERT:
		list->insert_tail(&list_head, (list_s*)pv);
		break;

	case MANMEM_REMOVE:
		switch (list->remove(&list_head, (list_s*)pv))
		{
		case 1:
			break;
		case 0:
			utils.msgbox(msg.hWndMain, MB_ICONERROR, NULL, "无此节点:%p", pv);
			break;
		case 2:
			utils.msgbox(msg.hWndMain, MB_ICONERROR, NULL, "链表为空");
			break;
		}
		break;
	case MANMEM_FREE:
		debug_out(("内存泄露检查\n"));
		if (!list->is_empty(&list_head)) {
			int i = 1;
			utils.msgbox(msg.hWndMain, MB_ICONERROR, NULL, "发现未释放的内存!\n\n");
			// free_mem 会移除链表节点  所以此处只能遍历  不能移除
			while (!list->is_empty(&list_head))
			{
				list_s* node = list_head.next;
				common_mem_context *pc = list_data(node, common_mem_context, entry);
				void* user_ptr = (void*)((unsigned char*)pc + sizeof(*pc));
				utils.msgbox(msg.hWndMain, MB_ICONEXCLAMATION, NULL,
					"内存节点:%d\n\n"
					"以下是内存分配信息:\n\n"
					"分配大小:%u\n"
					"来自文件:%s\n"
					"文件行号:%d\n"
					, i++, pc->size, pc->file, pc->line);

				memory.free_mem(&user_ptr, "MANMEM_FREE");
			}
			list->init(&list_head);
			DeleteCriticalSection(&critical_section);
			return;
		}
		return;
	}
	LeaveCriticalSection(&critical_section);
}
// 内存分配
void* get_mem_debug(size_t size, char* file, int line)
{
	size_t all = sizeof(common_mem_context) + size + sizeof(common_mem_context_tail);
	void *pv = malloc(all);
	common_mem_context* pc = (common_mem_context*)pv;
	//void* user_ptr = (unsigned char*)pc + sizeof(common_mem_context);
	void* user_ptr = (unsigned char*)pc + sizeof(*pc);
	common_mem_context_tail *pct = (common_mem_context_tail*)((unsigned char*)user_ptr + size);
	if (!pv)
	{
		utils.msgbox(msg.hWndMain, MB_ICONERROR, NULL, "内存分配错误");
		return NULL;
	}

	memset(pv, 0, all);

	pc->sign_head[0] = 'J';
	pc->sign_head[1] = 'J';

	pc->size = size;

	pc->file = file;
	pc->line = line;
	debug_out(("分配内存:%u 字节\n 来自文件:%s\n 文件行号:%d\n\n", pc->size, pc->file, pc->line));

	pct->sign_tail[0] = 'J';
	pct->sign_tail [1] = 'J';
	manage_mem(MANMEM_INTSERT, &pc->entry);
	return user_ptr;
}

// 释放分配的内存  并处理异常
void free_mem(void** ppv, char* prefix)
{
	common_mem_context* pc = NULL;
	common_mem_context_tail* pct = NULL;

	if (ppv == NULL || *ppv == NULL)
	{
		utils.msgbox(msg.hWndMain, MB_ICONEXCLAMATION, NULL, "memory.free_mem 释放空指针, 来自:%s", prefix);
		return;
	}

	__try {
		pc = (common_mem_context*)((size_t)*ppv - sizeof(*pc));
		pct = (common_mem_context_tail*)((unsigned char*)pc + sizeof(common_mem_context) + pc->size);

		if ((pc->sign_head[0] == 'J') && (pc->sign_head[1] == 'J') &&
			pct->sign_tail[0] == 'J' && (pct->sign_tail[1] == 'J'))
		{
			manage_mem(MANMEM_REMOVE, &pc->entry);
			free(pc);
			*ppv = NULL;
		}
		else
		{
			manage_mem(MANMEM_REMOVE, &pc->entry);
			utils.msgbox(msg.hWndMain, MB_ICONERROR, "debug error",
				"待释放内存签名不正确!\n\n"
				"文件:%s\n"
				"行数:%d", pc->file, pc->line);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		utils.msgbox(msg.hWndMain, MB_ICONERROR, COMMON_NAME,
			"%s:指针被错误地释放,请报告异常!\n\n"
			"文件:%s\n"
			"行数:%d", prefix ? prefix : "<null-function-name>", pc->file, pc->line);
	}
}
