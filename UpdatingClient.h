//---------------------------------------------------------------------------
#ifndef UpdatingClient_H
#define UpdatingClient_H
//---------------------------------------------------------------------------
#include <thread>

#include "MyQShellExecute.h"
#include "MyQDifferent.h"
#include "MyQFileDir.h"

class UpdatingClient
{
public:
    UpdatingClient(QString requestUpdateAddContent = "");
};

UpdatingClient::UpdatingClient(QString requestUpdateAddContent)
{
    QString updatingManagerPath = "S:/___Server/03 Users/MyslivchenkoRI/UpdatingManager";
    QString updatingManagerExe = updatingManagerPath+ "/UpdatingManager.exe";
    QString updatingManagerFiles = updatingManagerPath+ "/files";

    if(QFileInfo(updatingManagerExe).exists())
    {
	std::thread thread([requestUpdateAddContent, updatingManagerPath, updatingManagerFiles]()
	{
	    QString ExePath = MyQDifferent::ExePath();
	    QString localUpdatingManagerPathContainer = QDir().homePath() + "/AppData/Local/AVSpas";
	    QString localUpdatingManagerFiles= localUpdatingManagerPathContainer+ "/UpdatingManager/files";
	    QString localUpdatingManagerExe = localUpdatingManagerPathContainer + "/UpdatingManager/UpdatingManager.exe";
	    QString contentRequest = MyQDifferent::ExePathName() + "\n" + requestUpdateAddContent;

	    QFile localUpdaterVersion(localUpdatingManagerFiles+"/version.txt");
	    QFile serverUpdaterVersion(updatingManagerFiles+"/version.txt");
	    bool needUpdateUpdater = true;
	    if(localUpdaterVersion.open(QFile::ReadOnly) && serverUpdaterVersion.open(QFile::ReadOnly))
	    {
		QString localVersion = localUpdaterVersion.readAll();
		QString serverVersion = serverUpdaterVersion.readAll();
		localUpdaterVersion.close();
		serverUpdaterVersion.close();
		if(localVersion.toInt() >= serverVersion.toInt())
		    needUpdateUpdater = false;
	    }

	    if(needUpdateUpdater)
	    {
		if(!QDir(localUpdatingManagerPathContainer).removeRecursively())
		{ qWarning() << "EasyUpdater: can't remove " + localUpdatingManagerPathContainer; return; }
		if(!QDir().mkpath(localUpdatingManagerFiles))
		{ qWarning() << "EasyUpdater: can't create dir " + localUpdatingManagerPathContainer; return; }
		if(!MyQFileDir::CopyDirectory(updatingManagerPath, localUpdatingManagerPathContainer))
		{ qWarning() << "EasyUpdater: can't copy " + updatingManagerPath; return; }
		qDebug() << "updater updated";
	    }
	    else qDebug() << "has actual local updater";

	    QString requestFileName = "update_request.txt";
	    QString requestFilePathName = localUpdatingManagerFiles + "/" + requestFileName;
	    QFile requestFileQFile(requestFilePathName);
	    if(requestFileQFile.open(QFile::WriteOnly))
	    {
		requestFileQFile.write(contentRequest.toUtf8());
		requestFileQFile.close();
	    }
	    else { qDebug() << "EasyUpdater Error open requestFileQFile " + requestFilePathName; return; }

	    if(QFile::exists(localUpdatingManagerExe) && requestFileQFile.exists())
	    {
		MyQShellExecute::ShellExecuteFile(localUpdatingManagerExe);
	    }
	    else
	    {
		qWarning() << "EasyUpdater: something not exists";
		return;
	    }
	});
	thread.detach();
    }
    else qDebug() << "EasyUpdater " + updatingManagerExe + " not found";
}
//---------------------------------------------------------------------------
#endif
