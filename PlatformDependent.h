#ifndef PlatformDependent_H
#define PlatformDependent_H

#include <QString>
#include <QDateTime>
#include <QWidget>

#include "declare_struct.h"

struct PlatformDependent
{
	static QDateTime GetProcessStartTime(uint processID);
	static bool IsProcessRunning(uint processID);

	enum CopyMoveFileMode { move, copy };
	declare_struct_2_fields_no_move(CopyMoveFileRes, bool, success, int, errorCode);
	static CopyMoveFileRes CopyMoveFile(QString SourceFile, QString Destination, CopyMoveFileMode Mode);

	static void SetTopMost(QWidget *w, bool topMost);
	static void SetTopMostFlash(QWidget *w); // set topMost in true and immitietly sets false

	static void ShowPropertiesWindow(const QString &file);
};

#endif
