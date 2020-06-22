#ifndef __UTILS_H__
#define __UTILS_H__

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define __ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	void init_utils(void);

	struct utils_s {
		int(*msgbox)(HWND hOwner, UINT msgicon, char* caption, char* fmt, ...);
		void(*msgerr)(HWND hOwner, char* prefix);

		char *(*get_file_name)(char* title, char* filter, int action, int* opentype);

		int(*set_clip_data)(char* str);

		unsigned int(*str2hex)(char* str, unsigned char** ppBuffer, unsigned int buf_size);
		char* (*hex2str)(unsigned char* hexarray, int* length, int linecch, int start, char* buf, int buf_size);
		char* (*hex2chs)(unsigned char* hexarray, int length, char* buf, int buf_size);
		
		void(*center_window)(HWND hWnd, HWND hWndOwner);
		
		void(*bubble_sort)(int *Array, int size, int inc_or_dec);
		
		int(*show_expr)(void);
		void(*assert_expr)(void* pv, char* str);

		int(*wstr2lstr)(char* src);
		int(*check_chs)(unsigned char* ba, int cb);

		unsigned int(*remove_string_return)(char* str);
		unsigned int(*remove_string_linefeed)(char* str);
		unsigned int(*parse_string_escape_char)(char* str);
	};

#ifndef __UTILS_C__

	extern struct utils_s utils;
#else

#undef __UTILS_C__

	static int msgbox(HWND hOwner, UINT msgicon, char* caption, char* fmt, ...);
	static void msgerr(HWND hOwner, char* prefix);

	static char* get_file_name(char* title, char* filter, int action, int* opentype);

	static int set_clip_data(char* str);

	static unsigned int str2hex(char* str, unsigned char** ppBuffer, unsigned int buf_size);
	static char* hex2str(unsigned char* hexarray, int* length, int linecch, int start, char* buf, int buf_size);
	static char* hex2chs(unsigned char* hexarray, int length, char* buf, int buf_size);

	static void center_window(HWND hWnd, HWND hWndOwner);

	static void bubble_sort(int* a, int size, int inc_or_dec);

	static void myassert(void* pv, char* str);

	static int wstr2lstr(char* src);
	static int check_chs(unsigned char* ba, int cb);

	static unsigned int remove_string_return(char* str);
	static unsigned int remove_string_linefeed(char* str);
	static unsigned int parse_string_escape_char(char* str);

#endif // !__UTILS_C__

#ifdef __cplusplus
}
#endif


#endif // !__UTILS_H__
