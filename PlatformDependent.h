#ifndef PlatformDependent_H
#define PlatformDependent_H

#include <QString>
#include <QDateTime>
#include <QWidget>

struct PlatformDependent
{
	static QDateTime GetProcessStartTime(uint processID);
	static bool IsProcessRunning(uint processID);

	enum CopyMoveFileMode { move, copy };
	static int CopyMoveFile(QString SourceFile, QString Destination, CopyMoveFileMode Mode); // требуется QT += widgets, работает и в консольном

	static void SetTopMost(QWidget *w, bool topMost);
	static void SetTopMostFlash(QWidget *w); // set topMost in true and immitietly sets false

	static void ShowPropertiesWindow(const QString &file);
};

#endif
