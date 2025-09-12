//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MQFILEDIR_H
#define MQFILEDIR_H
//------------------------------------------------------------------------------------------------------------------------------------------
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QTextCodec>
#include <QMessageBox>
#include <QDirIterator>

#include "MyQShortings.h"
//------------------------------------------------------------------------------------------------------------------------------------------
struct MyQFileDir
{
	/// Возвращает пустую строку при успешном переименовании или текст ошибки
	inline static QString Rename(QString oldFile, QString newFile, bool forceCaseSensitiveRename);
		// если имена различнаются только в регистре QFile::rename не переименовывает (верно для ntfs)

	inline static QString GetCurrentFileNameFromRenameError(QString errorStr);

	inline static QFileInfo FindNewest(const QFileInfoList &files);

	enum SortFlags { noSort, name, modified, read };
	inline static QString RemoveOldFiles(QString directory, int remainCount, SortFlags sortFlag = MyQFileDir::modified);

	///\brief removes in all subcategories
	inline static void RemoveOldFilesByDaysThreshold(const QString &directory, int daysThresholdToRemove, bool removeEmptySubcats,
									   SortFlags daysThresholdType = modified);

	inline static void RemoveEmptySubcats(const QString &directory);

	inline static bool RemoveDirIfEmpty(const QString &dirStr, bool ShowErrorMessage);

	inline static void ReplaceFileWithBackup(const QFileInfo &src, const QFileInfo &dst, const QString &backupPath);
	inline static void ReplaceFilesWithBackup(const QFileInfoList &filesToReplace, const QFileInfo &fileSrc, const QString &backupPath);

	inline static QStringList GetAllNestedDirs(QString path,
											   QDir::Filters filters = QDir::Dirs | QDir::NoDotAndDotDot,
											   QDir::SortFlag sort = QDir::NoSort);
	inline static QFileInfoList GetAllFilesIncludeSubcats(QString path, const QStringList &extFilter = {});

	inline static QStringList FileInfoListToStrList(const QFileInfoList &fileInfoList);
	inline static QFileInfoList StrListToFileInfoList(const QStringList &fileInfoList);

	using cbProgress_t = std::function<void(int copied)>;
	inline static bool CopyDirectory(QString directory, QString pathDestination, QString newName = "", cbProgress_t progress = nullptr);
	// перезаписывает не спрашивая

	struct ReadResult { bool success = 0; QString content; };

	inline static bool		WriteFile(const QString &fileName, const QString &content, const char * encoding = "UTF-8");
	inline static bool		AppendFile(const QString &fileName, const QString &content, const char * encoding = "UTF-8");
	inline static QString	ReadFile1(const QString &fileName, const char * encoding = "UTF-8", bool *success = nullptr);
	inline static ReadResult	ReadFile2(const QString &fileName, const char * encoding = "UTF-8");

	inline static bool		CopyFileWithReplace(const QString &file, const QString & fileDst);

private:
	inline static const QString& RenameErrMarker() { static QString str = "currentFileName:"; return str; }
};
//------------------------------------------------------------------------------------------------------------------------------------------

QString MyQFileDir::Rename(QString oldFile, QString newFile, bool forceCaseSensitiveRename)
{
	if(forceCaseSensitiveRename)
	{
		oldFile = QDir::toNativeSeparators(oldFile);
		newFile = QDir::toNativeSeparators(newFile);
		if(oldFile.toLower() == newFile.toLower())
		{
			QFileInfo fi(oldFile);
			QString tmpNewFile;
			if(fi.fileName().size() < 4) tmpNewFile = newFile.chopped(1); // удаление 1 символа в конце для вставки цифры в цикле
			else /* имя файла >= 4 */ tmpNewFile = newFile.chopped(4) + "tmp"; // удаление 3+1 символов для вставки tmp и цифры в цикле
			
			for(int i=0; i<10; i++)
				if(!QFileInfo::exists(tmpNewFile + QChar('0'+i)))
				{
					tmpNewFile += QChar('0'+i);
					break;
				}
			if(tmpNewFile.size() != newFile.size())
				return "Can't create tmp name to forceCaseSensitiveRename\n\n" + oldFile + "\n\nto\n\n" + newFile + "\n\n"+RenameErrMarker()+oldFile;

			if(!QFile::rename(oldFile, tmpNewFile))
				return "QFile::rename returned false for forceCaseSensitiveRename(step1)\n\n" + oldFile + "\n\nto\n\n" + tmpNewFile + "\n\n"+RenameErrMarker()+oldFile;
			if(!QFile::rename(tmpNewFile, newFile))
				return "QFile::rename returned false for forceCaseSensitiveRename(step2)\n\n" + tmpNewFile + "\n\nto\n\n" + newFile + "\n\n"+RenameErrMarker()+tmpNewFile;
			return "";
		}
		else
		{
			if(QFile::rename(oldFile, newFile)) return "";
			else return "QFile::rename returned false for rename\n\n" + oldFile + "\n\nto\n\n" + newFile + "\n\n"+RenameErrMarker()+oldFile;
		}
	}
	if(QFile::rename(oldFile, newFile)) return "";
	else return "QFile::rename returned false for rename\n\n" + oldFile + "\n\nto\n\n" + newFile + "\n\n"+RenameErrMarker()+oldFile;
}

QString MyQFileDir::GetCurrentFileNameFromRenameError(QString errorStr)
{
	return errorStr.remove(0,errorStr.indexOf(RenameErrMarker()) + RenameErrMarker().size());
}

QFileInfo MyQFileDir::FindNewest(const QFileInfoList & files)
{
	QFileInfo newestModifFI;
	if(files.size())
	{
		newestModifFI = files[0];
		for(auto &f:files)
			if(f.lastModified() > newestModifFI.lastModified())
				newestModifFI = f;
	}
	return newestModifFI;
}

QString MyQFileDir::RemoveOldFiles(QString directory, int remainCount, SortFlags sortFlag)
{
	QString ret;
	QDir dir(directory);
	if(!dir.exists())
	{
		ret += "directory ["+directory+"] not exists";
	}

	QFileInfoList content = dir.entryInfoList(QDir::Files);
	for(int i=content.size()-1; i>=0; i--)
		if(!content[i].isFile()) content.removeAt(i);

	static auto cmpName = [](const QFileInfo &a, const QFileInfo &b){
		return a.fileName() < b.fileName(); // сортировка по имени (по возрастанию)
	};

	static auto cmpModified = [](const QFileInfo &a, const QFileInfo &b){
		if (a.lastModified() != b.lastModified()) {
			return a.lastModified() < b.lastModified(); // Сортировка по дате (от ранней к поздней)
		}
		return a.fileName() < b.fileName(); // Если даты одинаковые, сортировка по имени (по возрастанию)
	};
	static auto cmpRead = [](const QFileInfo &a, const QFileInfo &b){
		if (a.lastModified() != b.lastModified()) {
			return a.lastRead() < b.lastRead(); // Сортировка по дате (от ранней к поздней)
		}
		return a.fileName() < b.fileName(); // Если даты одинаковые, сортировка по имени (по возрастанию)
	};

	if(sortFlag == name)
		std::sort(content.begin(),content.end(),cmpName);
	else if(sortFlag == modified)
		std::sort(content.begin(),content.end(),cmpModified);
	else if(sortFlag == read)
		std::sort(content.begin(),content.end(),cmpRead);
	else if(sortFlag == noSort) {}
	else ret += "unrealesed sort flag ("+QString::number(sortFlag)+")";

	while (content.size() > remainCount) {
		if(!QFile(content.front().filePath()).remove())
			ret += "can't remove file ["+content.back().filePath()+"]\n";
		content.removeFirst();
	}
	return ret;
}

void MyQFileDir::RemoveOldFilesByDaysThreshold(const QString &directory, int daysThresholdToRemove,
											   bool removeEmptySubcats, SortFlags daysThresholdType)
{
	QDir dir(directory);
	if (!dir.exists()) return;

	QDateTime currentDate = QDateTime::currentDateTime();

	QDirIterator it(directory, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	while(it.hasNext())
	{
		it.next();
		QFileInfo fileInfo = it.fileInfo();
		QDateTime dateTimeOfFile = fileInfo.lastModified();

		if(daysThresholdType == modified) dateTimeOfFile = fileInfo.lastModified();
		else if(daysThresholdType == read) dateTimeOfFile = fileInfo.lastRead();
		else qdbg << "RemoveOldFiles2: error, wrong daysThresholdType " + QSn(daysThresholdType);

		qint64 daysDiffrence = dateTimeOfFile.daysTo(currentDate);

		if(daysDiffrence > daysThresholdToRemove)
		{
			QFile::remove(fileInfo.absoluteFilePath());
		}
	}

	if(removeEmptySubcats) RemoveEmptySubcats(directory);
}

void MyQFileDir::RemoveEmptySubcats(const QString &directory)
{
    QDir dir(directory);
    const QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &subDir : subDirs)
    {
	RemoveEmptySubcats(dir.filePath(subDir));
    }

    if(dir.isEmpty())
	dir.rmdir(".");
}

inline bool MyQFileDir::RemoveDirIfEmpty(const QString &dirStr, bool ShowErrorMessage)
{
	if(QDir dir(dirStr); dir.isEmpty())
	{
		if(!dir.rmdir(dirStr))
		{
			if(ShowErrorMessage) QMbError("Err removing dir " + dirStr);
			return false;
		}
	}
	return true;
}

void MyQFileDir::ReplaceFileWithBackup(const QFileInfo & src, const QFileInfo & dst, const QString & backupPath)
{
	QFile fileToReplace(dst.filePath());
	QString dateFormat = "yyyy.MM.dd hh:mm:ss:zzz";
	QString backupFile = backupPath + "/" + QDateTime::currentDateTime().toString(dateFormat).replace(':','.') + " " + dst.fileName();
	if(!fileToReplace.copy(backupFile)) QMessageBox::information(nullptr,"Ошибка","Не удалось создать backup-файл" + backupFile);
	else
	{
		if(!fileToReplace.remove()) QMessageBox::information(nullptr,"Ошибка","Не удалось удалить файл " + fileToReplace.fileName());
		if(!QFile::copy(src.filePath(),dst.filePath()))
			QMessageBox::information(nullptr,"Ошибка","Не удалось создать файл " + dst.fileName());
	}
}

void MyQFileDir::ReplaceFilesWithBackup(const QFileInfoList & filesToReplace, const QFileInfo & fileSrc, const QString & backupPath)
{
	// формруем QString с именами файлов, которые будут заменены
	QString filestpReplaceStr;
	QString dateFormat = "yyyy.MM.dd hh:mm:ss:zzz";
	for(auto &f:filesToReplace)
	{
		QString fileStrPlus = f.filePath();
		filestpReplaceStr += fileStrPlus + "    (" + f.lastModified().toString(dateFormat) + ")\n";
	}

	QString replaceFileStr = fileSrc.filePath() + "    (" + fileSrc.lastModified().toString(dateFormat) + ")";
	if(QMessageBox::question(nullptr, "Замена файлов", "Заменить файлы:\n" + filestpReplaceStr + "\nфайлом:\n" + replaceFileStr
							 + "?\n(Резервные копии будут сохранены)",
							 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
	{
		for(auto &f:filesToReplace)
		{
			ReplaceFileWithBackup(fileSrc, f, backupPath);
		}
	}
}

QStringList MyQFileDir::GetAllNestedDirs(QString path, QDir::Filters filters, QDir::SortFlag sort)
{
	if(!(filters & QDir::Dirs)) DO_ONCE(qdbg << "MyQFileDir::GetAllNestedDirs called without dir filter, files can be in output");

	QDir dir(path);
	QStringList res;
	QStringList subdirs = dir.entryList(filters, sort);
	for (const auto &subdir : subdirs) {
		res << path + "/" + subdir;
		res += GetAllNestedDirs(path + "/" + subdir, filters, sort);
	}
	return res;
}

QFileInfoList MyQFileDir::GetAllFilesIncludeSubcats(QString path, const QStringList &extFilter)
{
	QDir dir(path);
	QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

	if(!extFilter.isEmpty())
	{
		auto removeRes = std::remove_if(files.begin(), files.end(),
										[&extFilter](const QFileInfo &fi){ return !extFilter.contains(fi.suffix()); });
		files.erase(removeRes, files.end());
	}

	QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (const auto &subdir : subdirs)
	{
		files += GetAllFilesIncludeSubcats(path + "/" + subdir, extFilter);
	}

	return files;
}

QStringList MyQFileDir::FileInfoListToStrList(const QFileInfoList &fileInfoList)
{
	QStringList filesList;
	for(auto &fi:fileInfoList) filesList+= fi.filePath();
	return filesList;
}

QFileInfoList MyQFileDir::StrListToFileInfoList(const QStringList &filesList)
{
	QFileInfoList filesInfoList;
	for(auto &file:filesList) filesInfoList.push_back(QFileInfo(file));
	return filesInfoList;
}

bool MyQFileDir::CopyDirectory(QString srcDirectory, QString pathDestination, QString newName, MyQFileDir::cbProgress_t progress)
{
	QDir srcDir(srcDirectory);

	if(!srcDir.exists()) { qWarning() << "CopyDirectory: " + srcDirectory + " not exists"; return false; }
	if(!QDir(pathDestination).exists()) { qWarning() << "CopyDirectory: " + pathDestination + " not exists"; return false; }

	if(newName.isEmpty()) newName = srcDir.dirName();

	QString newDirectoryStr = pathDestination + "/" + newName;

	if(!QDir().exists(newDirectoryStr))
		if(!QDir().mkdir(newDirectoryStr)) { qWarning() << "CopyDirectory: can't create dir " + newDirectoryStr; return false; }

	int i=0;
	QDir currentDir(newDirectoryStr);
	QDirIterator it(srcDirectory, QStringList(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	while(it.hasNext())
	{
		auto fileOrDir = it.next();
		QDir copyingDir(fileOrDir);
		if(copyingDir.exists())
		{
			QString newDirStr = newDirectoryStr + QString(fileOrDir).remove(0,srcDirectory.length());
			currentDir.setPath(newDirStr);
			if(!currentDir.exists())
			{
				if(!QDir().mkdir(newDirStr))
				{
					qWarning() << "CopyDirectory: can't create subdir " + currentDir.path();
					return false;
				}
			}
		}
		else
		{
			QFile srcFile(fileOrDir);
			if(srcFile.exists())
			{
				QFile dstFile(newDirectoryStr + QString(fileOrDir).remove(0,srcDirectory.length()));
				if(dstFile.exists())
				{
					if(!dstFile.remove())
					{
						qWarning() << "CopyDirectory: can't delete existing dst file " + fileOrDir;
						return false;
					}
				}

				if(!srcFile.copy(dstFile.fileName()))
				{
					qWarning() << "CopyDirectory: can't copy srcFile to dst file " + fileOrDir;
				}
			}
			else
			{
				qWarning() << "CopyDirectory: src file not exists " + fileOrDir;
				return false;
			}
		}
		if(progress) progress(++i);
	}
	return true;
}

bool MyQFileDir::WriteFile(const QString & fileName, const QString & content, const char * encoding)
{
	QFile file(fileName);
	if(file.open(QFile::WriteOnly))
	{
		QTextStream stream(&file);

		if(encoding != nullptr && strcmp(encoding, "") != 0)
		{
			if(auto codec = QTextCodec::codecForName(encoding)) stream.setCodec(codec);
			else
			{
				qCritical() << QString("MyQFileDir::WriteFile unknown codec [") +encoding+"]";
				return false;
			}
		}

		stream << content;
		return true;
	}
	qCritical() << "MyQFileDir::WriteFile can't open file ["+fileName+"]";
	return false;
}

bool MyQFileDir::AppendFile(const QString & fileName, const QString & content, const char * encoding)
{
	QFile file(fileName);
	if(file.open(QFile::Append))
	{
		QTextStream stream(&file);

		if(encoding != nullptr && strcmp(encoding, "") != 0)
		{
			if(auto codec = QTextCodec::codecForName(encoding)) stream.setCodec(codec);
			else
			{
				qCritical() << QString("MyQFileDir::WriteFile unknown codec [") +encoding+"]";
				return false;
			}
		}

		stream << content;
		return true;
	}
	qCritical() << "MyQFileDir::AppendFile can't open file ["+fileName+"]";
	return false;
}

QString MyQFileDir::ReadFile1(const QString & fileName, const char * encoding, bool * success)
{
	if(success) *success = true;
	QFile file(fileName);
	if(file.open(QFile::ReadOnly))
	{
		QTextStream stream(&file);

		if(encoding != nullptr && strcmp(encoding, "") != 0)
		{
			if(auto codec = QTextCodec::codecForName(encoding)) stream.setCodec(codec);
			else
			{
				qCritical() << QString("MyQFileDir::WriteFile unknown codec [") +encoding+"]";
				return "";
			}
		}

		return stream.readAll();
	}
	if(success) *success = false;
	qCritical() << "MyQFileDir::ReadFile1 can't open file ["+fileName+"]";
	return "";
}

MyQFileDir::ReadResult MyQFileDir::ReadFile2(const QString & fileName, const char * encoding)
{
	QFile file(fileName);
	if(file.open(QFile::ReadOnly))
	{
		QTextStream stream(&file);

		if(encoding != nullptr && strcmp(encoding, "") != 0)
		{
			if(auto codec = QTextCodec::codecForName(encoding)) stream.setCodec(codec);
			else
			{
				qCritical() << QString("MyQFileDir::WriteFile unknown codec [") +encoding+"]";
				return { false, "" };
			}
		}

		return { true, stream.readAll() };
	}
	qCritical() << "MyQFileDir::ReadFile2 can't open file ["+fileName+"]";
	return { false, "" };
}

bool MyQFileDir::CopyFileWithReplace(const QString & fileSrc, const QString & fileDst)
{
	if(QFileInfo fileDstInfo(fileDst); fileDstInfo.exists())
	{
		if(fileDstInfo.isFile())
		{
			if(!QFile::remove(fileDst))
			{
				qDebug() << "MyQFileDir::CopyFileWithReplace can't remove existing dst [" + fileDst + "]";
				return false;
			}
		}
		else
		{
			qDebug() << "MyQFileDir::CopyFileWithReplace existing dst is not file [" + fileDst + "]";
			return false;
		}
	}

	if(!QFile::copy(fileSrc, fileDst))
	{
		qDebug() << "MyQFileDir::CopyFileWithReplace can't copy ["+fileSrc+"] to ["+fileDst+"]";
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
