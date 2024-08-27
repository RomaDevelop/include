//---------------------------------------------------------------------------
#ifndef MyQExecute_H
#define MyQExecute_H
//---------------------------------------------------------------------------
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
//---------------------------------------------------------------------------
struct MyQExecute
{
    inline static bool Execute(QString file, QStringList args = {})
    {
	QFileInfo fileInfo(file);
	if(fileInfo.isExecutable()) // Если файл исполняемый
	{
	    return QProcess::startDetached(file, args);
	}
	else
	{
	    if(!args.isEmpty())
		qDebug() << "MyQExecute::Execute: файл " + file + " не является исполняемым, аргументы игнорируются";
	    return QDesktopServices::openUrl(QUrl::fromLocalFile(file));
	}
    }
    inline static bool ShowInExplorer(QString fileOrDir)
    {
	if(QFileInfo(fileOrDir).exists() || QDir(fileOrDir).exists())
	{
	QStringList args;
	args << "/select," << QDir::toNativeSeparators(fileOrDir);
	return QProcess::startDetached("explorer", args);
	}
	else
	{
	    qCritical() << "MyQExecute::Execute: объект " + fileOrDir + " не обнаружен";
	    return false;
	}
    }
};
//---------------------------------------------------------------------------
#endif
