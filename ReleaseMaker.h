#ifndef ReleaseMaker_H
#define ReleaseMaker_H

#include <QString>
#include <QFileDialog>

#include "MyQExecute.h"
#include "ResourcesCreator.h"
#include "LaunchParams.h"
#include "AppDataWork.h"

struct MakeReleaseResult { bool success; QString dirOutput; QString pathToOutputExe; QString exeFileOutput; };

struct ReleaseMaker
{
	inline static MakeReleaseResult MakeRelease(const QString &programmName, bool openDirOutput=true, bool launchWindeploy=true);
	inline static void LaunchWindeploy(const QString &exeFileOutput);
};

//---------------------------------------------------------------------------------------------------------------------

MakeReleaseResult ReleaseMaker::MakeRelease(const QString &programmName, bool openDirOutput, bool launchWindeploy)
{
	MakeReleaseResult result;

#ifndef QT_NO_DEBUG
	if(QMbq({},"Debug mode active","Debug mode active, release will be unusless. Continue?") == QMessageBox::No)
		return result;
#endif

	if(!LaunchParams::DevCompAndFromWorkFiles()) { QMbInfo("MakeRelease works only at developer PC"); return result; }

	QWidget w; w.move(100,100); w.show(); w.hide();
	// w требуется, потому что тупое окно диалога выводится посередине двух мониторов
	QString &dirOutput = result.dirOutput;
	dirOutput = QFileDialog::getExistingDirectory(&w,"Выберите каталог где будет создан дистрибутив", "");

	if(dirOutput.isEmpty()) return result;
	dirOutput += "/release " + programmName + " " + QDateTime::currentDateTime().toString(DateTimeFormatForFileName);

	// если каталог выпуска уже существует - запрашиваем разрешение и  удаяем
	if(QFileInfo(dirOutput).isDir())
	{
		if(QMessageBox::question(0,"Dir already exists", "Dir " + dirOutput + " already exists, rewrite it")
				== QMessageBox::No)
			return result;
		if(!QDir(dirOutput).removeRecursively()) { QMbError("Can't removeRecursively dir "+dirOutput); return result; }
	}

	// создание подкаталогов
	result.pathToOutputExe = dirOutput + "/Files";
	QString &exeFileOutput = result.exeFileOutput;
	exeFileOutput = result.pathToOutputExe + "/" + MyQDifferent::ExeNameNoPath();
	if(!QDir().mkdir(dirOutput)) { QMbError("Can't create dir "+dirOutput); return result; }
	if(!QDir().mkdir(result.pathToOutputExe)) { QMbError("Can't create dir "+result.pathToOutputExe); return result; }

	// копирование exe
	if(!QFile::copy(MyQDifferent::ExeNameWithPath(), exeFileOutput))
	{ QMbError("Can't copy " + MyQDifferent::ExeNameWithPath()); return result; }

	// копирование ресурсов
	QStringList dirsResourcesSrc;
	for(auto &resouresesInstance:BaseResources::existingResources)
		dirsResourcesSrc += resouresesInstance->ResourcesStoragePathOnCompile();
	QString resourcesCopyingRes;
	for(auto &dir:dirsResourcesSrc)
	{
		if(!MyQFileDir::CopyDirectory(dir, result.pathToOutputExe))
			resourcesCopyingRes += dir+"\n";
	}
	if(resourcesCopyingRes.isEmpty() == false)
	{
		resourcesCopyingRes.prepend("Can't copy resources directory(ies):\n\n");
		QMbError(resourcesCopyingRes);
		return result;
	}

	// copy link
	QString link = LaunchParams::SourcesPath() + "/ReleaseLink_"+MyQDifferent::ExeNameNoPath()+".lnk";
	if(QFileInfo(link).isFile())
	{
		if(!QFile::copy(link, dirOutput+"/Запуск.lnk")) QMbWarning("Can't copy link file " + link);
	}
	else QMbWarning("Link file "+link+" to copy not found");

	if(openDirOutput) MyQExecute::OpenDir(dirOutput);

	if(launchWindeploy) LaunchWindeploy(exeFileOutput);

	result.success = true;
	return result;
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
