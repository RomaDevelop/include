//---------------------------------------------------------------------------
#ifndef MYQDIFFERENT_H
#define MYQDIFFERENT_H
//---------------------------------------------------------------------------
#include <direct.h>
#include <algorithm>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <QDateTime>

#include "MyQShortings.h"
#include "MyQFileDir.h"
//---------------------------------------------------------------------------
struct MyQDifferent
{
    inline static QString PathToExe() { return ExePath(); }
    inline static QString ExePath();
    inline static QString ExeNameNoPath();
    inline static QString ExeNameWithPath();
    inline static QString GetGeo(const QWidget &widget);
    inline static bool SetGeo(QString geoStr, QWidget &widget);
    inline static QString BytesToString(uint64_t bytesCount);

    inline static bool SaveSettings(QString fileName, const std::vector<QWidget*> &widgets, const QStringList &stringSettings);
    inline static bool LoadSettings(QString fileName, std::vector<QWidget*> &widgets, QStringList &stringSettings);

    inline static void GetPathName(QString file, QString *path, QString *name);
};

//---------------------------------------------------------------------------

QString MyQDifferent::ExePath()
{
    static QString path = QFileInfo(QCoreApplication::applicationFilePath()).path(); // альтернатива QDir::currentPath(), но если программа запущена из Qt игнорирует /debug
    return path;
}

QString MyQDifferent::ExeNameNoPath()
{
    static QString name = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
    return name;
}

QString MyQDifferent::ExeNameWithPath()
{
    static QString pathName = QCoreApplication::applicationFilePath();
    return pathName;
}

QString MyQDifferent::GetGeo(const QWidget &widget)
{
    return QSn(widget.x())+";"+QSn(widget.y())+";"+QSn(widget.width())+";"+QSn(widget.height());
}

bool MyQDifferent::SetGeo(QString geoStr, QWidget & widget)
{
    auto geo = geoStr.split(";",QString::SkipEmptyParts);
    if(geo.size() == 4)
    {
	bool boolResults[4];
	int vals[4] {geo[0].toInt(&boolResults[0]), geo[1].toInt(&boolResults[1]), geo[2].toInt(&boolResults[2]), geo[3].toInt(&boolResults[3]) };
	if(boolResults[0] && boolResults[1] && boolResults[2] && boolResults[3])
	{
	    widget.move(vals[0], vals[1]);
	    widget.resize(vals[2], vals[3]);
	    return true;
	}
	return false;
    }
    else return false;
}

QString MyQDifferent::BytesToString(uint64_t bytesCount)
{
    if(bytesCount > (uint64_t)1024 * 1024 * 1024 * 1024)
        return QSn(float(bytesCount) / float((uint64_t)1024 * 1024 * 1024 * 1024), 'f', 1) + " Tb";
    if(bytesCount > 1024 * 1024 * 1024) return QSn(float(bytesCount) / float(1024 * 1024 * 1024), 'f', 1) + " Gb";
    if(bytesCount > 1024 * 1024       ) return QSn(float(bytesCount) / float(1024 * 1024       ), 'f', 1) + " Mb";
    if(bytesCount > 1024              ) return QSn(float(bytesCount) / float(1024              ), 'f', 1) + " Kb";
    return QSn(bytesCount) + " b";
}

bool MyQDifferent::SaveSettings(QString fileName, const std::vector<QWidget *> & widgets, const QStringList & stringSettings)
{
    QString endSetting = "[endSetting]\n";
    QString endParam = "[p;] ";

    QString settings = "save_version 003" + endSetting;
    for(uint i=0; i<widgets.size(); i++)
    {
        QString class_name=widgets[i]->metaObject()->className();
        QString name=widgets[i]->objectName();
        QString value;

        if(0) {}
#ifdef QLINEEDIT_H
        else if(class_name=="QLineEdit") value = static_cast<QLineEdit*>(widgets[i])->text();
#endif
#ifdef QCHECKBOX_H
        else if(class_name=="QCheckBox") value = QString::number(static_cast<QCheckBox*>(widgets[i])->isChecked());
#endif
#ifdef QTEXTEDIT_H
	else if (class_name=="QTextEdit") value = static_cast<QTextEdit*>(widgets[i])->toPlainText();
#endif
	else qDebug() << "Сохранение объекта типа " + class_name + " не реализовано";

	if(value != "")
	{
	    if(value.contains(endSetting) || value.contains(endParam))
		qDebug() << "value widget "+class_name+" "+name+" contains endSetting or endParam of SaveSettings. Value = " + value;
	    else settings += "widget" + endParam + class_name + endParam + name + endParam + value + endParam + endSetting;
	}
    }
    for(int i=0; i<stringSettings.size(); i++)
    {
	if(stringSettings[i] != "")
	{
	    if(stringSettings[i].contains(endSetting) || stringSettings[i].contains(endParam))
		qDebug() << "stringSettings contains endSetting or endParam of SaveSettings. stringSettings = " + stringSettings[i];
	    else settings += "stringSettings" + endParam + stringSettings[i] + endParam + endSetting;
	}
    }

    QFile file(fileName);

    QTextStream stream(&file);
    if(!MyQFileDir::WriteFile(fileName, settings))
    {
	qDebug() << "Не удалось сохранить файл настроек "+fileName;
	return false;
    }
    return true;
}

bool MyQDifferent::LoadSettings(QString fileName, std::vector<QWidget *> & widgets, QStringList & stringSettings)
{
    QString endSetting = "[endSetting]\n";
    QString endParam = "[p;] ";

    QFile file(fileName);
    if(file.open(QIODevice::ReadOnly))
    {
	QString settings = file.readAll();

	auto settingsList = settings.split(endSetting,QString::SkipEmptyParts);

	if(settingsList.size() && settingsList[0] == "save_version 003")
	{
	    for(int i=1; i<settingsList.size(); i++)
	    {
		auto paramsList = settingsList[i].split(endParam,QString::SkipEmptyParts);
		if(paramsList.size())
		{
		    QString settingType = paramsList[0];
		    if(settingType == "stringSettings" && paramsList.size() == 2)
			stringSettings += paramsList[1];
		    else if(settingType == "widget" && paramsList.size() == 4)
		    {
			QString className = paramsList[1];
			QString name = paramsList[2];
			QString value = paramsList[3];
			QWidget *component = nullptr;
			for(auto widg:widgets)
			    if(widg->objectName() == name) component = widg;
			if(component!=NULL)
			{
			    if(component->metaObject()->className() == className)
			    {
				if(0) {}
#ifdef QLINEEDIT_H
				else if(className=="QLineEdit") ((QLineEdit*)component)->setText(value);
#endif
#ifdef QCHECKBOX_H
				else if(className=="QCheckBox") ((QCheckBox*)component)->setChecked(value.toInt());
#endif
#ifdef QTEXTEDIT_H
				else if(className=="QTextEdit") ((QTextEdit*)component)->setPlainText(value);
#endif
				else qDebug() << "LoadSettings: Загрузка настроек типа " + className + " не реализована";
			    }
			    else qDebug() << "LoadSettings: component->className() != className" + name + className +" "+ component->metaObject()->className();
			}
			else qDebug() << "LoadSettings: При чтении настроек виджетов не обнаружен виджет " + className;
		    }
		    else qDebug() << "LoadSettings: Неизвестный settingType " + settingType;
		}
		else qDebug() << "LoadSettings: paramsList.size() == 0";
	    }
	}
	else
	{
	    qDebug() << "LoadSettings: Ошибка чтения настроек. Не известная версия сохранения или пусто.";
	    return false;
	}
    }
    else
    {
	qDebug() << "LoadSettings: Не удалось открыть файл настроек "+fileName;
	return false;
    }

    return true;
}

void MyQDifferent::GetPathName(QString file, QString *path, QString *name)
{
    QFileInfo fi(file);
    *path = fi.path();
    *name = fi.fileName();
}

#endif
//---------------------------------------------------------------------------

