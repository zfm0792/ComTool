#define __UTILS_C__
#include "utils.h"
#include "msg.h"
#include "about.h"
#include "memoryManage.h"

struct utils_s utils;
static char* __THIS_FILE__ = __FILE__;

void init_utils(void)
{
	memset(&utils, 0, sizeof(utils));
	utils.msgbox = msgbox;
	utils.msgerr = msgerr;

	utils.get_file_name = get_file_name;
	utils.set_clip_data = set_clip_data;

	utils.hex2chs = hex2chs;
	utils.hex2str = hex2str;
	utils.str2hex = str2hex;

	utils.wstr2lstr = wstr2lstr;

	utils.remove_string_linefeed = remove_string_linefeed;
	utils.remove_string_return = remove_string_return;
	utils.parse_string_escape_char = parse_string_escape_char;
	utils.check_chs = check_chs;

	utils.center_window = center_window;

	utils.bubble_sort = bubble_sort;
	utils.assert_expr = myassert;
}

// 显示消息框
// msgicon: 消息光标 caption:消息标题 fmt:格式字符串
int msgbox(HWND hOwner, UINT msgicon, char * caption, char * fmt, ...)
{
	va_list va;
	char smsg[1024] = { 0 };
	va_start(va, fmt);
	_vsnprintf(smsg, sizeof(smsg), fmt, va);
	va_end(va);
	return MessageBox(hOwner, smsg, caption, msgicon);
}
// 显示带prefix前缀的系统错误消息
// prefix - 前缀字符串
void msgerr(HWND hOwner, char* prefix)
{
	char *buffer = NULL;
	if (!prefix)
		prefix = "";
	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPSTR)&buffer, 1, NULL))
	{
		utils.msgbox(hOwner, MB_ICONHAND, NULL, "%s:%s", prefix, buffer);
		LocalFree(buffer);
	}
}

char* get_file_name(char* title, char* filter, int action, int* opentype)
{

	return NULL;
}

int set_clip_data(char* str)
{
	return 0;
}

// 转换16进制字符串到16进制值数组
// str:指向包含16进制的字符串
// ppBuffer:用来保存转换后的结果的缓冲区
// buf_size: 若传入默认缓冲区 指定默认缓冲区的大小
// 11 12 23 34 4F 5B...
// 三种状态的转换  

enum {S2H_NULL,S2H_SPACE,S2H_HEX,S2H_END};
unsigned int str2hex(char* str, unsigned char** ppBuffer, unsigned int buf_size)
{
	unsigned char hex = 0;			//用来保存解析的单个16进制值
	unsigned int count = 0;			//保存解析的16进制的个数
	unsigned char* hexarray;		//保存转换后的结果
	unsigned char* pba;				//用来向hexarray中写数据
	unsigned char* pp = (unsigned char*)str;	//待解析的字符串

	int flag_last = S2H_NULL, flag;				//词法解析用到的标记位

	if (str == NULL) return 0;
	//由于是2个字符+若干空白组成一个16进制, 所以最多不可能超过(strlen(str)/2) 0xff0x0a0x
	if (*ppBuffer && buf_size >= strlen(str) / 2) {
		hexarray = *ppBuffer;
	}
	else {
		hexarray = (unsigned char*)GET_MEM(strlen(str) / 2);
		if (hexarray == NULL) {
			*ppBuffer = NULL;
			return 0;
		}
		else {
			//放到最后,判断是否需要释放
			//*ppBuffer = hexarray;
		}
	}
	pba = hexarray;

	for (;;) {
		if (*pp == 0)
			flag = S2H_END;
		else if (isxdigit(*pp))  // 参数是否为16进制数字，是返回非零值，否则返回0
			flag = S2H_HEX;
		else if (*pp == 0x20 || *pp == 0x09 || *pp == '\r' || *pp == '\n')
			flag = S2H_SPACE;
		else {
			//printf("非法字符!\n");
			goto _parse_error;
		}

		switch (flag_last)
		{
		case S2H_HEX:
		{
			if (flag == S2H_HEX) {
				hex <<= 4;// 10000 === 8   
				if (isdigit(*pp)) hex += *pp - '0';
				else hex += (*pp | 0x20) - 87;
				*pba++ = hex;
				count++;
				flag_last = S2H_NULL;
				pp++;
				continue;
			}
			else {
				//printf("不完整!\n");
				goto _parse_error;
			}
		}
		case S2H_SPACE:
		{
			if (flag == S2H_SPACE) {
				pp++;
				continue;
			}
			else if (flag == S2H_HEX) {
				if (isdigit(*pp)) hex = *pp - '0';
				else hex = (*pp | 0x20) - 87;  //'a'(97)-->10
				pp++;
				flag_last = S2H_HEX;
				continue;
			}
			else if (flag == S2H_END) {
				goto _exit_for;
			}
		}
		case S2H_NULL:
		{
			if (flag == S2H_HEX) {
				if (isdigit(*pp)) hex = *pp - '0';
				else hex = (*pp | 0x20) - 87;
				pp++;
				flag_last = S2H_HEX;
				continue;
			}
			else if (flag == S2H_SPACE) {
				flag_last = S2H_SPACE;
				pp++;
				continue;;
			}
			else if (flag == S2H_END) {
				goto _exit_for;
			}
		}
		}
	}
_parse_error:
	if (hexarray != *ppBuffer) {
		memory.free_mem((void**)&hexarray, "<utils.str2hex>");
	}
	return 0 | ((unsigned int)pp - (unsigned int)str);
_exit_for:
	*ppBuffer = hexarray;
	return count | 0x80000000;
}
// 转换16进制值数组到16进制字符串
char* hex2str(unsigned char* hexarray, int* length, int linecch, int start, char* buf, int buf_size)
{

	return NULL;
}
// 转换16进制数组到字符字符串
char* hex2chs(unsigned char* hexarray, int length, char* buf, int buf_size)
{
	return NULL;
}


// 把指定窗口居中于指定的另一窗口
// hWnd:待居中的窗口
// hWndOwner:参考的窗口句柄
void center_window(HWND hWnd, HWND hWndOwner)
{
	RECT rchWnd, rchWndOwner;
	int width, height;
	int x, y;
	if (!IsWindow(hWnd)) return;

	GetWindowRect(hWnd, &rchWnd);
	// 屏幕
	if (!hWndOwner || !IsWindow(hWndOwner))
	{
		int scrWidth, scrHeight;
		scrWidth = GetSystemMetrics(SM_CXSCREEN);
		scrHeight = GetSystemMetrics(SM_CYSCREEN);
		SetRect(&rchWndOwner,0,0,scrWidth,scrHeight);
	}
	else
	{
		GetWindowRect(hWndOwner, &rchWndOwner);
	}
	width = rchWnd.right - rchWnd.left;
	height = rchWnd.bottom - rchWnd.top;

	x = (rchWndOwner.right - rchWndOwner.left - width) / 2 + rchWndOwner.left;
	y = (rchWndOwner.bottom - rchWndOwner.top - height) / 2 + rchWndOwner.top;

	MoveWindow(hWnd, x, y, width, height, TRUE);
}

void bubble_sort(int* a, int size, int inc_or_dec)
{
	return;
}

void myassert(void* pv, char* str)
{
	if (!pv)
	{
		utils.msgbox(msg.hWndMain, MB_ICONERROR, COMMON_NAME, "Debug Error:%s\n\n"
			"应用程序遇到内部错误,请报告错误!\n\n"
			"请重新启动应用程序后再试!", str);
	}
	return;
}

int wstr2lstr(char* src)
{
	return 0;
}

int check_chs(unsigned char* ba, int cb)
{
	return 0;
}

// 移除字符串中的回车换行 '\r' '\n' 
unsigned int remove_string_return(char* str)
{
	char *p1 = str;
	char *p2 = str;

	while (*p1)
	{
		if (*p1 == '\r' || *p1 == '\n')
			p1++;
		else
			*p2++ = *p1++;
	}
	*p2 = '\0';
	return (unsigned int)p2 - (unsigned int)str;
}
//移除字符串中的 '\r'
unsigned int remove_string_linefeed(char* str)
{
	char *p1 = str;
	char *p2 = str;

	while (*p1)
	{
		if (*p1 == '\r')
			p1++;
		else
			*p2++ = *p1++;
	}
	*p2 = '\0';
	return (unsigned int)p2 - (unsigned int)str;
}
//
unsigned int parse_string_escape_char(char* str)
{
	return 0;
}