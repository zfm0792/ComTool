#ifndef __DEAL_H__
#define __DEAL_H__

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#ifdef __cplusplus
extern "C" {
#endif

	void init_deal(void);

	// ������״̬
	enum{SEND_DATA_TYPE_NOTUSED,SEND_DATA_TYPE_USED,SEND_DATA_TYPE_MUSTFREE,SEND_DATA_TYPE_AUTO_USED,SEND_DATA_TYPE_AUTO_MUSTFREE};
	// ��������Ϊ
	enum { SEND_DATA_ACTION_GET, SEND_DATA_ACTION_RETURN, SEND_DATA_ACTION_INIT, SEND_DATA_ACTION_FREE, SEND_DATA_ACTION_RESET };

	typedef struct _SEND_DATA {
		DWORD cb;		// ��ǰ���ݰ��Ĵ�С
		DWORD data_size; // ��ǰ�����Ĵ��������ݵĴ�С
		int flag;		// ��Ӧ�����enum
		unsigned char data[10240];
	}SEND_DATA;

	struct deal_s {
		void(*do_check_recv_buf)(void);
		int(*do_buf_send)(int action, void* pv);
		int(*do_buf_recv)(unsigned char* chs, int cb, int action);
		void(*update_status)(char* str);
		void(*update_savebtn_status)(void);
		void(*cancel_auto_send)(int reason);
		void(*check_auto_send)(void);
		unsigned int(__stdcall *thread_read)(void* pv);
		unsigned int(__stdcall *thread_write)(void* pv);
		void* (*do_send)(int action);
		void(*add_send_packet)(SEND_DATA* psd);
		SEND_DATA* (*make_send_data)(int fmt, void*data, size_t size);
		void(*start_timer)(int start);
		void(*add_text)(unsigned char* ba, int cb);

		int last_show;
		// ��ʱ��
		unsigned int counter;

		struct
		{
			// ��д�ܵ�
			HANDLE hPipeRead;
			HANDLE hPipeWrite;
			//��д�߳�
			HANDLE hThreadRead;
			HANDLE hThreadWrite;
		}thread;

		void* autoptr;//�Զ�����ʱʹ�õ�����ָ�� 
		unsigned int timer_id;//�Զ�����ʱ�Ķ�ʱ��ID

		// �ٽ���
		CRITICAL_SECTION critical_section;

		// �������û���δ��ʾ������ʱ  �������read�߳�
		HANDLE hEventContinueToRead;
	};


#ifndef __DEAL_C__
	extern struct deal_s deal;
#endif

#ifdef __DEAL_C__
	int do_buf_send(int action, void* pv);
	int do_buf_recv(unsigned char* chs, int cb, int action);
	void do_check_recv_buf(void);

	void update_status(char* str);
	void update_savebtn_status(void);
	void cancel_auto_send(int reason);
	void check_auto_send(void);


	unsigned int __stdcall thread_read(void* pv);
	unsigned int __stdcall thread_write(void* pv);

	void* do_send(int action);

	void add_send_packet(SEND_DATA* psd);
	SEND_DATA* make_send_data(int fmt, void* data, size_t size);

	//void add_ch(unsigned char ch);
	void add_text(unsigned char* ba, int cb);

	void start_timer(int start);
#endif

#ifdef __cplusplus
}
#endif

#endif