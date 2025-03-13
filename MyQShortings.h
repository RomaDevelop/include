//---------------------------------------------------------------------------
#ifndef MYQSHORTINGS_H
#define MYQSHORTINGS_H
//---------------------------------------------------------------------------
#include <QString>
//---------------------------------------------------------------------------
#define QMb  QMessageBox::information
#define QMbi QMessageBox::information
#define QMbw QMessageBox::warning
#define QMbc QMessageBox::critical
#define QMbq QMessageBox::question
#define QMbInfo(text) QMessageBox::information(0, "Information", text)
#define QMbError(text) QMessageBox::critical(0, "Error", text)
//---------------------------------------------------------------------------
//#define QSn QString::number
template<class NumberType>
QString QSn(NumberType number) { return QString::number(number); }
//---------------------------------------------------------------------------
template<class QStringType>
bool IsInt(QStringType stringVariable) { bool ok; stringVariable.toInt(&ok); return ok; }
template<class QStringType>
bool IsUInt(QStringType stringVariable) { bool ok; stringVariable.toUInt(&ok); return ok; }
//---------------------------------------------------------------------------
#define qDbg qDebug()
#define qdbg qDebug()
//---------------------------------------------------------------------------
#define LastAddedWidget(Grid, Type) static_cast<Type*>(Grid->itemAt(Grid->count()-1)->widget())
//---------------------------------------------------------------------------
#define DateTimeFormat "yyyy.MM.dd hh:mm:ss"
#define DateTimeFormatForFileName "yyyy.MM.dd hh-mm-ss"
//---------------------------------------------------------------------------
#endif
