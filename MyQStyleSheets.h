//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MyQStyleSheets_H
#define MyQStyleSheets_H
//------------------------------------------------------------------------------------------------------------------------------------------
//#include <direct.h>
#include <algorithm>
#include <set>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QWidget>
#include <QDir>
#include <QDateTime>

#include "MyQShortings.h"
#include "MyQFileDir.h"
//------------------------------------------------------------------------------------------------------------------------------------------
struct MyQStyleSheets
{
	/// height 5, borderRadius 2 - ok
	inline static QString QScrollBar_horizontal_thined(int height, int borderRadius);
};

//------------------------------------------------------------------------------------------------------------------------------------------

QString MyQStyleSheets::QScrollBar_horizontal_thined(int height, int borderRadius)
{
	return QString(R"(
	              QScrollBar:horizontal {
	                  height: %1px;
	                  margin: 0px;
	                  background: transparent;
	              }

	              QScrollBar::handle:horizontal {
	                  background: #a0a0a0;
	                  min-width: 20px;
	                  border-radius: %2px;
	              }

	              QScrollBar::add-line:horizontal,
	              QScrollBar::sub-line:horizontal {
	                  width: 0px;
	                  background: none;
	              }
	          )")
	        .arg(height)    // Заменяем %1 на значение переменной height
	        .arg(borderRadius); // Заменяем %2 на значение переменной borderRadius;
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
