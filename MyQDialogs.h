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
#include "CodeMarkers.h"
//---------------------------------------------------------------------------
class MyQDialogs
{
public:
    inline static void ShowText(const QString &text, uint w = 800, uint h = 600);

    inline static QString CustomDialog(QString caption, QString text, QStringList buttons);

	declare_struct_2_fields_move(MenuItem, QString, text, std::function<void()>, worker);
	inline static void MenuUnderWidget(QWidget *w, std::vector<MenuItem> items);
	inline static void MenuUnderWidget(QWidget *w, QStringList menuItems, std::vector<std::function<void()>> workers);

	declare_struct_2_fields_move(InputTextRes, QString, text, bool, accepted);
	inline static InputTextRes InputText(QString captionDialog = "", QString startText = "", uint w = 0, uint h = 0);
	inline static InputTextRes InputLine(QString captionDialog = "", QString textDialog = "", QString startText = "", uint w = 0);
	declare_struct_2_fields_move(InputLineResExt, QString, text, QString, button);
	inline static InputLineResExt InputLineExt(QString captionDialog = "", QString textDialog = "", QString startText = "",
											QStringList buttons = {Accept(),Cansel()}, uint w = 0);

	declare_struct_2_fields_move(ListDialogRes, int, index, QString, choosedText);
	inline static ListDialogRes ListDialog(QString caption, QStringList valuesList,  uint w = 0, uint h = 0); // returns -1 and empty text if cansel or close
    inline static ListDialogRes ListDialog(QString caption, QString valuesList, QString splitter, uint w = 0, uint h = 0);

	declare_struct_2_fields_move(CheckBoxDialogItem, QString, text, bool, chekState);
	declare_struct_4_fields_move(CheckBoxDialogResult,
								 std::vector<CheckBoxDialogItem>, allItems,
								 std::vector<CheckBoxDialogItem>, checkedItems,
								 QStringList, checkedTexts,
								 bool, accepted);

    inline static CheckBoxDialogResult CheckBoxDialog(const QString &caption,
                                                      const QStringList &values,
                                                      const std::vector<bool> &startCheched = {},
													  const std::vector<bool> &enabled = {});

	declare_struct_2_fields_move(TableDialogRes, std::unique_ptr<QTableWidget>, table, bool, accepted);
	inline static TableDialogRes Table(const std::vector<QStringList> &rows,
									   QStringList horisontalHeader = {}, QStringList verticaHeader = {},
									   bool autoColWidths = true,
									   uint w = 800, uint h = 600);
	inline static TableDialogRes Table(QString content, QString colSplitter, QString rowSplitter,
                                               QStringList horisontalHeader = {}, QStringList verticaHeader = {},
                                               bool autoColWidths = true,
                                               uint w = 800, uint h = 600);
	inline static TableDialogRes TableOneCol(QStringList rows,
                                                     QStringList horisontalHeader = {}, QStringList verticaHeader = {},
                                                     bool autoColWidths = true,
                                                     uint w = 800, uint h = 600);

	inline static void ShowAllStandartIcons();

	// buttons
	inline static const QString& Accept() { static QString str = "Принять"; return str; }
	inline static const QString& Cansel() { static QString str = "Отмена"; return str; }

	inline static const QString& Undefined() { static QString str = "Undefined"; return str; }
};
//---------------------------------------------------------------------------
void MyQDialogs::ShowText(const QString & text, uint w, uint h)
{
	std::unique_ptr<QDialog> dialog(new QDialog);
	QHBoxLayout *all  = new QHBoxLayout(dialog.get());
    QTextBrowser *tb = new QTextBrowser;
    tb->setPlainText(text);
    all->addWidget(tb);

    if(!w) w = 150;
    if(!h) h = 150;
    dialog->resize(w, h);
    dialog->exec();
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

MyQDialogs::InputTextRes MyQDialogs::InputText(QString captionDialog, QString startText,  uint w, uint h)
{
	std::unique_ptr<QDialog> dialog(new QDialog);
	InputTextRes res;
	dialog->setWindowTitle(captionDialog);
	QVBoxLayout *all  = new QVBoxLayout(dialog.get());
	QTextEdit *tb = new QTextEdit;
	tb->setText(startText);
	all->addWidget(tb);

	auto hloBtns = new QHBoxLayout;
	all->addLayout(hloBtns);

	hloBtns->addStretch();
	hloBtns->addWidget(new QPushButton(Accept()));
	QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&res, &dialog, tb](){
		res.accepted=true;
		res.text = tb->toPlainText();
		dialog->close();
	});
	hloBtns->addWidget(new QPushButton(Cansel()));
    QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&dialog](){ dialog->close(); });

	if(!w) w = 640;
	if(!h) h = 480;
    dialog->resize(w, h);
    dialog->exec();

	return res;
}

MyQDialogs::InputTextRes MyQDialogs::InputLine(QString captionDialog, QString textDialog, QString startText, uint w)
{
	auto resExt = InputLineExt(captionDialog, textDialog, startText, {Accept(),Cansel()}, w);
	if(resExt.button == Accept()) return InputTextRes(std::move(resExt.text), true);
	else return InputTextRes("", false);
}

MyQDialogs::InputLineResExt MyQDialogs::InputLineExt(QString captionDialog, QString textDialog, QString startText, QStringList buttons, uint w)
{
    std::unique_ptr<QDialog> dialog(new QDialog);
	InputLineResExt ret;
	ret.button = Undefined();
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
	for(auto &btnText:buttons)
    {
		hloBtns->addWidget(new QPushButton(btnText));
		QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&dialog, lineEdit, btnText, &ret](){
			ret.button = btnText;
			ret.text = lineEdit->text();
			dialog->close();
		});
    }

    if(!w) w = 400;
    dialog->setFixedWidth(w);
    dialog->exec();

    return ret;
}

MyQDialogs::ListDialogRes MyQDialogs::ListDialog(QString caption, QStringList valuesList, uint w, uint h)
{
	ListDialogRes res(-1,"");
    std::unique_ptr<QDialog> dialog(new QDialog);
    if(!w) w = 650;
    if(!h) h = 340;
    dialog->resize(w, h);
    dialog->setWindowTitle(caption);
    QVBoxLayout *vloMain  = new QVBoxLayout(dialog.get());
    QListWidget *listWidget = new QListWidget;
    listWidget->addItems(valuesList);
    vloMain->addWidget(listWidget);

	auto acceptAction = [&dialog, listWidget, &res]()
    {
        if(auto item = listWidget->currentItem())
        {
			res.index = listWidget->currentRow();
			res.choosedText = item->text();
        }
        dialog->close();
    };
    QObject::connect(listWidget, &QListWidget::itemDoubleClicked, acceptAction);

    auto hloBtns = new QHBoxLayout();
    vloMain->addLayout(hloBtns);
    hloBtns->addStretch();
	hloBtns->addWidget(new QPushButton(Accept()));
    QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, acceptAction);
	hloBtns->addWidget(new QPushButton(Cansel()));
    QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&dialog]() { dialog->close(); });

    dialog->exec();
	return res;
}

MyQDialogs::ListDialogRes MyQDialogs::ListDialog(QString caption, QString valuesList, QString splitter, uint w, uint h)
{
    if(valuesList.endsWith(splitter)) valuesList.chop(splitter.size());
    return ListDialog(caption, valuesList.split(splitter), w, h);
}

MyQDialogs::CheckBoxDialogResult MyQDialogs::CheckBoxDialog(const QString &caption,
                                                const QStringList & values,
                                                const std::vector<bool> & startCheched,
												const std::vector<bool> & enabled)
{
    CheckBoxDialogResult result;

	std::unique_ptr<QDialog> dialog(new QDialog);
    dialog->setWindowTitle(caption);
	auto loV = new QVBoxLayout(dialog.get());
    auto loH = new QHBoxLayout;
	auto lw = new QListWidget;
	auto chAl = new QCheckBox;
	auto chNo = new QCheckBox;

	auto dbb = new QDialogButtonBox;
	auto btnOk = new QPushButton(Accept());
	auto btnCa = new QPushButton(Cansel());
	QObject::connect(btnOk,&QPushButton::clicked,[&dialog, &result](){ result.accepted = true; dialog->hide(); });
	QObject::connect(btnCa,&QPushButton::clicked,[&dialog, &result](){ result.accepted = false; dialog->hide();});

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

MyQDialogs::TableDialogRes MyQDialogs::Table(const std::vector<QStringList> &rows, QStringList horisontalHeader, QStringList verticaHeader, bool autoColWidths, uint w, uint h)
{
	std::unique_ptr<QDialog> dialog(new QDialog);
	if(0) CodeMarkers::to_do("кнопки вверх, вниз, отменить изменения, автоподгонка ширины колонок");
	if(!w) w = 640;
	if(!h) h = 480;
	dialog->resize(w, h);

	TableDialogRes res;

	QVBoxLayout *vlo_main  = new QVBoxLayout(dialog.get());

	QTableWidget *table = new QTableWidget;
	res.table = std::unique_ptr<QTableWidget>(table);

	QHBoxLayout *hlo1 = new QHBoxLayout;
	vlo_main->addLayout(hlo1);

	auto btnAdd = new QPushButton("+");
	btnAdd->setFixedWidth(QFontMetrics(btnAdd->font()).width(btnAdd->text()) + 15);
	hlo1->addWidget(btnAdd);
	btnAdd->connect(btnAdd, &QPushButton::clicked, [table](){ table->insertRow(table->currentRow()); });

	auto btnRemove = new QPushButton("-");
	btnRemove->setFixedWidth(QFontMetrics(btnRemove->font()).width(btnRemove->text()) + 15);
	hlo1->addWidget(btnRemove);
	btnRemove->connect(btnRemove, &QPushButton::clicked, [table](){ table->removeRow(table->currentRow()); });

	hlo1->addStretch();

	vlo_main->addWidget(table);

	QHBoxLayout *hlo2 = new QHBoxLayout;
	vlo_main->addLayout(hlo2);

	hlo2->addStretch();
	auto btnAccept = new QPushButton(Accept());
	hlo2->addWidget(btnAccept);
	btnAccept->connect(btnAccept, &QPushButton::clicked, [&dialog, &res](){ res.accepted = true; dialog->close(); });

	auto btnCansel = new QPushButton(Cansel());
	hlo2->addWidget(btnCansel);
	btnCansel->connect(btnCansel, &QPushButton::clicked, [&dialog](){ dialog->close(); });

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

	return res;
}

MyQDialogs::TableDialogRes MyQDialogs::Table(QString content, QString colSplitter, QString rowSplitter,
											 QStringList horisontalHeader, QStringList verticaHeader,
											 bool autoColWidths, uint w, uint h)
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

MyQDialogs::TableDialogRes MyQDialogs::TableOneCol(QStringList rows, QStringList horisontalHeader, QStringList verticaHeader, bool autoColWidths, uint w, uint h)
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
