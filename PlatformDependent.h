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
	declare_struct_3_fields_no_move(CopyMoveFileRes, bool, success, int, errorCode, QString, errorText);
	static CopyMoveFileRes CopyMoveFile(QString SourceFile, QString Destination, CopyMoveFileMode Mode);

	static void SetTopMost(QWidget *w, bool topMost);
	static void SetTopMostFlash(QWidget *w); // set topMost in true and immitietly sets false

	static void FlashClickOnTitle(QWidget *w); // делает клик на заголовок окна и возвращает курсор обратно

	static void ShowPropertiesWindow(const QString &file);

	struct CreatePRVWRes { QString error; QImage image; };
	static CreatePRVWRes CreatePRVW(const QString& file_to_get_prvw);
	// requires LIBS += -luuid # -lshell32 и -lole32 не понадобились, хотя ИИ писал, что нужны
};

#endif
