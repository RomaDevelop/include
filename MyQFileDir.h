//---------------------------------------------------------------------------
#ifndef MQFILEDIR_H
#define MQFILEDIR_H
//---------------------------------------------------------------------------
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>
#include <QDirIterator>
#include <QTextCodec>
//---------------------------------------------------------------------------
struct MyQFileDir
{
    inline static QFileInfo FindNewest(const QFileInfoList &files);

    enum { modified = 1, read = 2, noSort = 0 };
    inline static QString RemoveOldFiles(QString directory, int remainCount, int sortFlag = MyQFileDir::modified);

    inline static void ReplaceFileWithBackup(const QFileInfo &src, const QFileInfo &dst, const QString &backupPath);
    inline static void ReplaceFilesWithBackup(const QFileInfoList &filesToReplace, const QFileInfo &fileSrc, const QString &backupPath);

    inline static QStringList GetAllNestedDirs(QString path);
    inline static QFileInfoList GetAllFilesIncludeSubcats(QString path);

    using cbProgress_t = std::function<void(int copied)>;
    inline static bool CopyDirectory(QString directory, QString pathDestination, QString newName = "", cbProgress_t progress = nullptr);
    // перезаписывает не спрашивая

    struct ReadResult { bool success = 0; QString content; };

    inline static bool		WriteFile(const QString &fileName, const QString &content, const char * encoding = "UTF-8");
    inline static bool		AppendFile(const QString &fileName, const QString &content, const char * encoding = "UTF-8");
    inline static QString	ReadFile(const QString &fileName, bool *success);
    inline static ReadResult	ReadFile(const QString &fileName);

    inline static bool		CopyFileWithReplace(const QString &file, const QString & fileDst);
};

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

QString MyQFileDir::RemoveOldFiles(QString directory, int remainCount, int sortFlag)
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

    auto cmpModified = [](const QFileInfo &fi1, const QFileInfo &fi2){ return fi1.lastModified() > fi2.lastModified();};
    auto cmpRead = [](const QFileInfo &fi1, const QFileInfo &fi2){ return fi1.lastRead() > fi2.lastRead();};

    if(sortFlag == modified)
	std::sort(content.begin(),content.end(),cmpModified);
    else if(sortFlag == read)
	std::sort(content.begin(),content.end(),cmpRead);
    else if(sortFlag == noSort) {}
    else ret += "wrong sort flag ("+QString::number(sortFlag)+")";

    while (content.size() > remainCount) {
	if(!QFile(content.back().filePath()).remove())
	    ret += "can't remove file ["+content.back().filePath()+"]\n";
	content.removeLast();
    }
    return ret;
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
	if(!QFile::copy(src.filePath(),dst.filePath())) QMessageBox::information(nullptr,"Ошибка","Не удалось создать файл " + dst.fileName());
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

QStringList MyQFileDir::GetAllNestedDirs(QString path)
{
    QDir dir(path);
    QStringList res;
    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &subdir : subdirs) {
	res << path + "/" + subdir;
	res += GetAllNestedDirs(path + "/" + subdir);
    }
    return res;
}

QFileInfoList MyQFileDir::GetAllFilesIncludeSubcats(QString path)
{
    QDir dir(path);
    auto files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const auto &subdir : subdirs)
    {
	files += GetAllFilesIncludeSubcats(path + "/" + subdir);
    }
    return files;
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
	if(auto codec = QTextCodec::codecForName(encoding))
	{
	    stream.setCodec(codec);
	    stream << content;
	    return true;
	}
	qCritical() << QString("MyQFileDir::WriteFile unknown codec [") +encoding+"]";
	return false;
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
	if(auto codec = QTextCodec::codecForName(encoding))
	{
	    stream.setCodec(codec);
	    stream << content;
	    return true;
	}
	qCritical() << QString("MyQFileDir::WriteFile unknown codec [") +encoding+"]";
	return false;
    }
    qCritical() << "MyQFileDir::WriteFile can't open file ["+fileName+"]";
    return false;
}

QString MyQFileDir::ReadFile(const QString & fileName, bool * success)
{
    if(success) *success = true;
    QFile file(fileName);
    if(file.open(QFile::ReadOnly))
    {
	QTextStream stream(&file);
	return stream.readAll();
    }
    if(success) *success = false;
    qCritical() << "MyQFileDir::ReadFile can't open file ["+fileName+"]";
    return "";
}

MyQFileDir::ReadResult MyQFileDir::ReadFile(const QString & fileName)
{
    QFile file(fileName);
    if(file.open(QFile::ReadOnly))
    {
	QTextStream stream(&file);
	return { true, stream.readAll() };
    }
    qCritical() << "MyQFileDir::ReadFile can't open file ["+fileName+"]";
    return { false, "" };
}

bool MyQFileDir::CopyFileWithReplace(const QString & fileSrc, const QString & fileDst)
{
    if(QFileInfo fileDstInfo(fileDst); fileDstInfo.exists())
    {
	if(fileDstInfo.isFile())
	{
	    if(!QFile::remove(fileDst)) { qDebug() << "MyQFileDir::CopyFileWithReplace can't remove existing dst [" + fileDst + "]"; return false; }
	}
	else { qDebug() << "MyQFileDir::CopyFileWithReplace existing dst is not file [" + fileDst + "]"; return false; }
    }

    if(!QFile::copy(fileSrc, fileDst)) { qDebug() << "MyQFileDir::CopyFileWithReplace can't copy ["+fileSrc+"] to ["+fileDst+"]"; return false; }

    return true;
}

//---------------------------------------------------------------------------
#endif
