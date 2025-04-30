#ifndef MyQWidget_H
#define MyQWidget_H

#include <QDebug>
#include <QStringList>
#include <QFontMetrics>
#include <QRect>
#include <QWidget>
#include <QAction>

struct MyQWidget
{
	inline static void SetFontPointSize(QWidget *obj, int fontSize);
	inline static void SetFontPointSize(QAction *obj, int fontSize);
	inline static void SetFontBold(QWidget *obj, bool enable);
	inline static void SetFontBold(QAction *obj, bool enable);
};

//------------------------------------------------------------------------------------------------------------------------------

void MyQWidget::SetFontPointSize(QWidget *obj, int fontSize)
{
	auto font = obj->font();
	font.setPointSize(fontSize);
	obj->setFont(font);
}

void MyQWidget::SetFontPointSize(QAction *obj, int fontSize)
{
	auto font = obj->font();
	font.setPointSize(fontSize);
	obj->setFont(font);
}

void MyQWidget::SetFontBold(QWidget *obj, bool enable)
{
	auto font = obj->font();
	font.setBold(enable);
	obj->setFont(font);
}

void MyQWidget::SetFontBold(QAction *obj, bool enable)
{
	auto font = obj->font();
	font.setBold(enable);
	obj->setFont(font);
}

#endif
