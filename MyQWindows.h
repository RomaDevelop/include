#ifndef MyQWindows_H
#define MyQWindows_H

#include <windows.h>

struct MyQWindows
{
	MessageBox(nullptr, QString("Can't make path " + QFileInfo(fileName).path()).toStdWString().c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
};

#endif