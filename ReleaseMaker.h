#ifndef ReleaseMaker_H
#define ReleaseMaker_H

#include <QString>
#include <QFileDialog>

#include "MyQExecute.h"
#include "ResourcesCreator.h"
#include "LaunchParams.h"
#include "AppDataWork.h"

struct MakeReleaseResult {
	bool success = false;
	QString dirOutput;
	QString dirWithOutputExe;
	QString pathNameOutputExe;
};

struct ReleaseMaker {
	// MakeReleaseStep1, копирование exe, копирование ресурсов, копирование ссылок, открывание каталога, запуск Windploy
	inline static MakeReleaseResult MakeRelease(const QString &programmName,
												bool copyLinkFile=true,
												bool openDirOutput=true,
												bool launchWindeploy=true);

	// Выбор каталога, проверка существования и перезапись, создание подкаталогов
	// выделено в отдельную функцию, потому что понадобилось для создания выпуска из выпуска,
	// там выполняется первый этап, а дальше копируется целиком весь каталог, где уже есть exe и ресурсы
	inline static MakeReleaseResult MakeReleaseStep1(const QString &programmName);

	inline static void CopyLinkFile(const QString &exeNameNoPath, const QString &dirOutput);
	inline static QString LinkFileName(const QString &exeNameNoPath);
	inline static QString LinkFileNameInSources(const QString &exeNameNoPath);

	inline static void LaunchWindeploy(const QString &exeFileOutput);
};

//---------------------------------------------------------------------------------------------------------------------

MakeReleaseResult ReleaseMaker::MakeRelease(const QString &programmName,
											bool copyLinkFile,
											bool openDirOutput,
											bool launchWindeploy)
{
	MakeReleaseResult result = MakeReleaseStep1(programmName);
	if(!result.success) return result;

	result.success = false;

	// копирование exe
	if(!QFile::copy(MyQDifferent::ExeNameWithPath(), result.pathNameOutputExe))
	{ QMbError("Can't copy " + MyQDifferent::ExeNameWithPath()); return result; }

	// копирование ресурсов
	QStringList dirsResourcesSrc;
	for(auto &resouresesInstance:BaseResources::existingResources)
		dirsResourcesSrc += resouresesInstance->ResourcesStoragePathOnCompile();
	QString resourcesCopyingRes;
	for(auto &dir:dirsResourcesSrc)
	{
		if(!MyQFileDir::CopyDirectory(dir, result.dirWithOutputExe))
			resourcesCopyingRes += dir+"\n";
	}
	if(resourcesCopyingRes.isEmpty() == false)
	{
		resourcesCopyingRes.prepend("Can't copy resources directory(ies):\n\n");
		QMbError(resourcesCopyingRes);
		return result;
	}

	if(copyLinkFile) CopyLinkFile(MyQDifferent::ExeNameNoPath(), result.dirOutput);

	if(openDirOutput) MyQExecute::OpenDir(result.dirOutput);

	if(launchWindeploy)
	{
		QTimer::singleShot(300, [result](){ ReleaseMaker::LaunchWindeploy(result.pathNameOutputExe); });
		// singleShot потому что папка открывается после Windeploy и перекрывает его
	}

	result.success = true;
	return result;
}

MakeReleaseResult ReleaseMaker::MakeReleaseStep1(const QString &programmName)
{
	MakeReleaseResult result;

#ifndef QT_NO_DEBUG
	if(QMbq({},"Debug mode active","Debug mode active, release will be unusless. Continue?") == QMessageBox::No)
		return result;
#endif

	QWidget w; w.move(100,100); w.show(); w.hide();
	// w требуется, потому что тупое окно диалога выводится посередине двух мониторов
	result.dirOutput = QFileDialog::getExistingDirectory(&w,"Выберите каталог где будет создан дистрибутив", "");

	if(result.dirOutput.isEmpty()) return result;
	result.dirOutput += "/release " + programmName + " " + QDateTime::currentDateTime().toString(DateTimeFormatForFileName);

	// если каталог выпуска уже существует - запрашиваем разрешение и удаяем
	if(QFileInfo(result.dirOutput).isDir())
	{
		if(QMessageBox::question(0,"Dir already exists", "Dir " + result.dirOutput + " already exists, rewrite it")
				== QMessageBox::No)
			return result;
		if(!QDir(result.dirOutput).removeRecursively()) { QMbError("Can't removeRecursively dir "+result.dirOutput); return result; }
	}

	// создание подкаталогов
	result.dirWithOutputExe = result.dirOutput + "/Files";
	result.pathNameOutputExe = result.dirWithOutputExe + "/" + MyQDifferent::ExeNameNoPath();
	if(!QDir().mkdir(result.dirOutput)) { QMbError("Can't create dir "+result.dirOutput); return result; }
	if(!QDir().mkdir(result.dirWithOutputExe)) { QMbError("Can't create dir "+result.dirWithOutputExe); return result; }

	result.success = true;
	return result;
}

void ReleaseMaker::CopyLinkFile(const QString &exeNameNoPath, const QString &dirOutput)
{
	QString prevLink;
	QString link = LaunchParams::SourcesPath() + "/" + LinkFileName(exeNameNoPath);
	if(not QFileInfo(link).isFile())
	{
		prevLink = link;
		link = MyQDifferent::PathToExe() + "/" + LinkFileName(exeNameNoPath);
	}

	if(QFileInfo(link).isFile())
	{
		if(!QFile::copy(link, dirOutput+"/Запуск.lnk"))
			QMbWarning("Can't copy link file " + link);
	}
	else
	{
		QMbWarning("Link file not found. Checked "+(prevLink.isEmpty() ?
														link
													  : link+"\nand\n"+prevLink));
	}
}

QString ReleaseMaker::LinkFileName(const QString &exeNameNoPath)
{
	return "ReleaseLink_"+exeNameNoPath+".lnk";
}

QString ReleaseMaker::LinkFileNameInSources(const QString &exeNameNoPath)
{
	return LaunchParams::SourcesPath() + "/" + LinkFileName(exeNameNoPath);
}

void ReleaseMaker::LaunchWindeploy(const QString &exeFileOutput)
{
	// запуск Windeploy
	auto appDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
	auto windeployExeFile = appDataPath + "/"+ADWN::RomaDevelop+"/"+ADWN::Windeploy+"/exe_path_name.txt";
	if(QFileInfo(windeployExeFile).isFile())
	{
		auto readRes = MyQFileDir::ReadFile2(windeployExeFile);
		if(!readRes.success) QMbError("error reading " + windeployExeFile);
		else
		{
			if(!QFileInfo(readRes.content).isFile()) QMbError("not found " + readRes.content);
			else
			{
				if(!MyQExecute::Execute(readRes.content, {exeFileOutput}))
					QMbError("error executing " + readRes.content);
			}
		}
	}
	else QMbWarning("not found " + windeployExeFile);
}

//---------------------------------------------------------------------------------------------------------------------

#endif
