//---------------------------------------------------------------------------
#ifndef MyQShellExecute_H
#define MyQShellExecute_H
//---------------------------------------------------------------------------
void ShellExecutePath(QString file)
{
	QFileInfo fileI(file);
	QString command = QString("/select, \"") + fileI.fileName() + "\"";

	ShellExecute(NULL, L"open", L"explorer.exe", command.toStdWString().c_str(), fileI.path().toStdWString().c_str(), SW_NORMAL);
}
//---------------------------------------------------------------------------
#endif
