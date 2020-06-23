#define __COMM_C__

#include "comm.h"
#include "msg.h"
#include "utils.h"
#include "about.h"
#include "debug.h"
#include "deal.h"
#include "resource.h"

struct comm_s comm;
static char* __THIS_FILE__ = __FILE__;

// ������������
char* cBaudRate[] = { "110","300","600","1200","2400","4800","9600","14400","19200","38400","57600","115200","128000","256000",NULL };
DWORD iBaudRate[] = { CBR_110,CBR_300,CBR_600,CBR_1200,CBR_2400,CBR_4800,CBR_9600,CBR_14400,CBR_19200,CBR_38400,CBR_57600,CBR_115200,CBR_128000,CBR_256000 };
char *cParity[] = { "��У��","żУ��","��У��","���У��","�ո�У��",NULL };
BYTE iParity[] = { NOPARITY,EVENPARITY,ODDPARITY,MARKPARITY,SPACEPARITY };
char* cStopBit[] = { "1λ","1.5λ","2λ",NULL };
BYTE iStopBit[] = { ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS };
char* cDataSize[] = { "8λ","7λ","6λ","5λ",NULL };
BYTE iDataSize[] = { 8,7,6,5 };

// �ؼ����
HWND hWndMain;
HWND hComPort;
HWND hBaudRate;
HWND hParity;
HWND hDataSize;
HWND hStopBit;

// ������صĽṹ��
COMMCONFIG cconfig;
COMMTIMEOUTS ctimeouts = { 0,1,0,1,0 };

void init_comm(void)
{
	memset(&comm, 0, sizeof(comm));
	comm.init = init;
	comm.update = get_comm_list;
	comm.open = open;
	comm.save_to_file = save_to_file;
	comm.load_from_file = load_from_file;
	comm.set_data_fmt = set_data_fmt;
	comm.close = close;
	comm.hardware_config = hardware_config;
	//comm.show_pin_ctrl = show_pin_ctrl;
	comm.update_config = update_config;
	comm.switch_disp = switch_disp;
}

// �ڳ����ʼ��ʱ����ʼ������   ��WM_CREATE����
void init(void)
{
	int it;
	hWndMain = msg.hWndMain;
#define _GETHWND(name,id) \
	name = GetDlgItem(hWndMain,id)
	_GETHWND(hComPort, IDC_CBO_CP);
	_GETHWND(hBaudRate, IDC_CBO_BR);
	_GETHWND(hStopBit, IDC_CBO_STOP);
	_GETHWND(hParity, IDC_CBO_CHK);
	_GETHWND(hDataSize, IDC_CBO_DATA);
#undef _GETHWND

#define _SETLIST(_array,_hwnd,_init) \
	do{ \
		for(it = 0;_array[it];it++) \
			ComboBox_AddString(_hwnd,_array[it]); \
		ComboBox_SetCurSel(_hwnd,_init); \
	}while(0)

	_SETLIST(cBaudRate, hBaudRate, 6); // 9600��ʼ
	ComboBox_AddString(hBaudRate, "<����>");
	_SETLIST(cStopBit, hStopBit, 0);
	_SETLIST(cParity, hParity, 0);
	_SETLIST(cDataSize, hDataSize, 0);
#undef _SETLIST
	//CheckRadioButton: 
	CheckRadioButton(hWndMain, IDC_RADIO_SEND_HEX,IDC_RADIO_SEND_CHAR,IDC_RADIO_SEND_HEX);
	CheckRadioButton(hWndMain, IDC_RADIO_RECV_HEX, IDC_RADIO_RECV_CHAR, IDC_RADIO_RECV_HEX);
	// �����Զ�����ʱ��
	SetDlgItemText(hWndMain, IDC_EDIT_DELAY, "1000");
	CheckDlgButton(hWndMain, IDC_CHECK_IGNORE_RETURN, BST_UNCHECKED);
	CheckDlgButton(hWndMain, IDC_CHECK_USE_ESCAPE_CHAR, BST_UNCHECKED);
	EnableWindow(GetDlgItem(hWndMain, IDC_CHECK_IGNORE_RETURN), FALSE);
	EnableWindow(GetDlgItem(hWndMain, IDC_CHECK_USE_ESCAPE_CHAR), FALSE);
}

#define MAX_COM 16
struct COM_LIST {
	int count;
	int com_id[MAX_COM];
	char com_name[MAX_COM][256];
};

//����ϵͳ�����豸�������б�
int	get_comm_list(int* which)
{
	int it;
	static struct COM_LIST com_list;
	unsigned int com_index = 0;

	int now_sel = -1;
	int new_sel = 0;

	// SetupApi
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVINFO_DATA spdata = { 0 };
	// 0 ~ MAX_COM 
	if ((int)which < MAX_COM && (int)which >= 0)
		return com_list.com_id[(int)which]; // ����com id
	else if ((int)which == -1)
	{
		now_sel = ComboBox_GetCurSel(hComPort);
		if (now_sel != -1)
			now_sel = com_list.com_id[now_sel];
	}
	else if ((int)which >= MAX_COM && (int)which < 2 * MAX_COM) // ����com������
		return (int)com_list.com_name[(int)which - MAX_COM];

	// ���������豸
	com_list.count = 0;
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		utils.msgerr(msg.hWndMain, "SetupApi");
		com_list.count = 0;
		return 0;
	}

	//ö��ָ���豸��Ϣ���ϵĳ�Ա���������ݷ���PSP_DEVINFO_DATA��
	// hDevInfo:�豸��Ϣ���ϵľ��
	// it:Ҫȡ�õ��豸��Ϣ��Ա��ţ���0��ʼ
	spdata.cbSize = sizeof(spdata);
	for (it = 0; SetupDiEnumDeviceInfo(hDevInfo, it, &spdata); it++)
	{
		//��ǰ��õ���ָ��ĳһ�������豸��Ϣ���ϵ�ָ����ȡ��ĳһ����Ϣ
		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &spdata, SPDRP_FRIENDLYNAME, NULL,
			(PBYTE)&com_list.com_name[com_index][0], sizeof(com_list.com_name[0]), NULL))
		{
			char* pch = NULL;
			int tmp_id = 0;
			pch = strstr(&com_list.com_name[com_index][0], "LPT");
			if (pch) continue;//����

			pch = strstr(&com_list.com_name[com_index][0], "(COM");
			__try {
				tmp_id = atoi(pch + 4);
				*(pch - 1) = 0;
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				utils.msgbox(msg.hWndMain, MB_ICONERROR, COMMON_NAME, "SetupDiGetDeviceRegistryProperty:Access Violation! Please Report This Bug!");
			}
			com_list.com_id[com_index] = tmp_id;
			com_index++;
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	com_list.count = com_index;

	ComboBox_ResetContent(hComPort);
	for (;;) {
		unsigned int it;
		char str[300]; //"COMxx"
		for (it = 0; it < com_index; it++) {
			sprintf(str, "COM%2d  %s", com_list.com_id[it], com_list.com_name[it]);
			ComboBox_AddString(hComPort, str);
			if (which == (int*)-1) {
				if (now_sel != -1 && com_list.com_id[it] == now_sel) {
					new_sel = it;
				}
			}
			else {
				if (com_list.com_id[it] == *which) {
					*which = it;
				}
			}
		}
		break;
	}

	ComboBox_SetCurSel(hComPort, now_sel != -1 ? new_sel : 0);
	if (now_sel == -1 && (int)which == -1) {
		char str[64];
		_snprintf(str, __ARRAY_SIZE(str), "���ҵ������豸 %d ��\n", com_index);
		debug_out((str));
	}
	return 1;
}
// �򿪴���
int	open(void)
{
	char str[64];
	int index = ComboBox_GetCurSel(hComPort);
	if (index == CB_ERR)
	{
		utils.msgbox(msg.hWndMain, MB_ICONINFORMATION, COMMON_NAME, "û���κο��õĴ���");
		return 0;
	}
	index = get_comm_list((int*)index);
	sprintf(str, "\\\\.\\COM%d", index);

	// ͨ���ļ�api �򿪴���
	msg.hComPort = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (msg.hComPort == INVALID_HANDLE_VALUE) {
		DWORD lasterror = GetLastError();
		utils.msgerr(msg.hWndMain, "����");
		if (lasterror == ERROR_FILE_NOT_FOUND)
			comm.update((int*)-1);
		return 0;
	}
	if (!update_config(0)) { // ����com�Ĳ���
		CloseHandle(msg.hComPort);
		msg.hComPort = INVALID_HANDLE_VALUE;
		return 0;
	}

	deal.update_savebtn_status();
	EnableWindow(hComPort, FALSE);
	EnableWindow(hBaudRate, FALSE);
	EnableWindow(hStopBit, FALSE);
	EnableWindow(hParity, FALSE);
	EnableWindow(hDataSize, FALSE);
	SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "�رմ���");
	deal.update_status("�����Ѵ�");
	comm.fCommOpened = 1;

	// ��д�ܵ�,��д�߳�,�߳�ͬ��
	CreatePipe(&deal.thread.hPipeRead, &deal.thread.hPipeWrite, NULL, 0);

	deal.hEventContinueToRead = CreateEvent(NULL, TRUE, TRUE, NULL);

	deal.thread.hThreadRead = (HANDLE)_beginthreadex(NULL, 0, deal.thread_read, NULL, 0, NULL);
	deal.thread.hThreadWrite = (HANDLE)_beginthreadex(NULL, 0, deal.thread_write, NULL, 0, NULL);

	utils.assert_expr((void*)(
		deal.thread.hPipeRead &&
		deal.thread.hPipeWrite &&
		deal.thread.hThreadRead &&
		deal.thread.hThreadWrite &&
		deal.hEventContinueToRead),
		"<pipe,thread,event:creation failed!>");

	deal.check_auto_send();
	deal.start_timer(1);

	return 0;
}
int	save_to_file(void)
{
	return 0;
}
int	load_from_file(void)
{
	return 0;
}
// ȡ�õ�ǰ���͡��������ݵĸ�ʽ
void set_data_fmt(void)
{
	// ���Իس�����
	comm.data_fmt_ignore_return = IsDlgButtonChecked(msg.hWndMain, IDC_CHECK_IGNORE_RETURN);
	// ����16����
	comm.data_fmt_send = IsDlgButtonChecked(msg.hWndMain, IDC_RADIO_SEND_HEX);
	// ����16����
	comm.data_fmt_recv = IsDlgButtonChecked(msg.hWndMain, IDC_RADIO_RECV_HEX);
	// ʹ��ת���ַ�
	comm.data_fmt_use_escape_char = IsDlgButtonChecked(msg.hWndMain, IDC_CHECK_USE_ESCAPE_CHAR);

	EnableWindow(GetDlgItem(msg.hWndMain, IDC_CHECK_IGNORE_RETURN), comm.data_fmt_send == 0 ? TRUE : FALSE);
	EnableWindow(GetDlgItem(msg.hWndMain, IDC_CHECK_USE_ESCAPE_CHAR), comm.data_fmt_send == 0 ? TRUE : FALSE);

	ShowWindow(msg.hEditRecv, comm.data_fmt_recv > 0);
	ShowWindow(msg.hEditRecv2, comm.data_fmt_recv == 0);
	SetDlgItemText(msg.hWndMain, IDC_STATIC_RECV, comm.data_fmt_recv ? "���ݽ��� - 16����ģʽ" : "���ݽ��� - �ַ�ģʽ");
	SetDlgItemText(msg.hWndMain, IDC_STATIC_SEND, comm.data_fmt_send ? "���ݷ��� - 16����ģʽ" : "���ݷ��� - �ַ�ģʽ");

}
int	hardware_config(void)
{
	return 0;
}
// �ر��Ѿ��򿪵Ĵ���
int	close(int reason)
{
	DWORD dw;
	unsigned char* saved = NULL;
	int i = 0;
	if (comm.cchNotSend > 0 && reason == 0) {
		dw = utils.msgbox(msg.hWndMain, MB_ICONQUESTION | MB_YESNO, COMMON_NAME,
			"Ŀǰ���� %u �ֽڵ�����δ������,��ȷ��Ҫȡ����������?", comm.cchNotSend);
		if (dw == IDNO) {
			if (comm.cchNotSend == 0)
			{
				dw = utils.msgbox(msg.hWndMain, MB_ICONQUESTION | MB_YESNO, COMMON_NAME,
					"�����㶯��̫��,�ȴ��رյĹ����������Ѿ���ȫ������,���ڹرմ�����");
				if (dw == IDNO)
					return 0;
			}
			else
				return 0;
		}
	}
	// �ر�ǰ�����Ƿ���δ��ʾ������
	saved = (unsigned char*)deal.do_buf_recv(NULL, 0, 1);
	if (saved != NULL) {
		int ret = utils.msgbox(msg.hWndMain, MB_ICONQUESTION | MB_YESNOCANCEL, COMMON_NAME,
			"���ջ�����������δ��ʾ������ ,ȷ������ʾ�͹رմ�����\n\n"
			"	ѡ��		��:ֱ�ӹر�\n"
			"	ѡ��		��:��ʾ����,���رմ���\n"
			"	ѡ��		ȡ��:�����κβ���");
		if (ret == IDYES) {
			deal.do_buf_recv(NULL, 0, 3);
			SendMessage(msg.hWndMain, WM_COMMAND, MAKEWPARAM(IDC_BTN_STOPDISP, BN_CLICKED), (LPARAM)GetDlgItem(msg.hWndMain, IDC_BTN_STOPDISP));
		}
		else if (ret == IDNO) {
			SendMessage(msg.hWndMain, WM_COMMAND, MAKEWPARAM(IDC_BTN_STOPDISP, BN_CLICKED), (LPARAM)GetDlgItem(msg.hWndMain, IDC_BTN_STOPDISP));
			return 0;
		}
		else
			return 0;
	}
	SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "���ڹر�");
	EnableWindow(GetDlgItem(msg.hWndMain, IDC_BTN_OPEN), FALSE);

	//....
	comm.fCommOpened = FALSE;
	CloseHandle(msg.hComPort);
	msg.hComPort = INVALID_HANDLE_VALUE;

	EnableWindow(hComPort, TRUE);
	EnableWindow(hBaudRate, TRUE);
	EnableWindow(hParity, TRUE);
	EnableWindow(hDataSize, TRUE);
	EnableWindow(hStopBit, TRUE);
	EnableWindow(GetDlgItem(msg.hWndMain, IDC_BTN_OPEN), TRUE);
	SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "�򿪴���(&W)");

	return 1;
}
// ���´��򿪵Ĵ����豸����ز�������
// ����DCB�ṹ��
int	update_config(int only_update)
{
	int index;
	if (!only_update)
	{
		// COMMCONFIG ������DCB
		unsigned long size = sizeof(cconfig);
		if (!GetCommConfig(msg.hComPort, &cconfig, &size)) {
			utils.msgerr(msg.hWndMain, "ȡ�ô���Ĭ�����ó���");
			return 0;
		}
		if (!GetCommState(msg.hComPort, &cconfig.dcb))
		{
			utils.msgerr(msg.hWndMain, "ȡ�ô���״̬����");
			return 0;
		}
	}

	cconfig.dcb.fBinary = TRUE;

	// ������
	index = ComboBox_GetCurSel(hBaudRate);
	char s[128] = { 0 };
	ComboBox_GetLBText(hBaudRate, index, s);
	cconfig.dcb.BaudRate = atoi(s);

	// У��λ
	index = ComboBox_GetCurSel(hParity);
	cconfig.dcb.Parity = iParity[index];
	cconfig.dcb.fParity = (index == 0 ? FALSE : TRUE);
		
	// ���ݳ���
	index = ComboBox_GetCurSel(hDataSize);
	cconfig.dcb.ByteSize = iDataSize[index];

	// ֹͣλ
	index = ComboBox_GetCurSel(hStopBit);
	cconfig.dcb.StopBits = iStopBit[index];

	if (!only_update)
	{
		if (!SetCommConfig(msg.hComPort, &cconfig, sizeof(cconfig))) {
			utils.msgerr(msg.hWndMain, "COM���ô���");
			return 0;
		}
		if (!SetCommMask(msg.hComPort, EV_RXCHAR))
		{
			utils.msgerr(msg.hWndMain, "SetCommMask����");
			return 0;
		}
		// ��ʱ
		if (!SetCommTimeouts(msg.hComPort, &ctimeouts))
		{
			utils.msgerr(msg.hWndMain, "���ó�ʱ����! �����ص�Ĭ������!");
			return 0;
		}
		// ���������� ����:ͨ���豸���   ���뻺������С  �����������С
		if (!SetupComm(msg.hComPort, COMMON_READ_BUFFER_SIZE, COMMON_READ_BUFFER_SIZE))
		{
			utils.msgerr(msg.hWndMain, "SetupComm����");
			return 0;
		}
		// ��ֹ��д ��ջ�����
		PurgeComm(msg.hComPort, PURGE_RXABORT | PURGE_RXCLEAR);
		PurgeComm(msg.hComPort, PURGE_TXABORT | PURGE_TXCLEAR);
	}
	return 1;
}
int	switch_disp(void)
{
	return 0;
}