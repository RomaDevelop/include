//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MyQExecute_H
#define MyQExecute_H
//------------------------------------------------------------------------------------------------------------------------------------------
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
//------------------------------------------------------------------------------------------------------------------------------------------
struct MyQExecute
{
	inline static bool Execute(QString file, QStringList args = {});
	inline static bool OpenDir(QString dir);
	inline static bool ShowInExplorer(QString fileOrDir);
};
//------------------------------------------------------------------------------------------------------------------------------------------
bool MyQExecute::Execute(QString file, QStringList args)
{
	#define ERROR_PREFIX "MyQExecute::Execute: объект " + file
	QFileInfo fileInfo(file);
	bool doOpenUrl = false;

	if(!fileInfo.exists()) { qDebug() << ERROR_PREFIX + " не существует"; return false; }

	if(fileInfo.isSymLink()) doOpenUrl = true; // если ярлык - запустим через QDesktopServices
	else if(fileInfo.isFile())
	{
		if(fileInfo.isExecutable()) // Если файл исполняемый
		{
			bool startRes = QProcess::startDetached(file, args); // запускем через QProcess
			if(!startRes) qDebug() << ERROR_PREFIX + ": QProcess::startDetached returned false";
			return startRes;
		}
		else doOpenUrl = true; // если нет - запустим через QDesktopServices
	}

	if(doOpenUrl)
	{
		if(!args.isEmpty()) qDebug() << ERROR_PREFIX + " не является исполняемым, аргументы игнорируются";

		bool openRes = QDesktopServices::openUrl(QUrl::fromLocalFile(file));
		if(!openRes) qDebug() << ERROR_PREFIX + ": QDesktopServices::openUrl returned false";
		return openRes;
	}

	qDebug() << ERROR_PREFIX + " не был запущен, не известный формат";
	return false;
}

bool MyQExecute::OpenDir(QString dir)
{
	QFileInfo fileInfo(dir);
	if(!fileInfo.isSymLink() && fileInfo.isDir())
	{
		QStringList args;
		args << QDir::toNativeSeparators(dir);
		return QProcess::startDetached("explorer", args);
	}

	qDebug() << "MyQExecute::Execute: директория " + dir + " не обнаружена";
	return false;
}

bool MyQExecute::ShowInExplorer(QString fileOrDir)
{
	QFileInfo fileInfo(fileOrDir);
	if(!fileInfo.isSymLink() && (fileInfo.isFile() || fileInfo.isDir()))
	{
		QStringList args;
		args << "/select," << QDir::toNativeSeparators(fileOrDir);
		return QProcess::startDetached("explorer", args);
	}
	else
	{
		qCritical() << "MyQExecute::ShowInExplorer: объект " + fileOrDir + " не обнаружен";
		return false;
	}
}
//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
