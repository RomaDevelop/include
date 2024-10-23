#ifndef MYQDIALOGS_H
#define MYQDIALOGS_H
//---------------------------------------------------------------------------
#include <QString>
#include <QDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QListWidget>
#include <QCheckBox>
#include <QDialogButtonBox>
//---------------------------------------------------------------------------
struct CheckBoxDialogResult;

class MyQDialogs
{
public:
    inline static void ShowText(const QString &text, uint w = 800, uint h = 600);
    inline static QString CustomDialog(QString caption, QString text, QStringList buttons);
    inline static QString InputText(QString captionDialog = "", uint w = 0, uint h = 0);
    inline static CheckBoxDialogResult CheckBoxDialog(const QStringList &values,
					       const std::vector<bool> &startCheched = {},
					       const std::vector<bool> &enabled = {},
					       QWidget *parent = nullptr);
};
//---------------------------------------------------------------------------
void MyQDialogs::ShowText(const QString & text, uint w, uint h)
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
    delete dialog;
}

QString MyQDialogs::CustomDialog(QString caption, QString text, QStringList buttons)
{
    QMessageBox messageBox(QMessageBox::Question, caption, text);
    for(auto &btn:buttons)
	messageBox.addButton(btn,QMessageBox::YesRole);  // Role не имеет значения
    int desision =  messageBox.exec(); // возвращает 0 1 2 по порядку кнопок
    return messageBox.buttons()[desision]->text();
}

QString MyQDialogs::InputText(QString captionDialog, uint w, uint h)
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
    delete dialog;

    return ret;
}

struct CheckBoxDialogItem
{
    QString text;
    bool chekState = false;
    CheckBoxDialogItem(QString text, bool chekState): text {text}, chekState {chekState} {}
};
struct CheckBoxDialogResult
{
    bool accepted = false;

    std::vector<CheckBoxDialogItem> allItems;
    std::vector<CheckBoxDialogItem> checkedItems;
    QStringList checkedTexts;
};

CheckBoxDialogResult MyQDialogs::CheckBoxDialog(const QStringList & values,
						const std::vector<bool> & startCheched,
						const std::vector<bool> & enabled,
						QWidget *parent)
{
    CheckBoxDialogResult result;

    QDialog *dialog = new QDialog(parent);
    auto loV = new QVBoxLayout(dialog);
    auto loH = new QHBoxLayout;
    auto lw = new QListWidget(dialog);
    auto chAl = new QCheckBox(dialog);
    auto chNo = new QCheckBox(dialog);

    auto dbb = new QDialogButtonBox(dialog);
    auto btnOk = new QPushButton("Ok", dialog);
    auto btnCa = new QPushButton("Отмена", dialog);
    QObject::connect(btnOk,&QPushButton::clicked,[dialog, &result](){ result.accepted = true; dialog->hide(); });
    QObject::connect(btnCa,&QPushButton::clicked,[dialog, &result](){ result.accepted = false; dialog->hide();});

    chAl->setChecked(true);
    QObject::connect(chAl,&QCheckBox::clicked,[lw, chAl](){
	    chAl->setChecked(true);
	    for(int i=0; i<lw->count(); i++)
		    lw->item(i)->setCheckState(Qt::Checked);
    });

    chNo->setChecked(false);
    QObject::connect(chNo,&QCheckBox::clicked,[lw, chNo](){
	    chNo->setChecked(false);
	    for(int i=0; i<lw->count(); i++)
		    lw->item(i)->setCheckState(Qt::Unchecked);
    });

    loV->addLayout(loH);
    loH->addWidget(chAl);
    loH->addWidget(chNo);
    loH->addStretch();
    loV->addWidget(lw);
    loV->addWidget(dbb);
    dbb->addButton(btnOk,QDialogButtonBox::AcceptRole);
    dbb->addButton(btnCa,QDialogButtonBox::AcceptRole);

    for(auto str:values)
    {
	    lw->addItem(str);
	    lw->item(lw->count()-1)->setCheckState(Qt::Unchecked);
    }

    for(int i=0; i<lw->count() && i<(int)startCheched.size(); i++)
	    if(startCheched[i]) lw->item(i)->setCheckState(Qt::Checked);

    for(int i=0; i<lw->count() && i<(int)enabled.size(); i++)
	    if(startCheched[i]) lw->item(i)->setFlags(lw->item(i)->flags() ^ Qt::ItemIsEnabled);

    dialog->exec();

    if(result.accepted)
    {
	for(int i=0; i<lw->count(); i++)
	{
	    result.allItems.emplace_back(lw->item(i)->text(), lw->item(i)->checkState() == Qt::Checked);
	    if(result.allItems.back().chekState)
	    {
		result.checkedItems.emplace_back(result.allItems.back());
		result.checkedTexts += result.allItems.back().text;
	    }
	}
    }

    return result;
}

//---------------------------------------------------------------------------
#endif // MYQDIALOGS_H
