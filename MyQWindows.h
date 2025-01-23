#ifndef MYQWINDOWS_H
#define MYQWINDOWS_H

#include <QWidget>

struct MyQWindows
{
	static QDateTime GetProcessStartTime(uint processID);

	static bool IsProcessRunning(uint processID);
};

#endif // MYQWINDOWS_H
