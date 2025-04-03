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
};

//------------------------------------------------------------------------------------------------------------------------------

void MyQWidget::SetFontPointSize(QWidget *w, int fontSize)
{
	auto font = w->font();
	font.setPointSize(fontSize);
	w->setFont(font);
}


#endif
