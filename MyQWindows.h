#ifndef MYQWINDOWS_H
#define MYQWINDOWS_H

#include <QWidget>
#include <QString>

struct MyQWindows
{
	static QDateTime GetProcessStartTime(uint processID);

	static bool IsProcessRunning(uint processID);

	enum CopyMoveFileMode { move, copy };
	static int CopyMoveFile(QString S, QString D, CopyMoveFileMode Mode);
};

#endif // MYQWINDOWS_H
