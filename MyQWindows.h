#ifndef MYQWINDOWS_H
#define MYQWINDOWS_H

#include <QWidget>

struct MyQWindows
{
	static void ActivateWindow(QWidget *window);
	//MessageBox(nullptr, QString("Can't make path " + QFileInfo(fileName).path()).toStdWString().c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
};

#endif // MYQWINDOWS_H
