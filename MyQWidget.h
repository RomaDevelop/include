#ifndef MyQWidget_H
#define MyQWidget_H

#include <QDebug>
#include <QStringList>
#include <QFontMetrics>
#include <QRect>
#include <QPalette>
#include <QWidget>
#include <QAction>

#include "CodeMarkers.h"

struct MyQWidget
{
	inline static void SetFontPointSize(QWidget *obj, int fontSize);
	inline static void SetFontPointSize(QAction *obj, int fontSize);
	inline static void SetTextColor_palette(QWidget *obj, const QColor &color);
	inline static void SetFontBold(QWidget *obj, bool enable);
	inline static void SetFontBold(QAction *obj, bool enable);
	/* Можно сделать шаблонную функцию, но тогда копилятор будет создавать функции для каждого наследника QWidget, а их очень много
	 * Когда перейдем на С++ 20, попробовать через концепты этого избежать */
};

//------------------------------------------------------------------------------------------------------------------------------------------

void MyQWidget::SetFontPointSize(QWidget *obj, int fontSize)
{
	auto font = obj->font();
	font.setPointSize(fontSize);
	obj->setFont(font);
}

void MyQWidget::SetFontPointSize(QAction *obj, int fontSize)
{
	if(0) CodeMarkers::to_do_with_cpp20("перейти на шаблон с концептом, см. комментарий в классе");
	auto font = obj->font();
	font.setPointSize(fontSize);
	obj->setFont(font);
}

void MyQWidget::SetTextColor_palette(QWidget *obj, const QColor &color)
{
	QPalette palette = obj->palette();
	palette.setColor(QPalette::Text, color);
	obj->setPalette(palette);
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

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
