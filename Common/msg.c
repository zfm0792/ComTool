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

		//WM_COMMAND����������������˵��� ������ټ�������Ӵ��ڰ�ť�������������ť
		_SETRESULT(WM_COMMAND, msg.on_command((HWND)lParam, LOWORD(wParam), HIWORD(wParam)), 1);

		//���豸������/�γ���ʱ��WINDOWS����ÿ�����巢��WM_DEVICECHANGE ��Ϣ
		_SETRESULT(WM_DEVICECHANGE, msg.on_device_change(wParam, (DEV_BROADCAST_HDR*)lParam), 1);

		//��һ�������Ե���Ϣ����װ�ã�����ÿ��һ��ָ��ʱ�䷢��һ�ζ�ʱ��Ϣ ��SetTimer() �������
		_SETRESULT(WM_TIMER, msg.on_timer((int)wParam), 1);

		//�������޸���SystemParametersInfo���ã���Windows�ͻ�㲥����Ϣ
		_SETRESULT(WM_SETTINGCHANGE, msg.on_setting_change(), 1);
		_SETRESULT(WM_SIZE, msg.on_size(LOWORD(lParam), HIWORD(lParam)), 1);

		// ���ı䴰�ڴ�Сʱ����ǰ���������Ϣ,�����ڳ����н��մ���Ϣ���������ش�С,
		_SETRESULT(WM_SIZING, msg.on_sizing(wParam, (RECT*)lParam), 1); 

		//��������˽����Ϣ,������WM_APP+X�ĸ�ʽʹ��
		_SETRESULT(WM_APP, msg.on_app(uMsg, wParam, lParam), 1);
		//===========================
	case WM_INITDIALOG:
		msg.on_create(hWnd, (HINSTANCE)GetModuleHandle(NULL));
	default:
		return 0;
	}
}
#undef _SETRESULT

//��Ϣ����������
int run_app(void)
{
	// ��ȡһ��Ӧ�ó����̬���ӿ��ģ����
	HINSTANCE hInstance = GetModuleHandle(NULL);
	// ��һ���Ի���ģ����Դ����һ����ģʽ�ĶԻ���
	return (int)CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DLG_MAIN), NULL, (DLGPROC)MainWndProc);
}
// 
int on_create(HWND hWnd, HINSTANCE hInstance)
{
	HICON hIcon = NULL;
	
	// ��ʼ�����
	msg.hWndMain = hWnd;
	msg.hInstance = hInstance;
	msg.hEditRecv = GetDlgItem(hWnd, IDC_EDIT_RECV);
	msg.hEditRecv2 = GetDlgItem(hWnd, IDC_EDIT_RECV2);
	msg.hEditSend = GetDlgItem(hWnd, IDC_EDIT_SEND);
	msg.hComPort = INVALID_HANDLE_VALUE;

	// ���洰��Ĭ�ϴ�С
	GetWindowRect(msg.hEditRecv, &msg.WndSize.rcRecv);
	GetWindowRect(msg.hEditSend, &msg.WndSize.rcSend);
	GetWindowRect(GetDlgItem(hWnd, IDC_STATIC_RECV), &msg.WndSize.rcRecvGroup);
	GetWindowRect(GetDlgItem(hWnd, IDC_STATIC_SEND), &msg.WndSize.rcSendGroup);
	GetWindowRect(hWnd, &msg.WndSize.rcWindow);
	GetClientRect(hWnd, &msg.WndSize.rcWindowC);

	// ���ھ���
	utils.center_window(hWnd, NULL);
	// ���ñ���
	SetWindowText(hWnd, COMMON_NAME_AND_VERSION);
	// ���ù��λ��
	SetFocus(GetDlgItem(hWnd, IDC_BTN_OPEN));

	// ���ؿ�ݼ���
	msg.hAccel = LoadAccelerators(msg.hInstance, MAKEINTRESOURCE(IDC_COMMON));
	
	msg.hFont = CreateFont(
		10, 5, /*Height,Width*/
		0, 0, /*escapement,orientation*/
		FW_REGULAR, FALSE, FALSE, FALSE, /*weight, italic, underline, strikeout*/
		ANSI_CHARSET, OUT_DEVICE_PRECIS, CLIP_MASK, /*charset, precision, clipping*/
		DEFAULT_QUALITY, DEFAULT_PITCH, /*quality, and pitch*/
		"Courier"); /*font name*/

	// �ı���ʾ�������ʽ
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV2, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_SEND, WM_SETFONT, (WPARAM)msg.hFont, MAKELPARAM(TRUE, 0));

	//�ı���ʾ�������뷢�ͻ����С
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV, EM_SETLIMITTEXT, (WPARAM)COMMON_RECV_BUF_SIZE, 0);
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_SEND, EM_SETLIMITTEXT, (WPARAM)COMMON_SEND_BUF_SIZE, 0);
	SendDlgItemMessage(msg.hWndMain, IDC_EDIT_RECV2, EM_SETLIMITTEXT, (WPARAM)COMMON_RECV_BUF_SIZE, 0);


	//�ı�ָ�����ڵ�����
	// GWL_WNDPROC :���ڹ�������һ���µĵ�ַ��  ֻ����Win32ƽ̨
	OldRecvEditWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(msg.hWndMain, IDC_EDIT_RECV), GWL_WNDPROC, (LONG)RecvEditWndProc);
	OldRecv2EditWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(msg.hWndMain, IDC_EDIT_RECV2), GWL_WNDPROC, (LONG)Recv2EditWndProc);
	ShowWindow(msg.hEditRecv2, FALSE);


	comm.init();
	// ����com���б�  �Զ�ʶ������Com
	comm.update((int*)-1); 
	if (ComboBox_GetCount(GetDlgItem(hWnd, IDC_CBO_CP)) == 0)
		deal.update_status("û���κο��õĴ���");

	//�ڴ�����������������Լ����ڴ����get_mem������֮ǰ��ʼ��
	memory.manage_mem(MANMEM_INITIALIZE, NULL); // ��ʼ���ڴ�����list_head  �ֲ���̬����
	// ��ʼ��ȫ�ֵ�list_head
	list->init(&list_head);
	// ��ʼ�������ٽ�������
	InitializeCriticalSection(&window_critical_section);

	// ��ʼ��δ�����ͻ���
	deal.do_buf_send(SEND_DATA_ACTION_INIT, NULL);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	return 0;
}
int on_close(void)
{
	// �˳� �������
	// �ڴ��ͷ�===
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
	{//��ݼ���Ϣ
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
// ���豸�����ı� ��⴮���豸�ĸĶ�
// event --- �豸�¼�
// pDBH  --- DEV_BROADCAST_HDR
// ����VC6
#if _MSC_VER > 1200			
#define  strnicmp _strnicmp
#endif
int on_device_change(WPARAM event, DEV_BROADCAST_HDR* pDBH)
{
	if (msg.hComPort == INVALID_HANDLE_VALUE)
	{
		if (event == DBT_DEVICEARRIVAL)// ���豸����
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
					_snprintf(buff, sizeof(buff), "�Ѽ�⵽�����豸 %s �Ĳ���\n", name);
					debug_out((buff));
					if(comm.update((int*)&com_id))
						ComboBox_SetCurSel(hComPort, com_id);
					SetFocus(GetDlgItem(msg.hWndMain, IDC_EDIT_SEND));
				}
			}
		}
		else if (event == DBT_DEVICEREMOVECOMPLETE)// �豸�Ƴ�
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
					_snprintf(buff, sizeof(buff), "�����豸 %s ���Ƴ�\n", name);
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
		debug_out(("����on_time\n"));
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