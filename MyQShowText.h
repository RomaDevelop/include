#ifndef MYQSHOWTEXT_H
#define MYQSHOWTEXT_H

#include <QString>
#include <QDialog>
#include <QHBoxLayout>
#include <QTextBrowser>

class ShowText
{
public:
	static void Show(QString text, uint w = 0, uint h = 0)
	{
		QDialog *dialog = new QDialog;
		QHBoxLayout *all  = new QHBoxLayout(dialog);
		QTextBrowser *tb = new QTextBrowser;
		tb->setPlainText(text);
		all->addWidget(tb);

		if(h && w) dialog->resize(w, h);
		dialog->exec();
		delete tb;
		delete all;
		delete dialog;
	}
};

#define ShT ShowText::Show

#endif // MYQSHOWTEXT_H
