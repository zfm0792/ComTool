// Common.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Common.h"
#include "comm.h"
#include "debug.h"
#include "utils.h"
#include "deal.h"
#include "memoryManage.h"
#include "msg.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    MSG message;

	init_msg();
	init_utils();
	//init_about();
	init_comm();
	init_deal();
	init_memory();

#ifdef _DEBUG
	AllocConsole();
#endif
	debug_out(("程序已运行\n"));
	msg.run_app();

    // 主消息循环:
    while (GetMessage(&message, NULL, 0, 0))
    {
		//将一个WM_KEYDOWN或WM_SYSKEYDOWN消息翻译成一个WM_COMMAND或WM_SYSCOMMAND消息
		if (!TranslateAccelerator(msg.hWndMain, msg.hAccel, &message)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
    }
	debug_out(("程序已结束\n"));
#ifdef _DEBUG
	Sleep(1000);
	FreeConsole();
#endif

	//播放一个波形声音。
	MessageBeep(MB_OK);//播放由SystemDefault定义的声音

    return (int)message.wParam;
}