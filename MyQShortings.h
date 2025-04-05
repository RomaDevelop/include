//---------------------------------------------------------------------------
#ifndef MYQSHORTINGS_H
#define MYQSHORTINGS_H
//---------------------------------------------------------------------------
#define QMb  QMessageBox::information
#define QMbi QMessageBox::information
#define QMbw QMessageBox::warning
#define QMbc QMessageBox::critical
#define QMbq QMessageBox::question
#define QMbInfo(text) QMessageBox::information(nullptr, "Information", text)
#define QMbWarning(text) QMessageBox::warning(nullptr, "Warning", text)
#define QMbError(text) QMessageBox::critical(nullptr, "Error", text)
//---------------------------------------------------------------------------
#define QSn QString::number
//---------------------------------------------------------------------------
template<class QStringType>
bool IsInt(QStringType stringVariable) { bool ok; stringVariable.toInt(&ok); return ok; }
template<class QStringType>
bool IsUInt(QStringType stringVariable) { bool ok; stringVariable.toUInt(&ok); return ok; }
//---------------------------------------------------------------------------
#define qDbg qDebug()
#define qdbg qDebug()
//---------------------------------------------------------------------------
#define LastAddedWidget(Layout, Type) static_cast<Type*>(Layout->itemAt(Layout->count()-1)->widget())
//---------------------------------------------------------------------------
#define DateTimeFormat "yyyy.MM.dd hh:mm:ss"
#define DateTimeFormatForFileName "yyyy.MM.dd hh-mm-ss"
//---------------------------------------------------------------------------
#endif
