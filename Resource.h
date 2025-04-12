#ifndef RESOURCE_H
#define RESOURCE_H

#include <memory>
#include <functional>

#include <QDebug>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QFileInfo>

#include "MyQShortings.h"

struct Resource
{
	QString GetPathName() const { return pathNameInDistib; }

	inline static QStringList errors;
	Resource(QString aPathNameOnCompile, const QString &aPathInDistib, bool copyResourcesInDistib)
	{
		if(aPathNameOnCompile == NotInitedMarker()) return;

		static QString erPrefix = "ERROR Resource creation: ";
		QFileInfo fiOnCompile(aPathNameOnCompile);
		pathNameInDistib = aPathInDistib;
		pathNameInDistib.append('/');
		pathNameInDistib.append(fiOnCompile.fileName());
		QFileInfo fiOnDistib(pathNameInDistib);

		if(copyResourcesInDistib)
		{
			if(!QDir().mkpath(aPathInDistib))
			{
				errors += erPrefix + "error mkpath resource " + QFileInfo(aPathInDistib).path();
				return;
			}

			if(fiOnDistib.exists())
			{
				if(!QFile::remove(pathNameInDistib))
				{
					errors += erPrefix + "error remove " + pathNameInDistib;
					return;
				}
			}

			if(!QFile::copy(aPathNameOnCompile, pathNameInDistib))
			{
				errors += erPrefix + "error copying resource " + aPathNameOnCompile + " to " + pathNameInDistib;
				return;
			}
		}
		if(!QFileInfo(pathNameInDistib).isFile())
			errors += erPrefix + "resource file doesn't exists (" + pathNameInDistib + ")";
	}

	static const QString& NotInitedMarker(){ static QString s = "resources not inited"; return s; }
private:
	QString pathNameInDistib;
};

class ResourcesTest
{
	QString resourcesStoragePathOnCompile;
	QString resourcesStoragePathInDistrib;
	bool copyResourcesInDistib;

	inline static std::function<void(QString)> errorWorker;

	inline static ResourcesTest *instancePtr = nullptr;
	inline static Resource forNotInited { Resource::NotInitedMarker(), Resource::NotInitedMarker(), false };
	inline static Resource& RetNotInited()
	{
		if(errorWorker) errorWorker("Resource requested before Init");
		qCritical() << "Resource requested before Init";
		return forNotInited;
	}

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
		if(!aCopyResourcesInDistib) qDebug() << QString("WARNING: ") + "#ResourecesName" + " Init param aCopyResourcesInDistib = "
		                                        + (aCopyResourcesInDistib ? "true" : "false");
		if(!Resource::errors.isEmpty()) for(auto &error:Resource::errors) qCritical() << error;
		if(!Resource::errors.isEmpty()) if(errorWorker) errorWorker(Resource::errors.join('\n'));
	}

private: Resource m_copyIco { resourcesStoragePathOnCompile + "/" + "copy.ico", resourcesStoragePathInDistrib, copyResourcesInDistib };
public: static const Resource& copyIco() { if(instancePtr) return instancePtr->m_copyIco; else return RetNotInited(); }

private: Resource m_pasteIco { resourcesStoragePathOnCompile + "/" + "paste.ico", resourcesStoragePathInDistrib, copyResourcesInDistib };
public: static const Resource& pasteIco() { if(instancePtr) return instancePtr->m_pasteIco; else return RetNotInited(); }
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
	    inline static Resource& RetNotInited()															\
        {																								\
	        if(errorWorker) errorWorker("Resource requested before Init");								\
	        qCritical() << "Resource requested before Init";											\
	        return forNotInited;																		\
	    }																								\
	                                                                                                    \
	public:																								\
	    ResourecesName(QString aResourcesStoragePathOnCompile, QString aResourcesStoragePathInDistrib, bool aCopyResourcesInDistib):	\
	        resourcesStoragePathOnCompile(aResourcesStoragePathOnCompile),																\
	        resourcesStoragePathInDistrib(aResourcesStoragePathInDistrib),																\
	        copyResourcesInDistib{aCopyResourcesInDistib}																				\
        {}																																\
	                                                                                                                                    \
	    static void Init(QString aResourcesStoragePathOnCompile, QString aResourcesStoragePathInDistrib, bool aCopyResourcesInDistib)	\
        {																																\
	        static ResourecesName instance(std::move(aResourcesStoragePathOnCompile),													\
	                                  std::move(aResourcesStoragePathInDistrib),														\
	                                  aCopyResourcesInDistib);																			\
	        instancePtr = &instance;																									\
	        if(!aCopyResourcesInDistib) qDebug() << QString("WARNING: ") + #ResourecesName + " Init param copyResourcesInDistib = "		\
	                                                + (aCopyResourcesInDistib ? "true" : "false");										\
	        if(!Resource::errors.isEmpty()) for(auto &error:Resource::errors) qCritical() << error;										\
	        if(!Resource::errors.isEmpty()) if(errorWorker) errorWorker(Resource::errors.join('\n'));									\
	    }
//------------------------------------------------------------------------------------------------------------------------------------------
#define END_DECLARE_RESOURCES \
	};
//------------------------------------------------------------------------------------------------------------------------------------------
#define DECLARE_RESOURCE(name, file) \
	private: Resource m_##name { resourcesStoragePathOnCompile + "/" + file , resourcesStoragePathInDistrib, copyResourcesInDistib };	\
	public: static const Resource& name() { if(instancePtr) return instancePtr->m_##name; else return RetNotInited(); }
//------------------------------------------------------------------------------------------------------------------------------------------
#define SET_ERROR_WORKER(worker) \
	private: inline static std::function<void(QString)> errorWorker { worker };
// Exmples:
// SET_ERROR_WORKER([](QString str){ QMbError(str); })
// SET_ERROR_WORKER(nullptr)
//------------------------------------------------------------------------------------------------------------------------------------------

#endif
