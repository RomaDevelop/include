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

///\brief определение параметров запуска
struct LaunchParams {
	declare_struct_4_fields_move(DeveloperData, QString, devName, QString, hostName, QString, sourcesPath, QString, buildPath);

	inline static void Init(std::vector<DeveloperData> developersData);

	inline static const DeveloperData& CurrentDeveloper();
	inline static const QString& SourcesPath() { static QString str = CurrentDeveloper().sourcesPath;  return str; }

	inline static bool LaunchedOnDevComp();
	inline static bool LaunchedFromWorkFiles();
	inline static bool DevCompAndFromWorkFiles() { return LaunchedOnDevComp() && LaunchedFromWorkFiles(); }

private:
	inline static std::vector<DeveloperData> developersData;
	inline static DeveloperData notDeveloper {"not_developer", "not_developer","not_developer","not_developer"};
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

//------------------------------------------------------------------------------------------------------------------------------------------
#endif // LAUNCHPARAMS_H
//------------------------------------------------------------------------------------------------------------------------------------------
