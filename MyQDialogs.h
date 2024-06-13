#ifndef MYQDIALOGS_H
#define MYQDIALOGS_H

#include <QString>
#include <QDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QTextBrowser>

class MyQDialogs
{
public:
    static QString CustomDialog(QString caption, QString text, QStringList buttons)
    {
	QMessageBox messageBox(QMessageBox::Question, caption, text);
	for(auto &btn:buttons)
	    messageBox.addButton(btn,QMessageBox::YesRole);  // Role не имеет значения
	int desision =  messageBox.exec(); // возвращает 0 1 2 по порядку кнопок
	return messageBox.buttons()[desision]->text();
    }

    static void ShowText(const QString &text, uint w = 0, uint h = 0)
    {
        QDialog *dialog = new QDialog;
        QHBoxLayout *all  = new QHBoxLayout(dialog);
        QTextBrowser *tb = new QTextBrowser;
        tb->setPlainText(text);
        all->addWidget(tb);

        if(!w) w = 150;
        if(!h) h = 150;
        dialog->resize(w, h);
        dialog->exec();
        delete tb;
        delete all;
        delete dialog;
    }

    static QString InputText(QString captionDialog = "", uint w = 0, uint h = 0)
    {
        if(captionDialog.size()) captionDialog = "InputText:" + captionDialog;
        else captionDialog = "InputText";
        QDialog *dialog = new QDialog;
        dialog->setWindowTitle(captionDialog);
        QHBoxLayout *all  = new QHBoxLayout(dialog);
        QTextEdit *tb = new QTextEdit;
        all->addWidget(tb);

        if(!w) w = 150;
        if(!h) h = 150;
        dialog->resize(w, h);
        dialog->exec();
        QString ret = tb->toPlainText();
        delete tb;
        delete all;
        delete dialog;

        return ret;
    }
};

#endif // MYQDIALOGS_H
