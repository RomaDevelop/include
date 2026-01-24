//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MYQSHORTINGS_H
#define MYQSHORTINGS_H
//------------------------------------------------------------------------------------------------------------------------------------------
#include <QString>

#include "MyCppShortings.h"
//------------------------------------------------------------------------------------------------------------------------------------------
#define QMb  QMessageBox::information
#define QMbi QMessageBox::information
#define QMbw QMessageBox::warning
#define QMbc QMessageBox::critical
#define QMbq QMessageBox::question
#define QMbInfo(text) QMessageBox::information(nullptr, "Information", text)
#define QMbWarning(text) QMessageBox::warning(nullptr, "Warning", text)
#define QMbError(text) QMessageBox::critical(nullptr, "Error", text)
//------------------------------------------------------------------------------------------------------------------------------------------
#define QSn QString::number
//------------------------------------------------------------------------------------------------------------------------------------------
template<class QStringType>
bool IsInt(const QStringType &stringVariable) { bool ok; stringVariable.toInt(&ok); return ok; }
template<class QStringType>
bool IsUInt(const QStringType &stringVariable) { bool ok; stringVariable.toUInt(&ok); return ok; }
//------------------------------------------------------------------------------------------------------------------------------------------
#define qDbg qDebug()
#define qdbg qDebug()
//------------------------------------------------------------------------------------------------------------------------------------------
#define LastAddedWidget(Layout, Type) static_cast<Type*>(Layout->itemAt(Layout->count()-1)->widget())
//------------------------------------------------------------------------------------------------------------------------------------------
inline QString TimeFormat 					= QStringLiteral("hh:mm:ss");
inline QString Time_hms 					= QStringLiteral("hh:mm:ss");
inline QString Time_hm 						= QStringLiteral("hh:mm");

inline QString DateTimeFormat 				= QStringLiteral("yyyy.MM.dd hh:mm:ss");
inline QString DateTimeFormat_ms 			= QStringLiteral("yyyy.MM.dd hh:mm:ss.zzz");
inline QString DateTimeFormatForFileName 	= QStringLiteral("yyyy.MM.dd hh-mm-ss");
inline QString DateTimeFormatForFileName_ms = QStringLiteral("yyyy.MM.dd hh-mm-ss-zzz");

inline QString DateFormat 					= QStringLiteral("yyyy.MM.dd");

inline QString DateFormat_rus 				= QStringLiteral("dd MMM yyyy");
inline QString DateTimeFormat_rus 			= QStringLiteral("dd MMM yyyy hh:mm:ss");
//------------------------------------------------------------------------------------------------------------------------------------------
#define DO_ONCE(code_to_do) { static bool did = false; if(!did) { code_to_do; did = true; } }
//------------------------------------------------------------------------------------------------------------------------------------------
#define DECLARE_TIME_MARKER(marker_identtificator) auto marker_identtificator = QDateTime::currentDateTime();
#define CALC_MILLISECONDS(time_maker) time_maker.msecsTo(QDateTime::currentDateTime())
#define TIME_CALC_TO_QString(prefix_str, time_maker) \
	QString(  QString(prefix_str) + " " + QString::number(CALC_MILLISECONDS(time_maker)) + " milliseconds")
//------------------------------------------------------------------------------------------------------------------------------------------
#endif
