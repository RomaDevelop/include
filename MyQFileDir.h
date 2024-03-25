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
//---------------------------------------------------------------------------
struct MQFD
{
    static QFileInfo GetNewestFI(const QFileInfoList &files)
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

    static void ReplaceFileWhithBacup(const QFileInfo &src, const QFileInfo &dst, const QString &backupPath)
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

    static void ReplaceFilesWithBackup(const QFileInfoList &filesToReplace, const QFileInfo &fileSrc, const QString &backupPath)
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
                ReplaceFileWhithBacup(fileSrc, f, backupPath);
            }
        }
    }

    static bool CreatePath(QString path)
    {
        path.replace('\\','/');
        auto catalogs = path.split("/", QString::SkipEmptyParts);
        if(catalogs.size() && QFileInfo::exists(catalogs[0]))
        {
            QString currentDir = catalogs[0];
            for(int i=1; i<catalogs.size(); i++)
            {
                currentDir += "/" + catalogs[i];
                if(!QFileInfo::exists(currentDir))
                    if(!QDir().mkdir(currentDir))
                    {
                        qDebug() << "Error. Can't create dir " + currentDir;
                        return false;
                    }
            }
        }
        else
        {
            qDebug() << "Error. Dir[0] does not exist " + path;
            return false;
        }

        return true;
    }
};
//---------------------------------------------------------------------------
#endif
