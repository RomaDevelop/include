#ifndef MYQDIALOGS_H
#define MYQDIALOGS_H
//---------------------------------------------------------------------------
#include <memory>

#include <QApplication>
#include <QString>
#include <QDialog>
#include <QDebug>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QTableWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QTimer>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>

#include "MyQShortings.h"
#include "MyQWidget.h"
#include "declare_struct.h"
//---------------------------------------------------------------------------
struct CheckBoxDialogResult;
//---------------------------------------------------------------------------
class MyQDialogs
{
public:
    inline static void ShowText(const QString &text, uint w = 800, uint h = 600);

    inline static QString CustomDialog(QString caption, QString text, QStringList buttons);

	declare_struct_2_fields_move(MenuItem, QString, text, std::function<void()>, worker);
	inline static void MenuUnderWidget(QWidget *w, std::vector<MenuItem> items);
	inline static void MenuUnderWidget(QWidget *w, QStringList menuItems, std::vector<std::function<void()>> workers);

    inline static QString InputText(QString captionDialog = "", QString startText = "", uint w = 0, uint h = 0);

	inline static QString InputLine(QString captionDialog = "", QString textDialog = "", QString startText = "", uint w = 0);
    declare_struct_2_fields_move(InputLineRes, QString, line, QString, button);
    inline static const QString& Accept() { static QString str = "Принять"; return str; }
    inline static const QString& Cansel() { static QString str = "Отмена"; return str; }
	inline static InputLineRes InputLineExt(QString captionDialog = "", QString textDialog = "", QString startText = "",
											QStringList buttons = {Accept(),Cansel()}, uint w = 0);

    declare_struct_2_fields_move(ListDialogRes, int, index, QString, text);
    inline static ListDialogRes ListDialog(QString caption, QStringList valuesList,  uint w = 0, uint h = 0);
    inline static ListDialogRes ListDialog(QString caption, QString valuesList, QString splitter, uint w = 0, uint h = 0);

    inline static CheckBoxDialogResult CheckBoxDialog(const QString &caption,
                                                      const QStringList &values,
                                                      const std::vector<bool> &startCheched = {},
                                                      const std::vector<bool> &enabled = {},
                                                      QWidget *parent = nullptr);

	inline static std::unique_ptr<QTableWidget> Table(const std::vector<QStringList> &rows,
                                               QStringList horisontalHeader = {}, QStringList verticaHeader = {},
                                               bool autoColWidths = true,
                                               uint w = 800, uint h = 600);
	inline static std::unique_ptr<QTableWidget> Table(QString content, QString colSplitter, QString rowSplitter,
                                               QStringList horisontalHeader = {}, QStringList verticaHeader = {},
                                               bool autoColWidths = true,
                                               uint w = 800, uint h = 600);
	inline static std::unique_ptr<QTableWidget> TableOneCol(QStringList rows,
                                                     QStringList horisontalHeader = {}, QStringList verticaHeader = {},
                                                     bool autoColWidths = true,
                                                     uint w = 800, uint h = 600);

	inline static void ShowAllStandartIcons();
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

void MyQDialogs::MenuUnderWidget(QWidget *w, std::vector<MenuItem> items)
{
	static std::vector<MenuItem> staticItems;
	staticItems = std::move(items);

	QMenu *menu = new QMenu(w);
	for(auto &item:staticItems)
	{
		QAction *action = new QAction(item.text, menu);
		menu->addAction(action);
		QObject::connect(action, &QAction::triggered, [action](){
			for(uint i=0; i<staticItems.size(); i++)
			{
				if(staticItems[i].text == action->text())
				{
					staticItems[i].worker();
					return;
				}
			}
			QMbError("MenuUnderWidget: error, action text not found in texts");
		});
	}
	menu->exec(w->mapToGlobal(QPoint(0, w->height())));
}

void MyQDialogs::MenuUnderWidget(QWidget */*w*/, QStringList /*menuItems*/, std::vector<std::function<void ()> > /*workers*/)
{
	QMbError("unrealesed");
}

QString MyQDialogs::InputText(QString captionDialog, QString startText,  uint w, uint h)
{
	std::unique_ptr<QDialog> dialog(new QDialog);
	bool accepted = false;
	dialog->setWindowTitle(captionDialog);
	QVBoxLayout *all  = new QVBoxLayout(dialog.get());
	QTextEdit *tb = new QTextEdit;
	tb->setText(startText);
	all->addWidget(tb);

	auto hloBtns = new QHBoxLayout;
	all->addLayout(hloBtns);

	hloBtns->addStretch();
	hloBtns->addWidget(new QPushButton("Принять"));
	QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&accepted, &dialog](){ accepted=true; dialog->close(); });
	hloBtns->addWidget(new QPushButton("Отмена"));
    QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&dialog](){ dialog->close(); });

    if(!w) w = 300;
    if(!h) h = 150;
    dialog->resize(w, h);
    dialog->exec();
    QString ret;
    if(accepted) ret = tb->toPlainText();

    return ret;
}

QString MyQDialogs::InputLine(QString captionDialog, QString textDialog, QString startText, uint w)
{
	auto res = InputLineExt(captionDialog, textDialog, startText, {Accept(),Cansel()}, w);
	if(res.button == Accept()) return res.line;
	return "";
}

MyQDialogs::InputLineRes MyQDialogs::InputLineExt(QString captionDialog, QString textDialog, QString startText, QStringList buttons, uint w)
{
    std::unique_ptr<QDialog> dialog(new QDialog);
    InputLineRes ret;
    dialog->setWindowTitle(captionDialog);
    QVBoxLayout *all  = new QVBoxLayout(dialog.get());

    QLabel *label = new QLabel(textDialog);
    all->addWidget(label);

    QLineEdit *lineEdit = new QLineEdit;
    lineEdit->setText(startText);
    all->addWidget(lineEdit);

    auto hloBtns = new QHBoxLayout;
    all->addLayout(hloBtns);

    hloBtns->addStretch();
    for(auto &btn:buttons)
    {
        hloBtns->addWidget(new QPushButton(btn));
        QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&ret, &btn, &dialog](){ ret.button = btn; dialog->close(); });
    }

    if(!w) w = 400;
    dialog->setFixedWidth(w);
    dialog->exec();
    ret.line = lineEdit->text();

    return ret;
}

MyQDialogs::ListDialogRes MyQDialogs::ListDialog(QString caption, QStringList valuesList, uint w, uint h)
{
    std::shared_ptr<int> retIndex(new int);     // because we need to give this in lambda
    std::shared_ptr<QString> retText(new QString);  // same
    std::unique_ptr<QDialog> dialog(new QDialog);
    if(!w) w = 650;
    if(!h) h = 340;
    dialog->resize(w, h);
    dialog->setWindowTitle(caption);
    QVBoxLayout *vloMain  = new QVBoxLayout(dialog.get());
    QListWidget *listWidget = new QListWidget;
    listWidget->addItems(valuesList);
    vloMain->addWidget(listWidget);

    auto acceptAction = [retText, retIndex, listWidget, &dialog]()
    {
        if(auto item = listWidget->currentItem())
        {
            *retIndex = listWidget->currentRow();
            *retText = item->text();
        }
        dialog->close();
    };
    QObject::connect(listWidget, &QListWidget::itemDoubleClicked, acceptAction);

    auto hloBtns = new QHBoxLayout();
    vloMain->addLayout(hloBtns);
    hloBtns->addStretch();
    hloBtns->addWidget(new QPushButton("Принять"));
    QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, acceptAction);
    hloBtns->addWidget(new QPushButton("Отмена"));
    QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&dialog]() { dialog->close(); });

    dialog->exec();
    return ListDialogRes(*retIndex, *retText);
}

MyQDialogs::ListDialogRes MyQDialogs::ListDialog(QString caption, QString valuesList, QString splitter, uint w, uint h)
{
    if(valuesList.endsWith(splitter)) valuesList.chop(splitter.size());
    return ListDialog(caption, valuesList.split(splitter), w, h);
}

struct CheckBoxDialogItem
{
    QString text;
    bool chekState = false;
    CheckBoxDialogItem(QString text, bool chekState): text {std::move(text)}, chekState {chekState} {}
};

struct CheckBoxDialogResult
{
    bool accepted = false;

    std::vector<CheckBoxDialogItem> allItems;
    std::vector<CheckBoxDialogItem> checkedItems;
    QStringList checkedTexts;
};

CheckBoxDialogResult MyQDialogs::CheckBoxDialog(const QString &caption,
                                                const QStringList & values,
                                                const std::vector<bool> & startCheched,
                                                const std::vector<bool> & enabled,
                                                QWidget *parent)
{
    CheckBoxDialogResult result;

    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(caption);
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

inline std::unique_ptr<QTableWidget> MyQDialogs::Table(const std::vector<QStringList> &rows, QStringList horisontalHeader, QStringList verticaHeader, bool autoColWidths, uint w, uint h)
{
    /// что нужно:
    /// кнопки вверх, вниз, отменить изменения
    /// автоподгонка ширины колонок
    QDialog *dialog = new QDialog;
    if(!w) w = 150;
    if(!h) h = 150;
    dialog->resize(w, h);

    QVBoxLayout *all  = new QVBoxLayout(dialog);

    QTableWidget *table = new QTableWidget;

    QHBoxLayout *hlo1 = new QHBoxLayout;
    all->addLayout(hlo1);

    auto btnAdd = new QPushButton("+");
    btnAdd->setFixedWidth(QFontMetrics(btnAdd->font()).width(btnAdd->text()) + 15);
    hlo1->addWidget(btnAdd);
    btnAdd->connect(btnAdd, &QPushButton::clicked, [table](){ table->insertRow(table->currentRow()); });

    auto btnRemove = new QPushButton("-");
    btnRemove->setFixedWidth(QFontMetrics(btnRemove->font()).width(btnRemove->text()) + 15);
    hlo1->addWidget(btnRemove);
    btnRemove->connect(btnRemove, &QPushButton::clicked, [table](){ table->removeRow(table->currentRow()); });

    hlo1->addStretch();

    all->addWidget(table);

    int colsCount = 0;
    for(uint r=0; r<rows.size(); r++) if(rows[r].size() > colsCount) colsCount = rows[r].size();

    table->setRowCount(rows.size());
    table->setColumnCount(colsCount);
    for(uint r=0; r<rows.size(); r++)
    {
        for(int c=0; c<colsCount; c++)
        {
            if(c < rows[r].size())
                table->setItem(r,c, new QTableWidgetItem(rows[r][c]));
            else table->setItem(r,c, new QTableWidgetItem(""));
        }
    }

    if(autoColWidths)
    {
        QTimer::singleShot(1,[table](){
            int oneColWidth = (table->width() - 45) / table->columnCount();
            for(int col=0; col<table->columnCount(); col++)
            {
                table->setColumnWidth(col, oneColWidth);
            }
        });
    }

    table->setHorizontalHeaderLabels(horisontalHeader);
    table->setVerticalHeaderLabels(verticaHeader);

    dialog->exec();

    table->setParent(nullptr);

    delete dialog;
    return std::unique_ptr<QTableWidget>(table);
}

inline std::unique_ptr<QTableWidget> MyQDialogs::Table(QString content, QString colSplitter, QString rowSplitter, QStringList horisontalHeader, QStringList verticaHeader, bool autoColWidths, uint w, uint h)
{
    if(content.endsWith(colSplitter+rowSplitter)) content.chop(colSplitter.size()+rowSplitter.size());
    std::vector<QStringList> rowsTmp;
    QStringList splitByRows = content.split(rowSplitter);
    for(auto &row:splitByRows)
    {
        if(row.endsWith(colSplitter)) row.chop(colSplitter.size());
        rowsTmp.emplace_back(row.split(colSplitter));
    }
    return Table(rowsTmp, horisontalHeader, verticaHeader, autoColWidths, w, h);
}

inline std::unique_ptr<QTableWidget> MyQDialogs::TableOneCol(QStringList rows, QStringList horisontalHeader, QStringList verticaHeader, bool autoColWidths, uint w, uint h)
{
    std::vector<QStringList> rowsTmp;
    for(auto &row:rows)
        rowsTmp.emplace_back(std::move(row));
    return Table(rowsTmp, horisontalHeader, verticaHeader, autoColWidths, w, h);
}

void MyQDialogs::ShowAllStandartIcons()
{
	QWidget *w = new QWidget;
	w->setAttribute(Qt::WA_DeleteOnClose);
	auto hlo_main = new QHBoxLayout(w);
	static std::vector<QPixmap*> pixmaps;
	pixmaps.clear();
	QVBoxLayout *vlo = nullptr;

	if(QString(QT_VERSION_STR) == "5.12.10") ; // все норм, enum проверен
	else { QMbWarning("enum QStyle::SP_TitleBarMenuButton not checked for Qt " QT_VERSION_STR); }

	bool fillDummy = false;
	int start = QStyle::SP_TitleBarMenuButton;
	int max = QStyle::SP_LineEditClearButton;
	int addDummy = 10 - (max+1) % 10;
	for(int i=start, count = 0; i<=max; i++, count++)
	{
		if(count%10 == 0)
		{
			vlo = new QVBoxLayout;
			hlo_main->addLayout(vlo);
		}
		if(!vlo) { QMbError("!vlo"); return; }

		pixmaps.emplace_back(new QPixmap);
		QPixmap& pixmapRef = *pixmaps.back();

		if(!fillDummy) pixmapRef = QApplication::style()->standardIcon((QStyle::StandardPixmap)i).pixmap(50,50);

		auto hlo = new QHBoxLayout;
		vlo->addLayout(hlo);
		auto labelImage = new QLabel;
		labelImage->setAlignment(Qt::AlignCenter);
		MyQWidget::SetFontPointSize(labelImage,14);
		if(!fillDummy) labelImage->setPixmap(pixmapRef);
		else { labelImage->setText("0"); }
		hlo->addWidget(labelImage);
		//QStyle::StandardPixmap pixmapVar = (QStyle::StandardPixmap)i;
		hlo->addSpacing(10);
		auto labelCaption = new QLabel("i="+QSn(i)+" n="+QSn(count+1));
		hlo->addWidget(labelCaption);
		MyQWidget::SetFontPointSize(labelCaption,14);
		hlo->addSpacing(10);

		if(i == max && !fillDummy)
		{
			fillDummy = true;
			max += addDummy;
		}
	}
	w->move(200,200);
	w->show();
}

//---------------------------------------------------------------------------
#endif // MYQDIALOGS_H
