#ifndef MYQWINDOWS_H
#define MYQWINDOWS_H

#include <QString>
#include <QDateTime>

struct MyQWindows
{
	static QDateTime GetProcessStartTime(uint processID);

	static bool IsProcessRunning(uint processID);

	enum CopyMoveFileMode { move, copy };
	static int CopyMoveFile(QString SourceFile, QString Destination, CopyMoveFileMode Mode); // требуется QT += widgets, работает и в консольном
};

#endif // MYQWINDOWS_H
