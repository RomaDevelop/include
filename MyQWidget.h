#ifndef MyQWidget_H
#define MyQWidget_H

#include <QDebug>
#include <QStringList>
#include <QFontMetrics>
#include <QRect>
#include <QWidget>

struct MyQWidget
{
	inline static void SetFontPointSize(QWidget *w, int fontSize);
	inline static void SetFontBold(QWidget *w, bool enable);
};

//------------------------------------------------------------------------------------------------------------------------------

void MyQWidget::SetFontPointSize(QWidget *w, int fontSize)
{
	auto font = w->font();
	font.setPointSize(fontSize);
	w->setFont(font);
}

void MyQWidget::SetFontBold(QWidget *w, bool enable)
{
	auto font = w->font();
	font.setBold(enable);
	w->setFont(font);
}


#endif
