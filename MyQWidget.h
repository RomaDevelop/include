#ifndef MyQWidget_H
#define MyQWidget_H

#include <QApplication>
#include <QScreen>
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

	inline static void AdjustWidgetPosition(QWidget *widget, int edgeDistance = 0);
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

void MyQWidget::AdjustWidgetPosition(QWidget *widget, int edgeDistance) {
	// Получаем текущую позицию виджета
	QPoint pos = widget->pos();

	// Получаем экран, на котором находится виджет
	QScreen *screen = QApplication::screenAt(pos);
	if(!screen) screen = QApplication::primaryScreen();

	if(screen) {
		QRect screenGeometry = screen->geometry();

		// Проверяем, выходит ли виджет за границы экрана
		if(pos.x() < screenGeometry.left()+ edgeDistance) {
			pos.setX(screenGeometry.left() + edgeDistance);
		} else if (pos.x() + widget->width() > screenGeometry.right() - edgeDistance) {
			pos.setX(screenGeometry.right() - widget->width() - edgeDistance);
		}

		if(pos.y() < screenGeometry.top() + edgeDistance) {
			pos.setY(screenGeometry.top() + edgeDistance);
		} else if (pos.y() + widget->height() > screenGeometry.bottom() - edgeDistance) {
			pos.setY(screenGeometry.bottom() - widget->height() - edgeDistance);
		}

		// Устанавливаем новую позицию виджета
		widget->move(pos);
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
