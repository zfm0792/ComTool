#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

	void init_memory(void);

	enum {MANMEM_INITIALIZE,MANMEM_INTSERT,MANMEM_REMOVE,MANMEM_FREE};

#define GET_MEM(size) memory.get_mem_debug(size,__THIS_FILE__,__LINE__);

	struct memory_s
	{
		void(*manage_mem)(int what, void*pv);
		void *(*get_mem_debug)(size_t size, char* file, int line);
		void(*free_mem)(void** ppv, char* prefix);
	};

#ifndef __MEMORY_C__
	extern struct memory_s memory;
#else

#undef __MEMORY_C__

	struct memory_s memory;
	void manage_mem(int what, void* pv);
	void* get_mem_debug(size_t size, char* file, int line);
	void free_mem(void** ppv, char* prefix);
#endif;

#ifdef __cplusplus
}
#endif

#endif