#ifndef RESOURCE_H
#define RESOURCE_H

#include <memory>

#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QFileInfo>

#include "MyQShortings.h"

struct Resource
{
	inline static QStringList errors;
	Resource(QString aPathNameOnCompile, const QString &aPathInDistib, bool copyResourcesInDistib)
	{
		QFileInfo fiOnCompile(aPathNameOnCompile);
		pathNameInDistib = aPathInDistib;
		pathNameInDistib.append('/');
		pathNameInDistib.append(fiOnCompile.fileName());
		QFileInfo fiOnDistib(pathNameInDistib);

		if(copyResourcesInDistib)
		{
			if(!QDir().mkpath(aPathInDistib))
			{
				errors += "error mkpath resource " + QFileInfo(aPathInDistib).path();
				return;
			}

			if(fiOnDistib.exists())
			{
				if(!QFile::remove(pathNameInDistib))
				{
					errors += "error remove " + pathNameInDistib;
					return;
				}
			}

			if(!QFile::copy(aPathNameOnCompile, pathNameInDistib))
				errors += "error copying resource " + aPathNameOnCompile + " to " + pathNameInDistib;
		}
	}
	QString GetPathName() const { return pathNameInDistib; }
private:
	QString pathNameInDistib;
};

class ResourcesTest
{
	QString resourcesStoragePathOnCompile;
	QString resourcesStoragePathInDistrib;
	bool copyResourcesInDistib;

	inline static ResourcesTest *instancePtr = nullptr;
	inline static Resource forNotInited { "resources not inited", "resources not inited", false };

public:

	ResourcesTest(QString aResourcesStoragePathOnCompile, QString aResourcesStoragePathInDistrib, bool aCopyResourcesInDistib):
		resourcesStoragePathOnCompile(aResourcesStoragePathOnCompile),
		resourcesStoragePathInDistrib(aResourcesStoragePathInDistrib),
		copyResourcesInDistib{aCopyResourcesInDistib}
	{}

	static void Init(QString aResourcesStoragePathOnCompile, QString aResourcesStoragePathInDistrib, bool aCopyResourcesInDistib)
	{
		static ResourcesTest instance(std::move(aResourcesStoragePathOnCompile),
								  std::move(aResourcesStoragePathInDistrib),
								  aCopyResourcesInDistib);
		instancePtr = &instance;
	}

private: Resource m_copyIco { resourcesStoragePathOnCompile + "/" + "copy.ico", resourcesStoragePathInDistrib, copyResourcesInDistib };
public: static const Resource& copyIco() { if(instancePtr) return instancePtr->m_copyIco; else return forNotInited; }

private: Resource m_pasteIco { resourcesStoragePathOnCompile + "/" + "paste.ico", resourcesStoragePathInDistrib, copyResourcesInDistib };
public: static const Resource& pasteIco() { if(instancePtr) return instancePtr->m_pasteIco; else return forNotInited; }
};

//------------------------------------------------------------------------------------------------------------------------------------------
#define DECLARE_RESOURCES(ResourecesName)						\
	class ResourecesName										\
	{															\
		QString resourcesStoragePathOnCompile;					\
		QString resourcesStoragePathInDistrib;					\
		bool copyResourcesInDistib;								\
																\
		inline static ResourecesName *instancePtr = nullptr;	\
		inline static Resource forNotInited { "resources not inited", "resources not inited", false };	\
																\
	public:														\
		ResourecesName(QString aResourcesStoragePathOnCompile, QString aResourcesStoragePathInDistrib, bool aCopyResourcesInDistib):   \
			resourcesStoragePathOnCompile(aResourcesStoragePathOnCompile),															   \
			resourcesStoragePathInDistrib(aResourcesStoragePathInDistrib),															   \
			copyResourcesInDistib{aCopyResourcesInDistib}																			   \
		{}																															   \
																																	   \
		static void Init(QString aResourcesStoragePathOnCompile, QString aResourcesStoragePathInDistrib, bool aCopyResourcesInDistib)  \
		{																															   \
			static ResourecesName instance(std::move(aResourcesStoragePathOnCompile),												   \
									  std::move(aResourcesStoragePathInDistrib),													   \
									  aCopyResourcesInDistib);																		   \
			instancePtr = &instance;																								   \
		}
//------------------------------------------------------------------------------------------------------------------------------------------
#define END_DECLARE_RESOURCES \
	};
//------------------------------------------------------------------------------------------------------------------------------------------
#define DECLARE_RESOURCE(name, file) \
	private: Resource m_##name { resourcesStoragePathOnCompile + "/" + file , resourcesStoragePathInDistrib, copyResourcesInDistib };	\
	public: static const Resource& name() { if(instancePtr) return instancePtr->m_##name; else return forNotInited; }
//------------------------------------------------------------------------------------------------------------------------------------------

#endif
