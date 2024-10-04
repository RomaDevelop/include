#include "MyQWindows.h"

#include <windows.h>

#include <QWindow>

void MyQWindows::ActivateWindow1(QWidget * window)
{
	window->showMinimized();
	window->showNormal();
}

void MyQWindows::ActivateWindow2(QWidget * window)
{
	window->showNormal();
	auto hwnd = (HWND)window->windowHandle()->winId();
	SetForegroundWindow(hwnd);													// выводит на передний план
	SetWindowPos(hwnd,HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);			// ставит всегда поверх всех окон
	SetWindowPos(hwnd,HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);		// убирает всегда поверх всех окон
	SetActiveWindow(hwnd);
}
