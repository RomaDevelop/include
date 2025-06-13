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

	inline static DeveloperData* CurrentDeveloper();

	static bool DevComp() { static bool myComp { QHostInfo::localHostName().toUpper() == CurrentDeveloper()->hostName }; return myComp; }
	static bool LaunchedFromWorkFiles() { static bool check {MyQDifferent::PathToExe().contains(CurrentDeveloper()->buildPath)}; return check;}
	static bool DevCompAndFromWorkFiles() { return DevComp() && LaunchedFromWorkFiles(); }

	static const QString& ServisCreatorSourcesPath() { static QString str = CurrentDeveloper()->sourcesPath;  return str; }
	static const QString& ConfigsPath() { static QString str = ServisCreatorSourcesPath()+"/Конфиги";  return str; }

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

LaunchParams::DeveloperData *LaunchParams::CurrentDeveloper()
{
	static DeveloperData* currentDeveloper = nullptr;
	static bool defineDid = false;
	if(!defineDid)
	{
		for(auto &dev:developersData)
			if(dev.hostName.toUpper() == QHostInfo::localHostName().toUpper())
				currentDeveloper = &dev;
		if(!currentDeveloper)
		{
			currentDeveloper = &notDeveloper;
			qCritical() << "can't define current by hostName " + QHostInfo::localHostName().toUpper();
		}
		defineDid = true;
	}
	return currentDeveloper;
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif // LAUNCHPARAMS_H
//------------------------------------------------------------------------------------------------------------------------------------------
