#include <stdio.h>
#include <ctype.h>
#include <process.h>

#define __MSG_C__
#include "msg.h"
#include "utils.h"
#include "deal.h"
#include "about.h"
#include "comm.h"
#include "debug.h"
#include "memoryManage.h"
#include "list.h"
#include "resource.h"

struct msg_s msg;
static list_s list_head;
CRITICAL_SECTION window_critical_section;

static char* __THIS_FILE__ = __FILE__;
int init_msg(void)
{
	memset(&msg, 0, sizeof(msg));
	
	msg.run_app = run_app;
	msg.on_create = on_create;
	msg.on_close = on_close;
	msg.on_destroy = on_destroy;
	msg.on_command = on_command;
	msg.on_timer = on_timer;
	msg.on_device_change = on_device_change;
	msg.on_setting_change = on_setting_change;
	msg.on_size = on_size;
	msg.on_sizing = on_sizing;
	msg.on_app = on_app;
	return 1;
}
WNDPROC OldRecvEditWndProc = NULL;
LRESULT CALLBACK RecvEditWndProc(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(OldRecvEditWndProc,hWnd, uMsg, wParam, lParam);
}


WNDPROC OldRecv2EditWndProc = NULL;
LRESULT CALLBACK Recv2EditWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc(OldRecv2EditWndProc, hWnd, uMsg, wParam, lParam);
}

#define _SETRESULT(_msg,_result,_msgret) \
	case _msg:SetDlgMsgResult(hWnd,_msg,_result);return _msgret;

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		//===========================
		_SETRESULT(WM_CLOSE, msg.on_close(), 1);
		_SETRESULT(WM_DESTROY, msg.on_destroy(), 1);

		//WM_COMMAND产生的条件：点击菜单， 点击加速键，点击子窗口按钮，点击工具栏按钮
		_SETRESULT(WM_COMMAND, msg.on_command((HWND)lParam, LOWORD(wParam), HIWORD(wParam)), 1);

		//当设备被插入/拔出的时候，WINDOWS会向每个窗体发送WM_DEVICECHANGE 消息
		_SETRESULT(WM_DEVICECHANGE, msg.on_device_change(wParam, (DEV_BROADCAST_HDR*)lParam), 1);

		//是一种周期性的消息产生装置，它会每隔一段指定时间发送一次定时消息 与SetTimer() 函数相关
		_SETRESULT(WM_TIMER, msg.on_timer((int)wParam), 1);

		//当程序修改了SystemParametersInfo设置，则Windows就会广播此消息
		_SETRESULT(WM_SETTINGCHANGE, msg.on_setting_change(), 1);
		_SETRESULT(WM_SIZE, msg.on_size(LOWORD(lParam), HIWORD(lParam)), 1);

		// 当改变窗口大小时会提前触发这个消息,可以在程序中接收此消息并可以拦截大小,
		_SETRESULT(WM_SIZING, msg.on_sizing(wParam, (RECT*)lParam), 1); 

		//用来定义私有消息,经常以WM_APP+X的格式使用
		_SETRESULT(WM_APP, msg.on_app(uMsg, wParam, lParam), 1);
		//===========================
	case WM_INITDIALOG:
		msg.on_create(hWnd, (HINSTANCE)GetModuleHandle(NULL));
	default:
		return 0;
	}
}
#undef _SETRESULT

//消息处理函数声明
int run_app(void)
{
	// 获取一个应用程序或动态链接库的模块句柄
	HINSTANCE hInstance = GetModuleHandle(NULL);
	// 从一个对话框模板资源创建一个无模式的对话框
	return (int)CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DLG_MAIN), NULL, (DLGPROC)MainWndProc);
}
// 
int on_create(HWND hWnd, HINSTANCE hInstance)
{
	HICON hIcon = NULL;
	
	// 初始化句柄
	msg.hWndMain = hWnd;
	msg.hInstance = hInstance;
	msg.hEditRecv = GetDlgItem(hWnd, IDC_EDIT_RECV);
	msg.hEditRecv2 = GetDlgItem(hWnd, IDC_EDIT_RECV2);
	msg.hEditSend = GetDlgItem(hWnd, IDC_EDIT_SEND);
	msg.hComPort = INVALID_HANDLE_VALUE;

	// 保存窗口默认大小
	GetWindowRect(msg.hEditRecv, &msg.WndSize.rcRecv);
	GetWindowRect(msg.hEditSend, &msg.WndSize.rcSend);
	GetWindowRect(GetDlgItem(hWnd, IDC_STATIC_RECV), &msg.WndSize.rcRecvGroup);
	GetWindowRect(GetDlgItem(hWnd, IDC_STATIC_SEND), &msg.WndSize.rcSendGroup);
	GetWindowRect(hWnd, &msg.WndSize.rcWindow);
	GetClientRect(hWnd, &msg.WndSize.rcWindowC);

	// 窗口居中
	utils.center_window(hWnd, NULL);
	// 设置标题
	SetWindowText(hWnd, COMMON_NAME_AND_VERSION);
	// 设置光标位置
	SetFocus(GetDlgItem(hWnd, IDC_BTN_OPEN));

	// 加载快捷键表
	msg.hAccel = LoadAccelerators(msg.hInstance, MAKEINTRESOURCE(IDC_COMMON));
	
	msg.hFont = CreateFont(
		10, 5, /*Height,Width*/
		0, 0, /*escapement,orientation*/
		FW_REGULAR, FALSE, FALSE, FALSE, /*weight, italic, underline, strikeout*/
		ANSI_CHARSET, OUT_DEVICE_PRECIS, CLIP_MASK, /*charset, precision, clipping*/
		DEFAULT_QUALITY, DEFAULT_PITCH, /*quality, and pitch*/
		"Courier"); /*font name*/

	// 文本显示框字体格式
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV2, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_SEND, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));

	//文本显示区接收与发送缓冲大小
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV, EM_SETLIMITTEXT, (WPARAM)COMMON_RECV_BUF_SIZE, 0);
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_SEND, EM_SETLIMITTEXT, (WPARAM)COMMON_SEND_BUF_SIZE, 0);
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV2, EM_SETLIMITTEXT, (WPARAM)COMMON_RECV_BUF_SIZE, 0);


	//改变指定窗口的属性
	// GWL_WNDPROC :窗口过程设置一个新的地址。  只能在Win32平台
	OldRecvEditWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(msg.hWndMain, IDC_EDIT_RECV), GWL_WNDPROC, (LONG)RecvEditWndProc);
	OldRecv2EditWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(msg.hWndMain, IDC_EDIT_RECV2), GWL_WNDPROC, (LONG)Recv2EditWndProc);
	ShowWindow(msg.hEditRecv2, FALSE);


	comm.init();
	// 更新com的列表  自动识别插入的Com
	comm.update((int*)-1); 
	if (ComboBox_GetCount(GetDlgItem(hWnd, IDC_CBO_CP)) == 0)
		deal.update_status("没有任何可用的串口");

	//内存管理必须放在所有我自己的内存分配get_mem被调用之前初始化
	memory.manage_mem(MANMEM_INITIALIZE, NULL); // 初始化内存管理的list_head  局部静态变量
	// 初始化全局的list_head
	list->init(&list_head);
	// 初始化窗口临界区变量
	InitializeCriticalSection(&window_critical_section);

	// 初始化未被发送缓冲
	deal.do_buf_send(SEND_DATA_ACTION_INIT, NULL);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	return 0;
}
int on_close(void)
{
	// 退出 清理操作
	// 内存释放===
	DestroyWindow(msg.hWndMain);
	return 0;
}
int on_destroy(void)
{
	msg.hWndMain = NULL;
	PostQuitMessage(0);
	return 0;
}
int on_command(HWND hwhWndCtrl, int id, int codeNotify)
{
	if (!hwhWndCtrl && !codeNotify)
	{//Menu
		switch (id)
		{

		}
	}
	if (!hwhWndCtrl && codeNotify == 1)
	{//快捷键消息
		switch (id)
		{

		}
	}

	switch (id)
	{

	case IDC_BTN_OPEN:
	{
		if (comm.fCommOpened) {
			if (comm.close(0))
				comm.update((int*)-1);
		}
		else
			comm.open();
		deal.update_savebtn_status();
		return 0;
	}
	case IDC_BTN_SEND:
		deal.do_send(0);
		return 0;
	}

	return 0;
}

int on_activateapp(BOOL bActivate)
{
	return 0;
}
// 在设备发生改变 检测串口设备的改动
// event --- 设备事件
// pDBH  --- DEV_BROADCAST_HDR
// 兼容VC6
#if _MSC_VER > 1200			
#define  strnicmp _strnicmp
#endif
int on_device_change(WPARAM event, DEV_BROADCAST_HDR* pDBH)
{
	if (msg.hComPort == INVALID_HANDLE_VALUE)
	{
		if (event == DBT_DEVICEARRIVAL)// 新设备插入
		{
			if (pDBH->dbch_devicetype == DBT_DEVTYP_PORT)
			{
				DEV_BROADCAST_PORT* pPort = (DEV_BROADCAST_PORT*)pDBH;
				char* name = &pPort->dbcp_name[0];
				if (strnicmp("COM", name, 3) == 0)
				{
					int com_id;
					char buff[32];
					extern HWND hComPort;
					com_id = atoi(name + 3);
					_snprintf(buff, sizeof(buff), "已检测到串口设备 %s 的插入\n", name);
					debug_out((buff));
					if(comm.update((int*)&com_id))
						ComboBox_SetCurSel(hComPort, com_id);
					SetFocus(GetDlgItem(msg.hWndMain, IDC_EDIT_SEND));
				}
			}
		}
		else if (event == DBT_DEVICEREMOVECOMPLETE)// 设备移除
		{
			if (pDBH->dbch_devicetype == DBT_DEVTYP_PORT)
			{
				DEV_BROADCAST_PORT* pPort = (DEV_BROADCAST_PORT*)pDBH;
				char* name = &pPort->dbcp_name[0];
				if (strnicmp("COM", name, 3) == 0)
				{
					int com_id;
					char buff[32];
					extern HWND hComPort;
					com_id = atoi(name + 3);
					_snprintf(buff, sizeof(buff), "串口设备 %s 已移除\n", name);
					debug_out((buff));
					comm.update((int*)-1);
					if (ComboBox_GetCount(hComPort))
						ComboBox_SetCurSel(hComPort, 0);
				}
			}
		}
	}
	return 0;
}
int on_setting_change(void)
{
	return 0;
}
int on_timer(int id)
{
	if (id == TIMER_ID_THREAD) {
		debug_out(("进入on_time\n"));
		comm.close(1);
		comm.update((int*)-1);
	}
	KillTimer(msg.hWndMain, TIMER_ID_THREAD);
	return 0;
}
int on_size(int width, int height)
{
	return 0;
}
int on_sizing(WPARAM edge, RECT* pRect)
{
	return 0;
}
int on_app(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}