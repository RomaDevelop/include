//---------------------------------------------------------------------------
#ifndef UpdatingClient_H
#define UpdatingClient_H
//---------------------------------------------------------------------------
#include <thread>

#include <windows.h>

#include "MyQShellExecute.h"
#include "MyQDifferent.h"
#include "MyQFileDir.h"

class UpdatingClient
{
public:
    UpdatingClient(QString requestUpdateAddContent = "");

    static void ShowAndWriteError(QString error, QString file = "", QString fileContent = "");
};

UpdatingClient::UpdatingClient(QString requestUpdateAddContent)
{
    QString updatingManagerOnServerPath = "S:/___Server/03 Users/MyslivchenkoRI/UpdatingManager";
    QString updatingManagerOnServerExe = updatingManagerOnServerPath+ "/UpdatingManager.exe";
    QString updatingManagerOnServerFiles = updatingManagerOnServerPath+ "/files";

    if(QFileInfo(updatingManagerOnServerExe).exists())
    {
	std::thread thread([requestUpdateAddContent, updatingManagerOnServerPath, updatingManagerOnServerFiles]()
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
		MyQShellExecute::ShellExecuteFile(localUpdatingManagerExe);
	    }
	    else
	    {
		ShowAndWriteError("EasyUpdater: something not exists");
		return;
	    }
	});
	thread.detach();
    }
    else qDebug() << "EasyUpdater " + updatingManagerOnServerExe + " not found";
}

void UpdatingClient::ShowAndWriteError(QString error, QString fileName, QString fileContent)
{
    MessageBox(nullptr, error.toStdWString().c_str(), L"Ошибка", MB_OK | MB_ICONERROR);

    if(fileName.isEmpty() && fileContent.isEmpty())
	return;

    if(!QDir().mkpath(QFileInfo(fileName).path()))
    {
	MessageBox(nullptr, QString("Can't make path " + QFileInfo(fileName).path()).toStdWString().c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
	return;
    }

    QFile file(fileName);
    if(file.open(QFile::WriteOnly))
    {
	file.write(fileContent.toUtf8());
    }
    else MessageBox(nullptr, QString("Can't write "+fileContent+" to file "+fileName).toStdWString().c_str(), L"Ошибка", MB_OK | MB_ICONERROR);
}
//---------------------------------------------------------------------------
#endif
