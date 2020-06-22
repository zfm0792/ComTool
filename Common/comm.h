#ifndef __COMM_H__
#define __COMM_H__


#include <windows.h>
#include <SetupAPI.h>
#include <devguid.h>
#include <stdio.h>

void init_comm(void);

struct comm_s
{
	void(*init)(void);
	int(*update)(int* which);
	int(*open)(void);
	int(*close)(int reason);
	int(*save_to_file)(void);
	int(*load_from_file)(void);
	void(*set_data_fmt)(void);
	int(*hardware_config)(void);
	int(*show_pin_ctrl)(void);
	int(*show_timeouts)(void);
	int(*update_config)(int only_update);
	int(*switch_disp)(void);

	// 数据成员
	struct {
		// 发送/接收数据格式
		int data_fmt_send; // 0 - 字符  1 - 16进制
		int data_fmt_recv; // 0 - 字符  1 - 16进制

		// 忽略回车/字符转义
		int data_fmt_ignore_return;
		int data_fmt_use_escape_char;

		// 计数
		unsigned int cchSent;	  // 发送计数
		unsigned int cchReceived; // 接收计数
		unsigned int cchNotSend;  // 等待发送的数据量

		// 是否自动发送
		int fAutoSend;

		// 是否显示接收内容
		int fShowDataReceived;

		// 串口是否已经打开
		int fCommOpened;

		// 是否允许显示中文
		int fDisableChinese;

		DWORD data_count;
	};
};

#define COMMON_MAX_LOAD_SIZE			((unsigned long)1<<20)
#define COMMON_LINE_CCH_SEND			16
#define COMMON_LINE_CCH_RECV			16
#define COMMON_SEND_BUF_SIZE			COMMON_MAX_LOAD_SIZE
#define COMMON_RECV_BUF_SIZE			(((unsigned long)1<<20)*10)
#define COMMON_INTERNAL_RECV_BUF_SIZE	((unsigned long)1<<20)
#define COMMON_READ_BUFFER_SIZE			((unsigned long)1<<20)


#ifndef __COMM_C__
extern struct comm_s comm;
#endif

#ifdef __COMM_C__

static void		init(void);
static int		get_comm_list(int* which);
static int		open(void);
static int		save_to_file(void);
static int		load_from_file(void);
static void		set_data_fmt(void);
static int		hardware_config(void);
static int		close(int reason);
//static int		show_pin_ctrl(void);
//static int		show_timeouts(void);
static int		update_config(int only_update);
static int		switch_disp(void);

#endif

#endif
