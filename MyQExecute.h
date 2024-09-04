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
    inline static bool Execute(QString file, QStringList args = {});
    inline static bool OpenDir(QString dir);
    inline static bool ShowInExplorer(QString fileOrDir);
};

bool MyQExecute::Execute(QString file, QStringList args)
{
    QFileInfo fileInfo(file);
    if(!fileInfo.isSymLink() && fileInfo.isFile())
    {
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

    qDebug() << "MyQExecute::Execute: файл " + file + " не обнаружен";
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
	qCritical() << "MyQExecute::Execute: объект " + fileOrDir + " не обнаружен";
	return false;
    }
}
//---------------------------------------------------------------------------
#endif
