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

// 串口设置数据
char* cBaudRate[] = { "110","300","600","1200","2400","4800","9600","14400","19200","38400","57600","115200","128000","256000",NULL };
DWORD iBaudRate[] = { CBR_110,CBR_300,CBR_600,CBR_1200,CBR_2400,CBR_4800,CBR_9600,CBR_14400,CBR_19200,CBR_38400,CBR_57600,CBR_115200,CBR_128000,CBR_256000 };
char *cParity[] = { "无校验","偶校验","奇校验","标记校验","空格校验",NULL };
BYTE iParity[] = { NOPARITY,EVENPARITY,ODDPARITY,MARKPARITY,SPACEPARITY };
char* cStopBit[] = { "1位","1.5位","2位",NULL };
BYTE iStopBit[] = { ONESTOPBIT,ONE5STOPBITS,TWOSTOPBITS };
char* cDataSize[] = { "8位","7位","6位","5位",NULL };
BYTE iDataSize[] = { 8,7,6,5 };

// 控件句柄
HWND hWndMain;
HWND hComPort;
HWND hBaudRate;
HWND hParity;
HWND hDataSize;
HWND hStopBit;

// 串口相关的结构体
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

// 在程序初始化时做初始化操作   被WM_CREATE调用
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

	_SETLIST(cBaudRate, hBaudRate, 6); // 9600开始
	ComboBox_AddString(hBaudRate, "<输入>");
	_SETLIST(cStopBit, hStopBit, 0);
	_SETLIST(cParity, hParity, 0);
	_SETLIST(cDataSize, hDataSize, 0);
#undef _SETLIST
	//CheckRadioButton: 
	CheckRadioButton(hWndMain, IDC_RADIO_SEND_HEX,IDC_RADIO_SEND_CHAR,IDC_RADIO_SEND_HEX);
	CheckRadioButton(hWndMain, IDC_RADIO_RECV_HEX, IDC_RADIO_RECV_CHAR, IDC_RADIO_RECV_HEX);
	// 设置自动发送时间
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

//更新系统串口设备到串口列表
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
		return com_list.com_id[(int)which]; // 返回com id
	else if ((int)which == -1)
	{
		now_sel = ComboBox_GetCurSel(hComPort);
		if (now_sel != -1)
			now_sel = com_list.com_id[now_sel];
	}
	else if ((int)which >= MAX_COM && (int)which < 2 * MAX_COM) // 返回com的名字
		return (int)com_list.com_name[(int)which - MAX_COM];

	// 遍历串口设备
	com_list.count = 0;
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		utils.msgerr(msg.hWndMain, "SetupApi");
		com_list.count = 0;
		return 0;
	}

	//枚举指定设备信息集合的成员，并将数据放在PSP_DEVINFO_DATA中
	// hDevInfo:设备信息集合的句柄
	// it:要取得的设备信息成员序号，从0开始
	spdata.cbSize = sizeof(spdata);
	for (it = 0; SetupDiEnumDeviceInfo(hDevInfo, it, &spdata); it++)
	{
		//在前面得到的指向某一个具体设备信息集合的指针中取出某一项信息
		if (SetupDiGetDeviceRegistryProperty(hDevInfo, &spdata, SPDRP_FRIENDLYNAME, NULL,
			(PBYTE)&com_list.com_name[com_index][0], sizeof(com_list.com_name[0]), NULL))
		{
			char* pch = NULL;
			int tmp_id = 0;
			pch = strstr(&com_list.com_name[com_index][0], "LPT");
			if (pch) continue;//并口

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
		_snprintf(str, __ARRAY_SIZE(str), "共找到串口设备 %d 个\n", com_index);
		debug_out((str));
	}
	return 1;
}
// 打开串口
int	open(void)
{
	char str[64];
	int index = ComboBox_GetCurSel(hComPort);
	if (index == CB_ERR)
	{
		utils.msgbox(msg.hWndMain, MB_ICONINFORMATION, COMMON_NAME, "没有任何可用的串口");
		return 0;
	}
	index = get_comm_list((int*)index);
	sprintf(str, "\\\\.\\COM%d", index);

	// 通过文件api 打开串口
	msg.hComPort = CreateFile(str, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (msg.hComPort == INVALID_HANDLE_VALUE) {
		DWORD lasterror = GetLastError();
		utils.msgerr(msg.hWndMain, "错误");
		if (lasterror == ERROR_FILE_NOT_FOUND)
			comm.update((int*)-1);
		return 0;
	}
	if (!update_config(0)) { // 配置com的参数
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
	SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "关闭串口");
	deal.update_status("串口已打开");
	comm.fCommOpened = 1;

	// 读写管道,读写线程,线程同步
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
// 取得当前发送、接收数据的格式
void set_data_fmt(void)
{
	// 忽略回车换行
	comm.data_fmt_ignore_return = IsDlgButtonChecked(msg.hWndMain, IDC_CHECK_IGNORE_RETURN);
	// 接收16进制
	comm.data_fmt_send = IsDlgButtonChecked(msg.hWndMain, IDC_RADIO_SEND_HEX);
	// 发送16进制
	comm.data_fmt_recv = IsDlgButtonChecked(msg.hWndMain, IDC_RADIO_RECV_HEX);
	// 使用转义字符
	comm.data_fmt_use_escape_char = IsDlgButtonChecked(msg.hWndMain, IDC_CHECK_USE_ESCAPE_CHAR);

	EnableWindow(GetDlgItem(msg.hWndMain, IDC_CHECK_IGNORE_RETURN), comm.data_fmt_send == 0 ? TRUE : FALSE);
	EnableWindow(GetDlgItem(msg.hWndMain, IDC_CHECK_USE_ESCAPE_CHAR), comm.data_fmt_send == 0 ? TRUE : FALSE);

	ShowWindow(msg.hEditRecv, comm.data_fmt_recv > 0);
	ShowWindow(msg.hEditRecv2, comm.data_fmt_recv == 0);
	SetDlgItemText(msg.hWndMain, IDC_STATIC_RECV, comm.data_fmt_recv ? "数据接收 - 16进制模式" : "数据接收 - 字符模式");
	SetDlgItemText(msg.hWndMain, IDC_STATIC_SEND, comm.data_fmt_send ? "数据发送 - 16进制模式" : "数据发送 - 字符模式");

}
int	hardware_config(void)
{
	return 0;
}
// 关闭已经打开的串口
int	close(int reason)
{
	DWORD dw;
	unsigned char* saved = NULL;
	int i = 0;
	if (comm.cchNotSend > 0 && reason == 0) {
		dw = utils.msgbox(msg.hWndMain, MB_ICONQUESTION | MB_YESNO, COMMON_NAME,
			"目前还有 %u 字节的数据未被发送,您确定要取消发送它们?", comm.cchNotSend);
		if (dw == IDNO) {
			if (comm.cchNotSend == 0)
			{
				dw = utils.msgbox(msg.hWndMain, MB_ICONQUESTION | MB_YESNO, COMMON_NAME,
					"由于你动作太慢,等待关闭的过程中数据已经被全部发送,现在关闭串口吗？");
				if (dw == IDNO)
					return 0;
			}
			else
				return 0;
		}
	}
	// 关闭前提醒是否有未显示的数据
	saved = (unsigned char*)deal.do_buf_recv(NULL, 0, 1);
	if (saved != NULL) {
		int ret = utils.msgbox(msg.hWndMain, MB_ICONQUESTION | MB_YESNOCANCEL, COMMON_NAME,
			"接收缓冲区还存在未显示的数据 ,确定不显示就关闭串口吗？\n\n"
			"	选择		是:直接关闭\n"
			"	选择		否:显示数据,不关闭串口\n"
			"	选择		取消:不作任何操作");
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
	SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "正在关闭");
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
	SetDlgItemText(msg.hWndMain, IDC_BTN_OPEN, "打开串口(&W)");

	return 1;
}
// 更新待打开的串口设备的相关操作设置
// 更新DCB结构体
int	update_config(int only_update)
{
	int index;
	if (!only_update)
	{
		// COMMCONFIG 包含了DCB
		unsigned long size = sizeof(cconfig);
		if (!GetCommConfig(msg.hComPort, &cconfig, &size)) {
			utils.msgerr(msg.hWndMain, "取得串口默认配置出错");
			return 0;
		}
		if (!GetCommState(msg.hComPort, &cconfig.dcb))
		{
			utils.msgerr(msg.hWndMain, "取得串口状态错误");
			return 0;
		}
	}

	cconfig.dcb.fBinary = TRUE;

	// 波特率
	index = ComboBox_GetCurSel(hBaudRate);
	char s[128] = { 0 };
	ComboBox_GetLBText(hBaudRate, index, s);
	cconfig.dcb.BaudRate = atoi(s);

	// 校验位
	index = ComboBox_GetCurSel(hParity);
	cconfig.dcb.Parity = iParity[index];
	cconfig.dcb.fParity = (index == 0 ? FALSE : TRUE);
		
	// 数据长度
	index = ComboBox_GetCurSel(hDataSize);
	cconfig.dcb.ByteSize = iDataSize[index];

	// 停止位
	index = ComboBox_GetCurSel(hStopBit);
	cconfig.dcb.StopBits = iStopBit[index];

	if (!only_update)
	{
		if (!SetCommConfig(msg.hComPort, &cconfig, sizeof(cconfig))) {
			utils.msgerr(msg.hWndMain, "COM配置错误");
			return 0;
		}
		if (!SetCommMask(msg.hComPort, EV_RXCHAR))
		{
			utils.msgerr(msg.hWndMain, "SetCommMask错误");
			return 0;
		}
		// 超时
		if (!SetCommTimeouts(msg.hComPort, &ctimeouts))
		{
			utils.msgerr(msg.hWndMain, "设置超时错误! 请检查或回到默认设置!");
			return 0;
		}
		// 缓冲区分配 参数:通信设备句柄   输入缓冲区大小  输出缓冲区大小
		if (!SetupComm(msg.hComPort, COMMON_READ_BUFFER_SIZE, COMMON_READ_BUFFER_SIZE))
		{
			utils.msgerr(msg.hWndMain, "SetupComm错误");
			return 0;
		}
		// 终止读写 清空缓冲区
		PurgeComm(msg.hComPort, PURGE_RXABORT | PURGE_RXCLEAR);
		PurgeComm(msg.hComPort, PURGE_TXABORT | PURGE_TXCLEAR);
	}
	return 1;
}
int	switch_disp(void)
{
	return 0;
}