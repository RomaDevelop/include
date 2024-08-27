//---------------------------------------------------------------------------
#ifndef UpdatingClient_H
#define UpdatingClient_H
//---------------------------------------------------------------------------
#include <memory>

#include <QProcess>
#include <QString>
#include <QTimer>

#include "thread_box.h"

#include "MyQDifferent.h"
#include "MyQFileDir.h"
#include "MyQShortings.h"

class UpdatingClient
{
public:
    UpdatingClient(QString requestUpdateAddContent = "");
    ~UpdatingClient()
    {
	if(thread.was_started() && !thread.whait_for_ending(3000))
	    QMbw(nullptr,"Error", "Updating time too long");
    }

private:
    void ShowAndWriteError(QString error, QString file = "", QString fileContent = "");
    thread_box thread{""};
};

UpdatingClient::UpdatingClient(QString requestUpdateAddContent)
{
    QString updatingManagerOnServerPath = "S:/___Server/03 Users/MyslivchenkoRI/UpdatingManager";
    QString updatingManagerOnServerExe = updatingManagerOnServerPath+ "/UpdatingManager.exe";
    QString updatingManagerOnServerFiles = updatingManagerOnServerPath+ "/files";

    if(QFileInfo(updatingManagerOnServerExe).exists())
    {
	thread.start([this, requestUpdateAddContent, updatingManagerOnServerPath, updatingManagerOnServerFiles]()
	{
	    QString localUpdatingManagerPathContainer = QDir().homePath() + "/AppData/Local/AVSpas";
	    QString localUpdatingManagerPath = localUpdatingManagerPathContainer+ "/UpdatingManager";
	    QString localUpdatingManagerFiles= localUpdatingManagerPath + "/files";
	    QString localUpdatingManagerExe = localUpdatingManagerPathContainer + "/UpdatingManager/UpdatingManager.exe";
	    QString contentRequest = MyQDifferent::ExePathName() + "\n" + requestUpdateAddContent;

	    QFile localUpdaterVersion(localUpdatingManagerFiles+"/version.txt");
	    QFile serverUpdaterVersion(updatingManagerOnServerFiles+"/version.txt");

	    if(QFileInfo(localUpdatingManagerExe).exists() && !localUpdaterVersion.exists())
	    {
		qWarning() << "Отсутствует локальный файл контроля версии";
		if(!QDir(localUpdatingManagerPath).removeRecursively())
		{
		    ShowAndWriteError("Не удалось удалить localUpdatingManagerPath " + localUpdatingManagerPath);
		    return;
		}
	    }

	    if(!serverUpdaterVersion.exists())
	    {
		ShowAndWriteError("На сервере отсутсвует файл контроля версии, обратитесь к разработчику",
				  MyQDifferent::ExePath() + "/files/updating_errors.txt",
				  "Отсутсвует "+serverUpdaterVersion.fileName());
		return;
	    }

	    bool needUpdateUpdater = true;
	    if(localUpdaterVersion.open(QFile::ReadOnly))
	    {
		if(serverUpdaterVersion.open(QFile::ReadOnly))
		{
		    QString localVersion = localUpdaterVersion.readAll();
		    QString serverVersion = serverUpdaterVersion.readAll();
		    localUpdaterVersion.close();
		    serverUpdaterVersion.close();
		    if(localVersion.toInt() >= serverVersion.toInt())
			needUpdateUpdater = false;
		}
		else
		{
		    ShowAndWriteError("Не удалось открыть файл версии на сервере, обратитесь к разработчику",
				      MyQDifferent::ExePath() + "/files/updating_errors.txt",
				      "Не удалось открыть файл "+serverUpdaterVersion.fileName());
		    return;
		}
	    }

	    if(needUpdateUpdater)
	    {
		if(!QDir(localUpdatingManagerPathContainer).removeRecursively())
		{
		    ShowAndWriteError("EasyUpdater: can't remove " + localUpdatingManagerPathContainer);
		    return;
		}
		if(!QDir().mkpath(localUpdatingManagerFiles))
		{
		    ShowAndWriteError("EasyUpdater: can't create dir " + localUpdatingManagerPathContainer);
		    return;
		}

		if(!MyQFileDir::CopyDirectory(updatingManagerOnServerPath, localUpdatingManagerPathContainer))
		{
		    ShowAndWriteError("EasyUpdater: can't copy " + updatingManagerOnServerPath + " in " + localUpdatingManagerPathContainer);
		    return;
		}
		else
		{
		    qDebug() << "updater updated";
		}
	    }
	    else
	    {
		qDebug() << "has actual local updater";
	    }

	    QString requestFileName = "update_request "+QDateTime::currentDateTime().toString("yyyy.MM.dd hh_mm_ss")+".txt";
	    QString requestFilePathName = localUpdatingManagerFiles + "/" + requestFileName;
	    QFile requestFileQFile(requestFilePathName);
	    if(requestFileQFile.open(QFile::WriteOnly))
	    {
		requestFileQFile.write(contentRequest.toUtf8());
		requestFileQFile.close();
	    }
	    else
	    {
		ShowAndWriteError("EasyUpdater Error open requestFileQFile " + requestFilePathName);
		return;
	    }

	    if(QFile::exists(localUpdatingManagerExe) && requestFileQFile.exists())
	    {
		QProcess process;
		QStringList args;
		process.startDetached(localUpdatingManagerExe, args);
		//MyQShellExecute::ShellExecuteFile(localUpdatingManagerExe);
	    }
	    else
	    {
		ShowAndWriteError("EasyUpdater: something not exists");
		return;
	    }
	});
    }
    else qDebug() << updatingManagerOnServerExe + " not found, update impossible";
}

#include <QApplication>

void UpdatingClient::ShowAndWriteError(QString error, QString fileName, QString fileContent)
{
    qCritical() << "Error " + error;
    QMetaObject::invokeMethod(QApplication::instance(), [error](){QMbc(nullptr,"Ошибка",error);}, Qt::QueuedConnection);

    if(fileName.isEmpty() && fileContent.isEmpty())
	return;

    if(!QDir().mkpath(QFileInfo(fileName).path()))
    {
	QMetaObject::invokeMethod(QApplication::instance(), [fileName](){
	    QMbc(nullptr,"Ошибка","Can't make path " + QFileInfo(fileName).path());}, Qt::QueuedConnection);
	return;
    }

    QFile file(fileName);
    if(file.open(QFile::WriteOnly))
    {
	file.write(fileContent.toUtf8());
    }
    else QMetaObject::invokeMethod(QApplication::instance(), [fileContent, fileName](){
	QMbc(nullptr,"Ошибка","Can't write "+fileContent+" to file "+fileName);}, Qt::QueuedConnection);
}
//---------------------------------------------------------------------------
#endif
