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

	// ���ݳ�Ա
	struct {
		// ����/�������ݸ�ʽ
		int data_fmt_send; // 0 - �ַ�  1 - 16����
		int data_fmt_recv; // 0 - �ַ�  1 - 16����

		// ���Իس�/�ַ�ת��
		int data_fmt_ignore_return;
		int data_fmt_use_escape_char;

		// ����
		unsigned int cchSent;	  // ���ͼ���
		unsigned int cchReceived; // ���ռ���
		unsigned int cchNotSend;  // �ȴ����͵�������

		// �Ƿ��Զ�����
		int fAutoSend;

		// �Ƿ���ʾ��������
		int fShowDataReceived;

		// �����Ƿ��Ѿ���
		int fCommOpened;

		// �Ƿ�������ʾ����
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
