#define __DEAL_C__
#include "deal.h"
#include "msg.h"
#include "comm.h"
#include "about.h"
#include "utils.h"
#include "debug.h"
#include "memoryManage.h"
#include "resource.h"

#pragma comment(lib,"WinMM")

struct deal_s deal;

static char* __THIS_FILE__ = __FILE__;

void init_deal(void)
{
	memset(&deal, 0, sizeof(deal));
	deal.do_check_recv_buf = do_check_recv_buf;
	deal.do_buf_recv = do_buf_recv;
	deal.do_buf_send = do_buf_send;
	deal.update_savebtn_status = update_savebtn_status;
	deal.update_status = update_status;
	deal.thread_read = thread_read;
	deal.thread_write = thread_write;
	deal.cancel_auto_send = cancel_auto_send;
	deal.check_auto_send = check_auto_send;
	deal.do_send = do_send;
	deal.make_send_data = make_send_data;
	deal.add_send_packet = add_send_packet;
	deal.start_timer = start_timer;
	deal.add_text = add_text;
	deal.last_show = 1;
}

// ����δ�����͵�����
#define SEND_DATA_SIZE 101
int do_buf_send(int action, void* pv)
{
	static SEND_DATA* send_data[SEND_DATA_SIZE];
	int retval = 0;
	// ��ȡ �黹 ��λ ������
	if (action == SEND_DATA_ACTION_GET || action == SEND_DATA_ACTION_RETURN || action == SEND_DATA_ACTION_RESET)
	{
		EnterCriticalSection(&deal.critical_section);
	}
	switch (action)
	{
		case SEND_DATA_ACTION_INIT: // ��ʼ��
		{
			int it;
			int len = sizeof(SEND_DATA)*SEND_DATA_SIZE;
			void* pv = GET_MEM(len);
			if (!pv)
			{
				utils.msgbox(msg.hWndMain, MB_ICONERROR, NULL, "��ʼ��������ʧ��,�����´򿪳���");
				return 0;
			}
			for (it = 0; it < SEND_DATA_SIZE; it++)
			{
				send_data[it] = (SEND_DATA*)((unsigned char*)pv + it * sizeof(SEND_DATA));
				send_data[it]->flag = SEND_DATA_TYPE_NOTUSED;
			}
			InitializeCriticalSection(&deal.critical_section);
			return 1;
		}

		case SEND_DATA_ACTION_GET: // ȡ�û�����
		{

		}
		case SEND_DATA_ACTION_RETURN:// �黹������
		{

		}
		case SEND_DATA_ACTION_FREE:// �ͷ����л�����
		{
		}
		case SEND_DATA_ACTION_RESET: // ��λ����
		{

		}
	}
_exit_dbs:
	LeaveCriticalSection(&deal.critical_section);
	return retval;
}

// ����ֹͣ��ʾ�������
// chs:������ַ�ָ��
// cb:��������ݴ�С
// action�� 
//   0 -- ��ӻ����ڴ�
//   1 -- ȡ�û�������   unsigned char*
//   2 -- ȡ�û��������� int 
//   3 -- �ͷŻ������ڴ�
int do_buf_recv(unsigned char* chs, int cb, int action)
{
	static unsigned char* str = NULL;
	static unsigned char* pstr = NULL;

	if (str == NULL && action == 0)
	{
		str = (unsigned char*)GET_MEM(COMMON_INTERNAL_RECV_BUF_SIZE);
		if (str == NULL) return 0;
		pstr = str;
	}

	switch (action)
	{

	}
	return 1;
}
void do_check_recv_buf(void)
{

}

void update_status(char* str) 
{
#define STATUS_LENGTH 6

	static char status[128] = { " ״̬: " };
	static HWND hStatus;
	if (!hStatus)
		hStatus = GetDlgItem(msg.hWndMain, IDC_STATIC_STATUS);
	if (str == NULL) // ���¼���״̬
		sprintf(status + STATUS_LENGTH, "���ռ���:%u,���ͼ���:%u,�ȴ�����:%u", comm.cchReceived, comm.cchSent, comm.cchNotSend);
	else //���״̬���
		_snprintf(status + STATUS_LENGTH, sizeof(status) - STATUS_LENGTH, "%s", str);
	SetWindowText(hStatus, status);

#undef STATUS_LENGTH
}
// ���� �����ļ� ��ť״̬

void update_savebtn_status(void) 
{
	HWND hSave = GetDlgItem(msg.hWndMain, IDC_BTN_SAVEFILE);
	HWND hCopy = GetDlgItem(msg.hWndMain, IDC_BTN_COPY_RECV);

	BOOL bEnable = msg.hComPort == INVALID_HANDLE_VALUE || !msg.hComPort || !comm.fShowDataReceived;
	EnableWindow(hSave, bEnable);
	EnableWindow(hCopy, bEnable);
}

void cancel_auto_send(int reason)
{

}

void check_auto_send(void) 
{
	int flag;
	int elapse;
	BOOL fTranslated;

	flag = IsDlgButtonChecked(msg.hWndMain, IDC_CHK_AUTO_SEND);

	if (!flag) { // ���Զ�����
		deal.cancel_auto_send(0);
		return;
	}
	else { // �Զ�����

	}
}

// ������ȡ�������ݵĹ����߳�
unsigned int __stdcall thread_read(void* pv)
{
	DWORD nRead, nTotalRead = 0, nBytesToRead;
	unsigned char* block_data = NULL;
	BOOL retval;

	block_data = (unsigned char*)GET_MEM(COMMON_READ_BUFFER_SIZE);
	if (block_data == NULL) {
		utils.msgbox(msg.hWndMain, MB_ICONERROR, COMMON_NAME, "���߳̽���");
		return 1;
	}

	for (;;)
	{
		COMSTAT sta;
		DWORD comerr;
		int flag = 0;
		if (comm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE) {
			debug_out(("�����Ч  ���߳��˳�\n"));
			goto _exit;
		}

		// ������� �Լ����ڻ���
		ClearCommError(msg.hComPort, &comerr, &sta);
		if (sta.cbInQue == 0) {
			sta.cbInQue++;
		}

		nBytesToRead = sta.cbInQue;

		// �������һ���ֽ��������������е������ַ�
		if (nBytesToRead >= COMMON_READ_BUFFER_SIZE) {
			nBytesToRead = COMMON_READ_BUFFER_SIZE - 1;
		}

		nTotalRead = 0;

		for (;;)
		{
			for (; nTotalRead < nBytesToRead;) {
				retval = ReadFile(msg.hComPort, &block_data[0] + nTotalRead, nBytesToRead - nTotalRead, &nRead, NULL);

				if (comm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE) {
					debug_out(("ReadFile ��Ϊcomm.fCommOpend == FALSE �˳�\n"));
					goto _exit;
				}

				if (retval == FALSE) {
					InterlockedExchange((long volatile*)&comm.cchNotSend, 0); // comm.cchNotSend  = 0
					if (comm.fAutoSend) {
						deal.cancel_auto_send(0);
					}
					utils.msgerr(msg.hWndMain, "������ʱ��������!\n"
						"�Ƿ��ڰε�����֮ǰ�����˹رմ���??\n\n"
						"����ԭ��");
					SetTimer(msg.hWndMain, TIMER_ID_THREAD, 100, NULL);
					goto _exit;
				}

				if (nRead == 0) continue;

				// �Ӵ����ж�������������
				nTotalRead += nRead;
				// comm.cchReceived += nRead ���������ֽ�������¼��
				InterlockedExchangeAdd((long volatile*)&comm.cchReceived, nRead); 
				update_status(NULL);
			}
		} // end for

		debug_out(("����ȴ�...\n"));
		WaitForSingleObject(deal.hEventContinueToRead, INFINITE);
		debug_out(("�����ȴ�...\n"));
		// �����������ݷŵ�������ʾ��
		add_text(&block_data[0], nBytesToRead);
	}

_exit:
	if (block_data) {
		memory.free_mem((void**)&block_data, "���߳�");
	}
	debug_out(("���߳���Ȼ�˳�"));
	return 0;
}

// д�����豸 �߳�
unsigned int __stdcall thread_write(void* pv)
{
	DWORD nWritten, nRead, nWrittenData;

	SEND_DATA* psd = NULL;
	BOOL bRet;

	for (;;)
	{
		if (comm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE) {
			return 0;
		}

		// �ӹܵ��ｫҪд�����ݶ�ȡ�������浽psd��
		bRet = ReadFile(deal.thread.hPipeRead, (void*)&psd, 4, &nRead, NULL);
		if (bRet == FALSE)
		{
			if (!comm.fCommOpened || !deal.thread.hPipeRead)
				return 0;
			utils.msgerr(msg.hWndMain, "��ȡ�ܵ�ʱ����");
		}

		if (nRead != 4)
			continue;
		// Լ��ָ��ֵΪ0x00000001ʱ�˳��߳�
		if ((unsigned long)psd == 0x00000001)
		{
			debug_out(("�յ�������Ϊ1��ָ��,д�߳������˳�"));
			return 0;
		}

		nWrittenData = 0;
		while (nWrittenData < psd->data_size)
		{
			bRet = WriteFile(msg.hComPort, &psd->data[0] + nWrittenData, psd->data_size - nWrittenData, &nWritten, NULL);
			if (comm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE)
			{
				debug_out(("��Ϊcomm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE,д�߳��˳�"));
				return 0;
			}
			if (bRet == FALSE)
			{

			}

			if (nWritten == 0)
				continue;

			nWrittenData += nWritten;
			InterlockedExchangeAdd((volatile long*)&comm.cchSent, nWritten);
			InterlockedExchangeAdd((volatile long*)&comm.cchNotSend, -(LONG)nWritten);
			update_status(NULL);
		}

		if (psd->flag == SEND_DATA_TYPE_USED || psd->flag == SEND_DATA_TYPE_AUTO_USED)
			do_buf_send(SEND_DATA_ACTION_RETURN, (void*)psd);
		else if (psd->flag == SEND_DATA_TYPE_MUSTFREE || psd->flag == SEND_DATA_TYPE_AUTO_MUSTFREE)
			memory.free_mem((void**)&psd, "��д�������");
	}

	// �������������δ���ò����ľ���
	UNREFERENCED_PARAMETER(pv);
	debug_out(("�߳���Ȼ�˳�"));
	return 1;
}

int get_edit_data(int fmt, void** ppv, size_t* size)
{
	HWND hSend; // ���������ݵ�EditBox
	char* buff = NULL; // ��������͵�����
	size_t len; // buff�� len
	unsigned char* bytearray = NULL; // 16��������
	hSend = GetDlgItem(msg.hWndMain, IDC_EDIT_SEND);
	len = GetWindowTextLength(hSend);

	if (len == 0)
		return 0;

	buff = (char*)GET_MEM(len + 1);
	if (buff == NULL)
		return 0;
	GetWindowText(hSend, buff, len + 1);

	if (fmt) { // 16����

		int ret;

	}
	else { // �ַ���ʽ
		len = utils.wstr2lstr(buff);
		--len;

		if (comm.data_fmt_ignore_return) { // ���Իس�
			len = utils.remove_string_return(buff);
		}
		if (comm.data_fmt_use_escape_char) { // ����ת��
			unsigned int ret = utils.parse_string_escape_char(buff);
			if (ret & 0x80000000) // ������С�� 0x80000000
			{
				len = ret & 0x7FFFFFFF; // �ȼ��� len = ret
			}
			else //����ת���ַ���������
			{
				if (comm.fAutoSend)
				{
					deal.cancel_auto_send(0);
				}
				len = ret & 0x7FFFFFFF;
				utils.msgbox(msg.hWndMain, MB_ICONEXCLAMATION, NULL, "����ת���ַ���ʱ��������!\n\n"
					"�ڵ�%d ���ַ��������ִ���!", len);
				memory.free_mem((void**)&buff, NULL);
				return 0;
			}
		}
	}
	// len 16���� /�ַ��������ճ���
	if (fmt)
	{
		*ppv = bytearray;
		memory.free_mem((void**)&buff, "");
	}
	else
	{
		*ppv = buff;
	}
	*size = len;
	return 1;
}

// �������ݵ�����
// 0 -- �ֶ����� 1 -- �Զ�����ʱ����(��һ�ε���  ȡ������) 2 -- �Զ����͵ĺ�������
void* do_send(int action)
{
	void *pv = NULL;
	size_t size = 0;
	SEND_DATA *psd = NULL;

	// �жϴ����Ƿ��
	if (msg.hComPort == INVALID_HANDLE_VALUE) {
		utils.msgbox(msg.hWndMain, MB_ICONERROR,NULL, "���ȴ򿪴����豸");
		return NULL;
	}

	// ȡ���� �ӱ༭��
	if (action == 0)
	{
		if (get_edit_data(comm.data_fmt_send, &pv, &size)) {
			psd = make_send_data(0, pv, size); // ��ȡ�������� ����SEND_DATA��ʽ
			memory.free_mem((void**)&pv, "do_send_0");
			if (psd == NULL) return NULL;
			if (action == 0) {
				// ��ӷ��Ͱ�
				add_send_packet(psd);
			}
			else if (action == 1)
				return psd;
		}
		return NULL;
	}
	return NULL;
}



void add_send_packet(SEND_DATA* psd)
{
	DWORD nWritten = 0;
	if (WriteFile(deal.thread.hPipeWrite, &psd, 4, &nWritten, NULL) && nWritten == sizeof(psd)) {
		InterlockedExchangeAdd((volatile long*)&comm.cchNotSend, psd->data_size);
	}
	else {
		utils.msgerr(NULL, "��ӷ������ݰ�ʱ����");
	}
	update_status(NULL);
}
// ���ڴ湹��SEND_DATA���ݰ�
// fmt data���ݵĸ�ʽ
// 0 -- 16���ƻ��ַ�   
// data -- ָ����ͨ�ڴ�
// size -- ���ݴ�С
SEND_DATA* make_send_data(int fmt, void* data, size_t size)
{
	SEND_DATA* psd = NULL;
	int is_buffer_enough = 0;

	if (fmt == 0)
	{

	}
	else if(fmt == 1)
	{
		is_buffer_enough = size <= sizeof(SEND_DATA);
		if (is_buffer_enough) {
			// ��δ���͵Ļ���ȡ����
			psd = (SEND_DATA*)deal.do_buf_send(SEND_DATA_ACTION_GET, NULL);
			// ��ȡ��ǰ���ݰ��Ĵ�С
			// ��ȡdata����ʼ��ַ
			// sizeof(SEND_DATA) - sizeof(((SEND_DATA*)NULL)->data)  
			if (psd) psd->cb = sizeof(SEND_DATA) - sizeof(((SEND_DATA*)NULL)->data) + size;
		}
		else
		{
			psd = (SEND_DATA*)GET_MEM(size);
			if (psd)
				psd->cb = size;
		}
	}
	
	if (!psd)
	{
		deal.cancel_auto_send(0);
		utils.msgbox(msg.hWndMain, MB_ICONEXCLAMATION, "��ȵ�...", "�����ٶȹ���,�ڲ����ͻ�����û�п���!\n\n"
			"����ѿ����Զ�����,���Զ����ͱ�ȡ��!");
		return NULL;
	}

	if(fmt == 0)
	{ 

	}
	else if (fmt == 1)
	{
		memcpy(psd, deal.autoptr, ((SEND_DATA*)deal.autoptr)->cb);
		psd->flag = is_buffer_enough ? SEND_DATA_TYPE_AUTO_USED : SEND_DATA_TYPE_AUTO_MUSTFREE;
	}
	return psd;
}

//���baָ������ݵ���ʾ��(16���ƺ��ַ�ģʽ)
// 16���Ƶ�ģʽ �����ַ�ģʽ
// cb:��ʾ�����ݴ�С
// ba:����ʾ���ַ���
void add_text(unsigned char* ba, int cb)
{
	static char inner_str[10240];
	if (cb == 0) return;

	// ��ʾ��������  --- ��ͣ��ʾ
	if (comm.fShowDataReceived) {
		if (comm.data_fmt_recv) {// 16����

		}
		else{ // �ַ�
			char* str = NULL;
			char* p = NULL;
			int len;
			if (comm.fDisableChinese) {// ��������ʾ���� >0x7f ���ַ��ĳ�? 
				int it;
				unsigned char uch;
				for (it = 0; it < cb; it++) {
					uch = ba[it];
					if (uch == '\r') {
						ba[it] = 0;
						uch = '\0';
					}

					if (uch > 0 && uch < 32 && uch != '\n' || uch > 0x7f) {
						ba[it] = (unsigned char)'?';
					}
				}
			}
			str = utils.hex2chs(ba, cb, inner_str, __ARRAY_SIZE(inner_str));
			p = str;
			// ���ַ�������
			for (;;)
			{
				__try {// �Թ�ǰ���'\0'
					while (!*p)
						p++;
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					utils.msgbox(msg.hWndMain, MB_ICONERROR, COMMON_NAME,
						"utils.hex2chs:�ڴ�����쳣,�뱨�����!\n\n"
						"str=0x%08X", p);
					p = 2 + str + cb;
				}
				len = Edit_GetTextLength(msg.hEditRecv2);
				Edit_SetSel(msg.hEditRecv2, len, len);
				Edit_ReplaceSel(msg.hEditRecv2, p);
				//SetDlgItemText(msg.hWndMain,IDC_EDIT_SEND,p);
				//MessageBox(NULL,p,NULL,0);
				while (*p++)//��λ����2��'\0',����hex2chsת����Ľ�β�ذ�������'\0'+һ��'x'
					;
			}
			if (str != inner_str) memory.free_mem((void**)&str, NULL);
		}
	}
	else {
		do_buf_recv(ba, cb, 0);
	}
}

// ������ʱ��   time �Ļص�����
static void __stdcall TimerProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	unsigned char* second, *minute, *hour;
	char str[9];

	second = (unsigned char*)((unsigned long)&deal.counter + 0);
	minute = (unsigned char*)((unsigned long)&deal.counter + 1);
	hour = (unsigned char*)((unsigned long)&deal.counter + 2);

	if (++*second == 60) {
		*second = 0;
		if (++*minute == 60) {
			*minute = 0;
			if (++*hour == 60) {
				*hour = 0;
			}
		}
	}

	sprintf(&str[0], "%02d:%02d:%02d", *hour, *minute, *second);
	SetDlgItemText(msg.hWndMain, IDC_STATIC_TIMER, str);
	UNREFERENCED_PARAMETER(uID);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(dwUser);
	UNREFERENCED_PARAMETER(dw1);
	UNREFERENCED_PARAMETER(dw2);
}

void start_timer(int start) 
{
	static UINT timer_id;
	if (start) {
		InterlockedExchange((volatile long*)&deal.counter, 0);
		SetDlgItemText(msg.hWndMain, IDC_STATIC_TIMER, "00:00:00");
		timer_id = timeSetEvent(1000, 0, TimerProc, 0, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
		if (timer_id == 0)
		{
			//...
		}
	}
	else {
		if (timer_id) {
			timeKillEvent(timer_id);
			timer_id = 0;
		}
	}
}
