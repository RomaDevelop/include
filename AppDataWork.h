#ifndef AppDataWork_H
#define AppDataWork_H

#include <queue>

#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QStandardPaths>

#include "MyQShortings.h"
#include "MyQDifferent.h"
#include "MyQFileDir.h"

namespace AppDataWorkNames {
	const char* RomaDevelop = "RomaDevelop";

	const char* Windeploy = "Windeploy";
	const char* Catalog = "Catalog";
}
namespace ADWN = AppDataWorkNames;
#define ADWN_RomaDevelop_Catalog ADWN::RomaDevelop, ADWN::Catalog
#define ADWN_RomaDevelop_Windeploy ADWN::RomaDevelop, ADWN::Windeploy

struct AppDataWork
{
	static bool MakeFolderInAppData(const QString &appDataSubdir, const QString &programmName);
	static void MakeFolderAndLinkInAppData(const QString &appDataSubdir, const QString &programmName);

	static QString GetLinkFromAppData(const QString &appDataSubdir, const QString &programmName);
	static QString GetFolderInAppData(const QString &appDataSubdir, const QString &programmName);

	static void WriteMessageFileInAppData(const QString &appDataSubdir, const QString &programmName, const QString &message);
	static void RemoveOldMessageFiles(const QString &folder);
	static void InitTimerMessagesReader(const QString &appDataSubdir, const QString &programmName, QObject *parent,
	                                    std::function<void (QString messageContent, bool &removeFile)> messageFunction);
};

bool AppDataWork::MakeFolderInAppData(const QString &appDataSubdir, const QString &programmName)
{
	auto thisDirInAppData = GetFolderInAppData(appDataSubdir, programmName);
	if(!QDir().mkpath(thisDirInAppData)) { QMbError("error mkpath " + thisDirInAppData); return false; }
	return true;
}

void AppDataWork::MakeFolderAndLinkInAppData(const QString &appDataSubdir, const QString &programmName)
{
	if(!MakeFolderInAppData(appDataSubdir, programmName)) return;

#ifdef QT_NO_DEBUG
	QString fileExePath = GetFolderInAppData(appDataSubdir, programmName).append("/exe_path_name.txt");
#endif
#ifdef QT_DEBUG
	QString fileExePath = GetFolderInAppData(appDataSubdir, programmName).append("/exe_path_name_debug.txt");
#endif
	bool linkExistsAndOk = false;
	if(QFileInfo(fileExePath).isFile())
	{
		auto readRes = MyQFileDir::ReadFile2(fileExePath);
		if(!readRes.success) QMbError("error reading " + fileExePath);
		else
		{
			if(readRes.content == MyQDifferent::ExeNameWithPath()) linkExistsAndOk = true;
			else
			{
				auto answ = QMessageBox::question(nullptr, "Дистрибутив",
				                                  "Программа запущена из нового расположения.\n\nСтарое расположение:\n"+readRes.content
				                                  +"\n\nНовое расположение:\n"+MyQDifferent::ExeNameWithPath()
				                                  + "\n\nСохранить новое расположение по умолчанию?");
				if(answ == QMessageBox::No)
					return;
			}
		}
	}
	if(!linkExistsAndOk)
	{
		if(!MyQFileDir::WriteFile(fileExePath, MyQDifferent::ExeNameWithPath()))
			QMbError("error writing " + fileExePath);
	}
}

QString AppDataWork::GetLinkFromAppData(const QString &appDataSubdir, const QString &programmName)
{
	QString fileExePath = GetFolderInAppData(appDataSubdir, programmName).append("/exe_path_name.txt");
	if(!QFileInfo(fileExePath).isFile()) { QMbError("file "+fileExePath+" doesn't exist"); return ""; }

	auto readRes = MyQFileDir::ReadFile2(fileExePath);
	if(!readRes.success) { QMbError("error reading " + fileExePath); return ""; }

	return readRes.content;
}

QString AppDataWork::GetFolderInAppData(const QString &appDataSubdir, const QString &programmName)
{
	auto appDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
	return appDataPath.append("/").append(appDataSubdir).append("/").append(programmName);
}

void AppDataWork::WriteMessageFileInAppData(const QString &appDataSubdir, const QString &programmName, const QString &message)
{
	auto folder = GetFolderInAppData(appDataSubdir, programmName);

	RemoveOldMessageFiles(folder);

	if(!QFileInfo(folder).isDir()) { QMbError("WriteMessageFileInAppData: Destination folder "+folder+" doesn't exist"); return; }

	static QString preparing_ = "preparing_";

	QString filePreparing = folder;
	filePreparing.append("/").append(preparing_).append("MessageFileInAppData_")
	        .append(QDateTime::currentDateTime().toString(DateTimeFormatForFileName_ms)).append(".txt");
	QString fileReady = folder;
	fileReady.append("/")/*.(preparing_)*/.append("MessageFileInAppData_")
	        .append(QDateTime::currentDateTime().toString(DateTimeFormatForFileName_ms)).append(".txt");

	if(!MyQFileDir::WriteFile(filePreparing, message))
		QMbError("error writing " + filePreparing);
	else
	{
		if(!QFile::rename(filePreparing, fileReady))
			QMbError("error renaming file " + filePreparing + " to " + fileReady);
	}
}

void AppDataWork::RemoveOldMessageFiles(const QString &folder)
{
	QDir dir(folder);
	auto messagesFilesInfos = dir.entryInfoList({"*MessageFileInAppData_*.txt"}, QDir::Files);
	for(auto &msg_fi:messagesFilesInfos)
	{
		if(msg_fi.created().secsTo(QDateTime::currentDateTime()) > 60*10)
		{
			if(!QFile::remove(msg_fi.absoluteFilePath()))
				QMbError("RemoveOldMessageFiles: error removing file "+msg_fi.absoluteFilePath());
		}
	}
}

void AppDataWork::InitTimerMessagesReader(const QString &appDataSubdir, const QString &programmName, QObject *parent,
                                          std::function<void (QString messageContent, bool &removeFile)> messageFunction)
{
	static QTimer *timerReader = nullptr;
	if(timerReader) timerReader->deleteLater();
	timerReader = new QTimer(parent);

	auto folder = GetFolderInAppData(appDataSubdir, programmName);

	RemoveOldMessageFiles(folder);

	QObject::connect(timerReader, &QTimer::timeout, timerReader, [
	                 messageFunctionInLambda = std::move(messageFunction),
	                 folderInLambda = std::move(folder)]()
	{
		QDir dir(folderInLambda);
		auto messagesFiles = dir.entryList({"MessageFileInAppData_*.txt"}, QDir::Files);
		for(auto &file:messagesFiles)
		{
			file.prepend('/');
			file.prepend(folderInLambda);

			bool removeFile = true;

			auto readRes = MyQFileDir::ReadFile2(file);
			if(readRes.success) messageFunctionInLambda(std::move(readRes.content), removeFile);
			else QMbError("error reading file "+file);

			if(removeFile and !QFile::remove(file)) QMbError("error removing file "+file);
		}
	});
	timerReader->start(100);
}

#endif
