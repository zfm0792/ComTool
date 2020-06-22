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

// 缓冲未被发送的数据
#define SEND_DATA_SIZE 101
int do_buf_send(int action, void* pv)
{
	static SEND_DATA* send_data[SEND_DATA_SIZE];
	int retval = 0;
	// 获取 归还 复位 缓冲区
	if (action == SEND_DATA_ACTION_GET || action == SEND_DATA_ACTION_RETURN || action == SEND_DATA_ACTION_RESET)
	{
		EnterCriticalSection(&deal.critical_section);
	}
	switch (action)
	{
		case SEND_DATA_ACTION_INIT: // 初始化
		{
			int it;
			int len = sizeof(SEND_DATA)*SEND_DATA_SIZE;
			void* pv = GET_MEM(len);
			if (!pv)
			{
				utils.msgbox(msg.hWndMain, MB_ICONERROR, NULL, "初始化缓冲区失败,请重新打开程序");
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

		case SEND_DATA_ACTION_GET: // 取得缓冲区
		{

		}
		case SEND_DATA_ACTION_RETURN:// 归还缓冲区
		{

		}
		case SEND_DATA_ACTION_FREE:// 释放所有缓冲区
		{
		}
		case SEND_DATA_ACTION_RESET: // 复位所有
		{

		}
	}
_exit_dbs:
	LeaveCriticalSection(&deal.critical_section);
	return retval;
}

// 缓冲停止显示后的数据
// chs:缓冲的字符指针
// cb:缓冲的数据大小
// action： 
//   0 -- 添加缓冲内存
//   1 -- 取得缓冲内容   unsigned char*
//   2 -- 取得缓冲区长度 int 
//   3 -- 释放缓冲区内存
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

	static char status[128] = { " 状态: " };
	static HWND hStatus;
	if (!hStatus)
		hStatus = GetDlgItem(msg.hWndMain, IDC_STATIC_STATUS);
	if (str == NULL) // 更新计数状态
		sprintf(status + STATUS_LENGTH, "接收计数:%u,发送计数:%u,等待发送:%u", comm.cchReceived, comm.cchSent, comm.cchNotSend);
	else //输出状态语句
		_snprintf(status + STATUS_LENGTH, sizeof(status) - STATUS_LENGTH, "%s", str);
	SetWindowText(hStatus, status);

#undef STATUS_LENGTH
}
// 更新 保存文件 按钮状态

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

	if (!flag) { // 不自动发送
		deal.cancel_auto_send(0);
		return;
	}
	else { // 自动发送

	}
}

// 用来读取串口数据的工作线程
unsigned int __stdcall thread_read(void* pv)
{
	DWORD nRead, nTotalRead = 0, nBytesToRead;
	unsigned char* block_data = NULL;
	BOOL retval;

	block_data = (unsigned char*)GET_MEM(COMMON_READ_BUFFER_SIZE);
	if (block_data == NULL) {
		utils.msgbox(msg.hWndMain, MB_ICONERROR, COMMON_NAME, "读线程结束");
		return 1;
	}

	for (;;)
	{
		COMSTAT sta;
		DWORD comerr;
		int flag = 0;
		if (comm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE) {
			debug_out(("句柄无效  读线程退出\n"));
			goto _exit;
		}

		// 清除错误 以及串口缓冲
		ClearCommError(msg.hComPort, &comerr, &sta);
		if (sta.cbInQue == 0) {
			sta.cbInQue++;
		}

		nBytesToRead = sta.cbInQue;

		// 保留最后一个字节用来容纳期望中的中文字符
		if (nBytesToRead >= COMMON_READ_BUFFER_SIZE) {
			nBytesToRead = COMMON_READ_BUFFER_SIZE - 1;
		}

		nTotalRead = 0;

		for (;;)
		{
			for (; nTotalRead < nBytesToRead;) {
				retval = ReadFile(msg.hComPort, &block_data[0] + nTotalRead, nBytesToRead - nTotalRead, &nRead, NULL);

				if (comm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE) {
					debug_out(("ReadFile 因为comm.fCommOpend == FALSE 退出\n"));
					goto _exit;
				}

				if (retval == FALSE) {
					InterlockedExchange((long volatile*)&comm.cchNotSend, 0); // comm.cchNotSend  = 0
					if (comm.fAutoSend) {
						deal.cancel_auto_send(0);
					}
					utils.msgerr(msg.hWndMain, "读串口时遇到错误!\n"
						"是否在拔掉串口之前忘记了关闭串口??\n\n"
						"错误原因");
					SetTimer(msg.hWndMain, TIMER_ID_THREAD, 100, NULL);
					goto _exit;
				}

				if (nRead == 0) continue;

				// 从串口中读到的数据总量
				nTotalRead += nRead;
				// comm.cchReceived += nRead 将读出的字节数量记录到
				InterlockedExchangeAdd((long volatile*)&comm.cchReceived, nRead); 
				update_status(NULL);
			}
		} // end for

		debug_out(("进入等待...\n"));
		WaitForSingleObject(deal.hEventContinueToRead, INFINITE);
		debug_out(("结束等待...\n"));
		// 将读到的数据放到数据显示区
		add_text(&block_data[0], nBytesToRead);
	}

_exit:
	if (block_data) {
		memory.free_mem((void**)&block_data, "读线程");
	}
	debug_out(("读线程自然退出"));
	return 0;
}

// 写串口设备 线程
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

		// 从管道里将要写的数据读取出来保存到psd中
		bRet = ReadFile(deal.thread.hPipeRead, (void*)&psd, 4, &nRead, NULL);
		if (bRet == FALSE)
		{
			if (!comm.fCommOpened || !deal.thread.hPipeRead)
				return 0;
			utils.msgerr(msg.hWndMain, "读取管道时错误");
		}

		if (nRead != 4)
			continue;
		// 约定指针值为0x00000001时退出线程
		if ((unsigned long)psd == 0x00000001)
		{
			debug_out(("收到了数据为1的指针,写线程正在退出"));
			return 0;
		}

		nWrittenData = 0;
		while (nWrittenData < psd->data_size)
		{
			bRet = WriteFile(msg.hComPort, &psd->data[0] + nWrittenData, psd->data_size - nWrittenData, &nWritten, NULL);
			if (comm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE)
			{
				debug_out(("因为comm.fCommOpened == FALSE || msg.hComPort == INVALID_HANDLE_VALUE,写线程退出"));
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
			memory.free_mem((void**)&psd, "被写完的数据");
	}

	// 避免编译器关于未引用参数的警告
	UNREFERENCED_PARAMETER(pv);
	debug_out(("线程自然退出"));
	return 1;
}

int get_edit_data(int fmt, void** ppv, size_t* size)
{
	HWND hSend; // 待发送内容的EditBox
	char* buff = NULL; // 保存待发送的内容
	size_t len; // buff的 len
	unsigned char* bytearray = NULL; // 16进制数组
	hSend = GetDlgItem(msg.hWndMain, IDC_EDIT_SEND);
	len = GetWindowTextLength(hSend);

	if (len == 0)
		return 0;

	buff = (char*)GET_MEM(len + 1);
	if (buff == NULL)
		return 0;
	GetWindowText(hSend, buff, len + 1);

	if (fmt) { // 16进制

		int ret;

	}
	else { // 字符方式
		len = utils.wstr2lstr(buff);
		--len;

		if (comm.data_fmt_ignore_return) { // 忽略回车
			len = utils.remove_string_return(buff);
		}
		if (comm.data_fmt_use_escape_char) { // 忽略转义
			unsigned int ret = utils.parse_string_escape_char(buff);
			if (ret & 0x80000000) // 数据量小于 0x80000000
			{
				len = ret & 0x7FFFFFFF; // 等价于 len = ret
			}
			else //解析转义字符遇到错误
			{
				if (comm.fAutoSend)
				{
					deal.cancel_auto_send(0);
				}
				len = ret & 0x7FFFFFFF;
				utils.msgbox(msg.hWndMain, MB_ICONEXCLAMATION, NULL, "解析转义字符串时遇到错误!\n\n"
					"在第%d 个字符附近出现错误!", len);
				memory.free_mem((void**)&buff, NULL);
				return 0;
			}
		}
	}
	// len 16进制 /字符数据最终长度
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

// 发送数据到串口
// 0 -- 手动发送 1 -- 自动发送时调用(第一次调用  取得数据) 2 -- 自动发送的后续调用
void* do_send(int action)
{
	void *pv = NULL;
	size_t size = 0;
	SEND_DATA *psd = NULL;

	// 判断串口是否打开
	if (msg.hComPort == INVALID_HANDLE_VALUE) {
		utils.msgbox(msg.hWndMain, MB_ICONERROR,NULL, "请先打开串口设备");
		return NULL;
	}

	// 取数据 从编辑框
	if (action == 0)
	{
		if (get_edit_data(comm.data_fmt_send, &pv, &size)) {
			psd = make_send_data(0, pv, size); // 将取出的数据 构建SEND_DATA格式
			memory.free_mem((void**)&pv, "do_send_0");
			if (psd == NULL) return NULL;
			if (action == 0) {
				// 添加发送包
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
		utils.msgerr(NULL, "添加发送数据包时出错");
	}
	update_status(NULL);
}
// 从内存构建SEND_DATA数据包
// fmt data数据的格式
// 0 -- 16进制或字符   
// data -- 指向普通内存
// size -- 数据大小
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
			// 从未发送的缓冲取数据
			psd = (SEND_DATA*)deal.do_buf_send(SEND_DATA_ACTION_GET, NULL);
			// 获取当前数据包的大小
			// 获取data的起始地址
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
		utils.msgbox(msg.hWndMain, MB_ICONEXCLAMATION, "请等等...", "发送速度过快,内部发送缓冲区没有空闲!\n\n"
			"如果已开启自动发送,则自动发送被取消!");
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

//添加ba指向的数据到显示区(16进制和字符模式)
// 16进制的模式 还是字符模式
// cb:显示的数据大小
// ba:待显示的字符串
void add_text(unsigned char* ba, int cb)
{
	static char inner_str[10240];
	if (cb == 0) return;

	// 显示接收数据  --- 暂停显示
	if (comm.fShowDataReceived) {
		if (comm.data_fmt_recv) {// 16进制

		}
		else{ // 字符
			char* str = NULL;
			char* p = NULL;
			int len;
			if (comm.fDisableChinese) {// 不允许显示中文 >0x7f 的字符改成? 
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
			// 多字符串处理
			for (;;)
			{
				__try {// 略过前面的'\0'
					while (!*p)
						p++;
				}
				__except (EXCEPTION_EXECUTE_HANDLER) {
					utils.msgbox(msg.hWndMain, MB_ICONERROR, COMMON_NAME,
						"utils.hex2chs:内存访问异常,请报告错误!\n\n"
						"str=0x%08X", p);
					p = 2 + str + cb;
				}
				len = Edit_GetTextLength(msg.hEditRecv2);
				Edit_SetSel(msg.hEditRecv2, len, len);
				Edit_ReplaceSel(msg.hEditRecv2, p);
				//SetDlgItemText(msg.hWndMain,IDC_EDIT_SEND,p);
				//MessageBox(NULL,p,NULL,0);
				while (*p++)//定位到第2个'\0',所有hex2chs转换后的结尾必包含两个'\0'+一个'x'
					;
			}
			if (str != inner_str) memory.free_mem((void**)&str, NULL);
		}
	}
	else {
		do_buf_recv(ba, cb, 0);
	}
}

// 开启计时器   time 的回调函数
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
