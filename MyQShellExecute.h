//---------------------------------------------------------------------------
#ifndef MyQShellExecute_H
#define MyQShellExecute_H
//---------------------------------------------------------------------------
#include <windows.h>
#include <QString>
#include <QFileInfo>
//---------------------------------------------------------------------------
struct MyQShellExecute
{
    inline static void ShellExecuteFile(QString file, QString params = "", int SW_MODE = SW_NORMAL)
    {
	ShellExecute(NULL, NULL, file.toStdWString().c_str(), params.toStdWString().c_str(), NULL, SW_MODE);
    }
    inline static void ShellExecutePath(QString file, int SW_MODE = SW_NORMAL)
    {
	QFileInfo fileInfo(file);
	QString command = QString("/select, \"") + fileInfo.fileName() + "\"";

	ShellExecute(NULL, L"open", L"explorer.exe", command.toStdWString().c_str(), fileInfo.path().toStdWString().c_str(), SW_MODE);
    }
};
//---------------------------------------------------------------------------
#endif  // MyQShellExecute
