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
#define qDbg qDebug()
#define qdbg qDebug()
//---------------------------------------------------------------------------
#endif
