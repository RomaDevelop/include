//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef LAUNCHPARAMS_H
#define LAUNCHPARAMS_H
//------------------------------------------------------------------------------------------------------------------------------------------
#include <vector>

#include <QString>
#include <QHostInfo>

#include "MyQDifferent.h"
#include "declare_struct.h"

//------------------------------------------------------------------------------------------------------------------------------------------

///\brief LaunchParams предназначен для определения параметров запуска
/// функцию Init необходимо вызвать на самом раннем этапе работы программы, передать в неё массив данных о разработчиках
/// далее, на этапе работы программы можно получать параметры запуска: данные разработчика,
///		выполнять проверку запущено ли на компьютере разработчика, из каталога сборки ли и т.д.
///	при вызове функций проверки и получения данных, значение вычислеяется однократно (Meyer's Singleton)
struct LaunchParams {
	declare_struct_4_fields_move(DeveloperData, QString, devName, QString, hostName, QString, sourcesPath, QString, buildPath);

	inline static void Init(std::vector<DeveloperData> developersData);

	inline static const DeveloperData& CurrentDeveloper();
	inline static const QString& SourcesPath();

	inline static bool LaunchedOnDevComp();
	inline static bool LaunchedFromWorkFiles();
	inline static bool DevCompAndFromWorkFiles();

	inline static const std::vector<DeveloperData>& DevelopersData() { return developersData; }

private:
	inline static std::vector<DeveloperData> developersData;
	inline static DeveloperData notDeveloper {"not_developer", "not_developer", "not_developer", "not_developer"};
};

//------------------------------------------------------------------------------------------------------------------------------------------

void LaunchParams::Init(std::vector<DeveloperData> developersData)
{
	LaunchParams::developersData = std::move(developersData);
	for(auto &devData:developersData)
	{
		devData.sourcesPath = QDir::toNativeSeparators(devData.sourcesPath);
		devData.buildPath = QDir::toNativeSeparators(devData.buildPath);
	}
}

const LaunchParams::DeveloperData& LaunchParams::CurrentDeveloper()
{
	static DeveloperData* currentDeveloper = &notDeveloper;
	static bool defineDeveloperDid = false;
	if(!defineDeveloperDid)
	{
		for(auto &dev:developersData)
			if(dev.hostName.toUpper() == QHostInfo::localHostName().toUpper())
				currentDeveloper = &dev;
		if(currentDeveloper == &notDeveloper)
		{
			currentDeveloper = &notDeveloper;
			qCritical() << "can't define current developer by hostName " + QHostInfo::localHostName().toUpper();
		}
		defineDeveloperDid = true;
	}
	return *currentDeveloper;
}

const QString &LaunchParams::SourcesPath()
{
	static QString str = CurrentDeveloper().sourcesPath;
	return str;
}

bool LaunchParams::LaunchedOnDevComp()
{
	static bool isDevComp { QHostInfo::localHostName().toUpper() == CurrentDeveloper().hostName.toUpper() };
	return isDevComp;
}

bool LaunchParams::LaunchedFromWorkFiles()
{
	static bool check { MyQDifferent::PathToExe().contains(CurrentDeveloper().buildPath) };
	return check;
}

bool LaunchParams::DevCompAndFromWorkFiles()
{
	return LaunchedOnDevComp() && LaunchedFromWorkFiles();
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif // LAUNCHPARAMS_H
//------------------------------------------------------------------------------------------------------------------------------------------
