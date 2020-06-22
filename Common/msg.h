#ifndef __MSG_H__
#define __MSG_H__

#include <windows.h>
#include <windowsX.h>
#include <Dbt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "list.h"
struct msg_s
{
	struct {
		HWND hWndMain;
		HINSTANCE hInstance;
		HANDLE hComPort;
		HWND hEditRecv;
		HWND hEditRecv2;
		HWND hEditSend;
		HFONT hFont;
		HACCEL hAccel;
		//
		struct
		{
			RECT rcWindowC;
			RECT rcWindow;
			RECT rcRecv;
			RECT rcRecvGroup;
			RECT rcSend;
			RECT rcSendGroup;
		}WndSize;
	};

	int (*run_app)(void);
	int (*on_create)(HWND hWnd, HINSTANCE hInstance);
	int (*on_close)(void);
	int (*on_destroy)(void);
	int (*on_command)(HWND hwhWndCtrl, int id, int codeNotify);
	int (*on_device_change)(WPARAM event, DEV_BROADCAST_HDR* pDBH);
	int (*on_setting_change)(void);
	int (*on_timer)(int id);
	int (*on_size)(int width, int height);
	int (*on_sizing)(WPARAM edge, RECT* pRect);
	int (*on_app)(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

int init_msg(void);
enum { TIMER_ID_THREAD };


#ifndef __MSG_C__
extern struct msg_s msg;
#else
#undef __MSG_C__

LRESULT CALLBACK RecvEditWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//消息处理函数声明
static int run_app(void);
static int on_create(HWND hWnd, HINSTANCE hInstance);
static int on_close(void);
static int on_destroy(void);
static int on_command(HWND hwhWndCtrl, int id, int codeNotify);
static int on_activateapp(BOOL bActivate);
static int on_device_change(WPARAM event, DEV_BROADCAST_HDR* pDBH);
static int on_setting_change(void);
static int on_timer(int id);
static int on_size(int width, int height);
static int on_sizing(WPARAM edge, RECT* pRect);
static int on_app(UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct {
	HWND hWnd;
	list_s entry;
}WINDOW_ITEM;

#endif // !__MSG_C__



#endif // !__MSG_H__

