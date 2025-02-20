//---------------------------------------------------------------------------
#ifndef MYQSHORTINGS_H
#define MYQSHORTINGS_H
//---------------------------------------------------------------------------
#define QMb  QMessageBox::information
#define QMbi QMessageBox::information
#define QMbw QMessageBox::warning
#define QMbc QMessageBox::critical
#define QMbq QMessageBox::question
#define QMbInfo(text) QMessageBox::information(0, "Information", text)
#define QMbError(text) QMessageBox::critical(0, "Error", text)
//---------------------------------------------------------------------------
#define QSn QString::number
//---------------------------------------------------------------------------
template<class T>
bool IsInt(T QStringVariable) { bool ok; QStringVariable.toInt(&ok); return ok; }
template<class T>
bool IsUInt(T QStringVariable) { bool ok; QStringVariable.toUInt(&ok); return ok; }
//---------------------------------------------------------------------------
#define qDbg qDebug()
#define qdbg qDebug()
//---------------------------------------------------------------------------
#define LastAddedWidget(Grid, Type) static_cast<Type*>(Grid->itemAt(Grid->count()-1)->widget())
//---------------------------------------------------------------------------
#endif
