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
	Resource(QString pathStorageInSources, const QString &pathStorageInDistrib, const QString &filename, bool copyResourcesInDistib)
	{
		static QString erPrefix = "ERROR Resource creation: ";

		if(pathStorageInSources == NotInitedMarker()) return;

		if(!Resource::CheckFileOrPath(filename))
		{
			errors += erPrefix + "bad filename " + filename;
			return;
		}

		QString aPathNameOnCompile = pathStorageInSources + "/" + filename;
		QFileInfo fiOnCompile(aPathNameOnCompile);
		pathNameInDistib = pathStorageInDistrib + "/" + filename;
		QFileInfo fiOnDistib(pathNameInDistib);

		if(copyResourcesInDistib)
		{
			if(!QDir().mkpath(pathStorageInDistrib))
			{
				errors += erPrefix + "error mkpath " + pathStorageInDistrib;
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

	static bool CheckFileOrPath(const QString &fileOrPath)
	{
		static QString badSymbols = "<>:|?*"
									"\e\r\t\a\f\b\n";

		int i=0;
		for(auto &c:fileOrPath)
		{
			#ifdef _WIN32
				if(i==1 && c == ':') continue;
			#endif
			if(badSymbols.contains(c))
			{
				qdbg << fileOrPath << c;
				return false;
			}
			i++;
		}
		return true;
	}

private:
	QString pathNameInDistib;
};
//------------------------------------------------------------------------------------------------------------------------------------------
class ResourcesTest
{
	QString resourcesStoragePathOnCompile;
	QString resourcesStoragePathInDistrib;
	bool copyResourcesInDistib;

	inline static std::function<void(QString)> errorWorker;

	inline static ResourcesTest *instancePtr = nullptr;
	inline static Resource forNotInited {	Resource::NotInitedMarker(), Resource::NotInitedMarker(),
											Resource::NotInitedMarker(), false };
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
		if(!Resource::CheckFileOrPath(aResourcesStoragePathOnCompile) || !Resource::CheckFileOrPath(aResourcesStoragePathInDistrib))
			Resource::errors.push_back("ResourcesTest::Init bad storages paths");

		static ResourcesTest instance(std::move(aResourcesStoragePathOnCompile),
		                              std::move(aResourcesStoragePathInDistrib),
		                              aCopyResourcesInDistib);
		instancePtr = &instance;
		if(!aCopyResourcesInDistib) qDebug() << QString("WARNING: ") + "#ResourecesName" + " Init param aCopyResourcesInDistib = "
		                                        + (aCopyResourcesInDistib ? "true" : "false");
		if(!Resource::errors.isEmpty()) for(auto &error:Resource::errors) qCritical() << error;
		if(!Resource::errors.isEmpty()) if(errorWorker) errorWorker(Resource::errors.join('\n'));
	}

private: Resource m_copyIco { resourcesStoragePathOnCompile, resourcesStoragePathInDistrib, "copy.ico", copyResourcesInDistib };
public: static const Resource& copyIco() { if(instancePtr) return instancePtr->m_copyIco; else return RetNotInited(); }

private: Resource m_pasteIco { resourcesStoragePathOnCompile, resourcesStoragePathInDistrib, "paste.ico", copyResourcesInDistib };
public: static const Resource& pasteIco() { if(instancePtr) return instancePtr->m_pasteIco; else return RetNotInited(); }
};
//------------------------------------------------------------------------------------------------------------------------------------------
#define DECLARE_RESOURCES(ResourcesName)						\
	class ResourcesName											\
	{															\
		QString resourcesStoragePathOnCompile;					\
		QString resourcesStoragePathInDistrib;					\
		bool copyResourcesInDistib;								\
																\
		inline static ResourcesName *instancePtr = nullptr;		\
		inline static Resource forNotInited {	Resource::NotInitedMarker(), Resource::NotInitedMarker(),	\
												Resource::NotInitedMarker(), false };						\
		inline static Resource& RetNotInited()																\
		{																									\
			if(errorWorker) errorWorker("Resource requested before Init");									\
			qCritical() << "Resource requested before Init";												\
			return forNotInited;																			\
		}																									\
																											\
	public:																									\
		ResourcesName(QString aResourcesStoragePathOnCompile, QString aResourcesStoragePathInDistrib, bool aCopyResourcesInDistib):		\
	        resourcesStoragePathOnCompile(aResourcesStoragePathOnCompile),																\
	        resourcesStoragePathInDistrib(aResourcesStoragePathInDistrib),																\
	        copyResourcesInDistib{aCopyResourcesInDistib}																				\
        {}																																\
	                                                                                                                                    \
	    static void Init(QString aResourcesStoragePathOnCompile, QString aResourcesStoragePathInDistrib, bool aCopyResourcesInDistib)	\
		{																																\
			if(!Resource::CheckFileOrPath(aResourcesStoragePathOnCompile) || !Resource::CheckFileOrPath(aResourcesStoragePathInDistrib))\
				Resource::errors.push_back(#ResourcesName"::Init bad storages paths");													\
																																		\
			static ResourcesName instance(std::move(aResourcesStoragePathOnCompile),													\
	                                  std::move(aResourcesStoragePathInDistrib),														\
	                                  aCopyResourcesInDistib);																			\
	        instancePtr = &instance;																									\
			if(!aCopyResourcesInDistib) qDebug() << QString("WARNING: ") + #ResourcesName + " Init param copyResourcesInDistib = "		\
	                                                + (aCopyResourcesInDistib ? "true" : "false");										\
	        if(!Resource::errors.isEmpty()) for(auto &error:Resource::errors) qCritical() << error;										\
	        if(!Resource::errors.isEmpty()) if(errorWorker) errorWorker(Resource::errors.join('\n'));									\
	    }
//------------------------------------------------------------------------------------------------------------------------------------------
#define SET_ERROR_WORKER(worker) \
	private: inline static std::function<void(QString)> errorWorker { worker };
// Exmples:
// SET_ERROR_WORKER([](QString str){ QMbError(str); })
// SET_ERROR_WORKER(nullptr)
//------------------------------------------------------------------------------------------------------------------------------------------
#define END_DECLARE_RESOURCES \
	};
//------------------------------------------------------------------------------------------------------------------------------------------
#define DECLARE_RESOURCE(name, file) \
	private: Resource m_##name { resourcesStoragePathOnCompile, resourcesStoragePathInDistrib, file, copyResourcesInDistib };	\
	public: static const Resource& name() { if(instancePtr) return instancePtr->m_##name; else return RetNotInited(); }
// нужно указать в file имя файла с которым он хранится в каталоге исходников и будет с таким же в каталоге сборки. Можно указать в подкаталоге
//------------------------------------------------------------------------------------------------------------------------------------------

#endif
