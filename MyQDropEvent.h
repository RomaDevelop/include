#ifndef MyQDropEvent_H
#define MyQDropEvent_H

#include <QFileInfo>
#include <QDropEvent>
#include <QMimeData>

enum class DropedObjectType { file, dir, symLink, unknown };

struct DropedObject
{
	QString error;
	DropedObjectType type = DropedObjectType::unknown;
	QString initialValue;
	QString workedValue;
	QFileInfo fileInfo;

	inline void DefineType();
};

struct MyQDropEvent
{
	inline static DropedObject GetDropedObject(QDropEvent *event);
};

//----------------------------------------------------------------------------------------------------------------

DropedObject MyQDropEvent::GetDropedObject(QDropEvent *event)
{
	QString text = event->mimeData()->text();
	if(text.left(8) == "file:///")
	{
		DropedObject obj;
		obj.initialValue = std::move(text);
		obj.workedValue = obj.initialValue;

		obj.workedValue.remove(0,8);
		obj.workedValue.replace("/","\\");

		obj.DefineType();

		return obj;
	}
	else if(text.left(7) == "file://")
	{
		DropedObject obj;
		obj.initialValue = std::move(text);
		obj.workedValue = obj.initialValue;

		obj.workedValue.remove(0,7);
		obj.workedValue.replace("/","\\");
		obj.workedValue.prepend("\\\\"); // это объект в сети, поэтому он начинается с двух слешей

		obj.DefineType();

		return obj;
	}
	else
	{
		DropedObject obj;
		obj.error = "Ошибка при обработке полученных данных.\n\nПолученные данные:\n"+text;
		return obj;
	}

	DropedObject obj;
	obj.error = "Программная ошибка";
	return obj;
}


void DropedObject::DefineType()
{
	fileInfo.setFile(workedValue);

	if(fileInfo.isSymLink()) type = DropedObjectType::symLink;
	else if(fileInfo.isFile()) type = DropedObjectType::file;
	else if(fileInfo.isDir()) type = DropedObjectType::dir;
	else error = "unknown type";
}

//----------------------------------------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------------------------------------
