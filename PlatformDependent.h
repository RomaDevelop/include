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

	static bool OpenWithDialog(const QString &filePath);

	static bool IsFileLocked(const QString &file);

	enum CopyMoveFileMode { move, copy };
	declare_struct_3_fields_no_move(CopyMoveFileRes, bool, success, int, errorCode, QString, errorText);
	static CopyMoveFileRes CopyMoveFile(QString SourceFile, QString Destination, CopyMoveFileMode Mode);

	static bool IsPathOnFixedDrive(QString path);

	static void SetTopMost(QWidget *w, bool topMost = true);
	static void SetTopMostFlash(QWidget *w); // set topMost in true and immediately sets false

	static void FlashClickOnTitle(QWidget *w); // clicks on the window title and returns cursor back

	static void ShowPropertiesWindow(const QString &file);
};

#endif
