#ifndef MyQDropEvent_H
#define MyQDropEvent_H

#include <QDebug>
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
	if(event->mimeData()->hasUrls()) // обработка файлов
	{
		QList<QUrl> urlList = event->mimeData()->urls();
		if(urlList.size() != 1)
		{
			DropedObject obj;
			obj.error = "Обработка множественной передачи не предусмотрена!\n\nПолученные данные:\n"+text;
			return obj;
		}

		DropedObject obj;
		obj.initialValue = event->mimeData()->text();
		obj.workedValue = urlList.front().toLocalFile();

		obj.DefineType();

		return obj;
	}
	else
	{
		DropedObject obj;
		obj.error = "Обработка данных этого типа не предусмотрена.\n\nПолученные данные:\n"+text;
		return obj;
	}

	DropedObject obj;
	obj.error = "Программная ошибка, все верки должны возвращать значение";
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
