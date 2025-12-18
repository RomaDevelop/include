#ifndef MYQDIALOGS_H
#define MYQDIALOGS_H
//------------------------------------------------------------------------------------------------------------------------------------------
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
#include <QHeaderView>
#include <QListWidget>
#include <QCheckBox>
#include <QTimer>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>

#include "MyQShortings.h"
#include "MyQString.h"
#include "MyQWidget.h"
#include "MyQTableWidget.h"
#include "declare_struct.h"
#include "CodeMarkers.h"
//------------------------------------------------------------------------------------------------------------------------------------------
class MyQDialogs
{
public:
	inline static void ShowText(const QString &text, uint w = 800, uint h = 600);
	inline static void ShowText(const QStringList &text, uint w = 800, uint h = 600);

	inline static QString CustomDialog(QString caption, QString text, QStringList buttons);
	inline static QString CustomDialogWithCheckBox(QString caption, QString text, QStringList buttons,
												   bool *chBoxRes=nullptr, QString checkBoxText="Apply to all current");
	/// returns empty string if Esc pressed
	/// defaultButtonIndex can take -1 to disable timer mechanics
	inline static QString CustomDialogWithTimer(QString caption, QString text, QStringList buttons,
												int defaultButtonIndex, uint secondsToDefaultPressed);

	declare_struct_2_fields_move(MenuItem, QString, text, std::function<void()>, worker);
	inline static void MenuUnderWidget(QWidget *w, std::vector<MenuItem> &&items);
	inline static void MenuUnderWidget(QWidget *w, QStringList texts, std::vector<std::function<void()>> workers);
	inline static MenuItem SeparatorMenuItem() { return MenuItem("SeparatorMenuItem", nullptr); }
	inline static MenuItem DisabledItem(QString text) { text+=DisabledItemMarker(); return MenuItem(std::move(text), nullptr);  }
	inline static const QString& DisabledItemMarker() { static QString str = "[!DisabledItem!]"; return str; }

	declare_struct_2_fields_move(InputTextRes, QString, text, bool, accepted);
	inline static InputTextRes InputText(QString captionDialog = "", QString startText = "", uint w = 640, uint h = 480);
	inline static InputTextRes InputLine(QString captionDialog = "", QString textDialog = "", QString startText = "", uint w = 640);
	declare_struct_2_fields_move(InputLineResExt, QString, text, QString, button);
	inline static InputLineResExt InputLineExt(QString captionDialog = "", QString textDialog = "", QString startText = "",
											   QStringList buttons = {Accept(),Cansel()}, uint w = 640);

	// -1 index and empty text if cansel or close
	declare_struct_3_fields_move(ListDialogRes, int, chosenIndex, QString, chosenText, bool, accepted);
	inline static ListDialogRes ListDialog(QString caption, QStringList valuesList,
	                                       QString acceptButton = Accept(), QString canselButton = Cansel(), uint w = 640, uint h = 480);
	inline static ListDialogRes ListDialog(QString caption, QString valuesList, QString splitter,
	                                       QString acceptButton = Accept(), QString canselButton = Cansel(), uint w = 640, uint h = 480);

	declare_struct_3_fields_move(CheckBoxDialogItem, QString, text, bool, checkState, bool, enabled);
	declare_struct_5_fields_move(CheckBoxDialogResult,
								 std::vector<CheckBoxDialogItem>, allItems,
								 std::vector<CheckBoxDialogItem>, checkedItems,
	                             std::vector<int>, checkedIndexes,
								 QStringList, checkedTexts,
								 bool, accepted);

	inline static CheckBoxDialogResult CheckBoxDialog(const QString &caption, std::vector<CheckBoxDialogItem> items, uint w=640, uint h=480);
	inline static CheckBoxDialogResult CheckBoxDialog(const QString &caption,
													  const QStringList &values,
	                                                  const std::vector<bool> &startChecked = {},
													  const std::vector<bool> &enabled = {},
	                                                  bool startAllChecked  = false,
													  uint w = 640, uint h = 480);

	declare_struct_2_fields_move(TableDialogRes, std::unique_ptr<QTableWidget>, table, bool, accepted);
	inline static TableDialogRes Table(const QString &caption, const std::vector<QStringList> &rows,
									   QStringList horisontalHeader = {}, QStringList verticalHeader = {},
									   bool autoColWidths = true, bool readOnly = false,
									   uint w = 800, uint h = 600);
	inline static TableDialogRes Table(const QString &caption, QString content, QString colSplitter, QString rowSplitter,
									   QStringList horisontalHeader = {}, QStringList verticalHeader = {},
									   bool autoColWidths = true, bool readOnly = false,
									   uint w = 800, uint h = 600);
	inline static TableDialogRes TableOneCol(const QString &caption, QStringList rows,
											 QStringList horisontalHeader = {}, QStringList verticalHeader = {},
											 bool autoColWidths = true, bool readOnly = false,
											 uint w = 800, uint h = 600);

	inline static void InfoBar(const QString &message, QWidget *widgetToShowIn);

	inline static void ShowAllStandartIcons();

	// buttons
	inline static const QString& Accept() { static QString str = "Принять"; return str; }
	inline static const QString& Cansel() { static QString str = "Отмена"; return str; }
	inline static const QString& Close() { static QString str = "Закрыть"; return str; }

	inline static const QString& Undefined() { static QString str = "Undefined"; return str; }
};
//------------------------------------------------------------------------------------------------------------------------------------------
void MyQDialogs::ShowText(const QString & text, uint w, uint h)
{
	QDialog dialog_obj;
	QDialog *dialog = &dialog_obj;
	QHBoxLayout *hlo  = new QHBoxLayout(dialog);
	QTextBrowser *textBrowser = new QTextBrowser;
	textBrowser->setTabStopDistance(40);
	textBrowser->setPlainText(text);
	hlo->addWidget(textBrowser);

	if(!w) w = 150;
	if(!h) h = 150;
	dialog->resize(w, h);
	dialog->exec();
}

void MyQDialogs::ShowText(const QStringList &text, uint w, uint h)
{
	QDialog dialog_obj;
	QDialog *dialog = &dialog_obj;
	QHBoxLayout *hlo  = new QHBoxLayout(dialog);
	QTextBrowser *textBrowser = new QTextBrowser;
	textBrowser->setTabStopDistance(40);

	for(auto &row:text) textBrowser->append(row);
	QTimer::singleShot(0, textBrowser, [textBrowser]() { textBrowser->verticalScrollBar()->setValue(0); });

	hlo->addWidget(textBrowser);

	if(!w) w = 150;
	if(!h) h = 150;
	dialog->resize(w, h);
	dialog->exec();
}

QString MyQDialogs::CustomDialog(QString caption, QString text, QStringList buttons)
{
	QMessageBox messageBox(QMessageBox::Question, caption, text);
	for(auto &btn:buttons)
	{
		btn.prepend(' ').append(' ');
		messageBox.addButton(btn, QMessageBox::YesRole);  // Role не имеет значения
		// ' ' + btn + ' ' потому что setContentsMargins для вн.виджетов, а не текста, а setStyleSheet("padding: 6px;") имеет побочные эффекты
	}
	messageBox.exec();
	QString retText;
	if(auto button = messageBox.clickedButton(); button)
	{
		retText = button->text();
		retText.chop(1);
		retText.remove(0,1);
	}
	return retText;
}

QString MyQDialogs::CustomDialogWithCheckBox(QString caption, QString text, QStringList buttons, bool *chBoxRes, QString checkBoxText)
{
	QString retText;
	QDialog dialog;
	dialog.setWindowTitle(caption);
	dialog.setWindowFlag(Qt::WindowCloseButtonHint, false);

	QVBoxLayout *vloMain = new QVBoxLayout(&dialog);

	QLabel *label = new QLabel(text);
	vloMain->addWidget(label);

	QCheckBox *chBoxForAll = nullptr;
	if(chBoxRes)
	{
		chBoxForAll = new QCheckBox(checkBoxText);
		vloMain->addWidget(chBoxForAll);
	}

	QHBoxLayout *hloBtns = new QHBoxLayout;
	vloMain->addLayout(hloBtns);

	for(auto &btnText: buttons)
	{
		QPushButton *button = new QPushButton(QString(btnText).prepend(' ').append(' '));
		hloBtns->addWidget(button);
		QObject::connect(button, &QPushButton::clicked, &dialog, [&dialog, &retText, btnText, chBoxRes, chBoxForAll]() {
			retText = btnText;
			if(chBoxRes) *chBoxRes = chBoxForAll->isChecked();
			dialog.close();
		});
	}

	dialog.setLayout(vloMain);
	dialog.exec();

	return retText;
}

QString MyQDialogs::CustomDialogWithTimer(QString caption, QString text, QStringList buttons,
										  int defaultButtonIndex, uint secondsToDefaultPressed)
{
	QString retText;

	QDialog dialog;
	dialog.setWindowTitle(caption);
	dialog.setWindowFlag(Qt::WindowCloseButtonHint, false);

	QTimer timer;

	QVBoxLayout *vloMain = new QVBoxLayout(&dialog);

	QLabel *label = new QLabel(text);
	vloMain->addWidget(label);

	QHBoxLayout *hloBtns = new QHBoxLayout;
	vloMain->addLayout(hloBtns);

	QPushButton* defaultButton = nullptr;
	QString defaultButtonText;

	int index = 0;
	for(auto &btnText: buttons)
	{
		QPushButton *button = new QPushButton(QString(btnText).prepend(' ').append(' '));
		if(index == defaultButtonIndex)
		{
			defaultButton = button;
			defaultButtonText = btnText;
		}
		hloBtns->addWidget(button);
		QObject::connect(button, &QPushButton::clicked, &dialog, [&dialog, &retText, btnText]() {
			retText = btnText;
			dialog.close();
		});
		index++;
	}

	if(defaultButtonIndex == -1) {}
	else if(defaultButton)
	{
		auto SetDefButtonText = [&defaultButtonText, &secondsToDefaultPressed, &defaultButton](){
			QString newText = " "+defaultButtonText;
			newText.append(" (").append(QSn(secondsToDefaultPressed)).append(") ");
			defaultButton->setText(newText);
		};
		SetDefButtonText();

		QObject::connect(&timer, &QTimer::timeout, &dialog, [&timer, defaultButton, &secondsToDefaultPressed,
						 &SetDefButtonText]() {
			secondsToDefaultPressed--;
			SetDefButtonText();
			if(secondsToDefaultPressed == 0)
			{
				timer.stop();
				defaultButton->click();
			}
		});
		timer.start(1000);
	}
	else QMbError("CustomDialogWithTimer defaultButton wrong index");

	dialog.setLayout(vloMain);
	dialog.exec();

	return retText;
}

void MyQDialogs::MenuUnderWidget(QWidget *w, std::vector<MenuItem> &&items)
{
	std::vector<MenuItem> *itemsPtr = new std::vector<MenuItem>;
	std::vector<MenuItem> &itemsRef = *itemsPtr;
	itemsRef = std::move(items);
	// Без itemsPtr = new ... меню создано и показано, функция отработота - items уничтожены.
	// Пользователь нажмёт кнопку и будет попытка работы с уничтоженным объектом

	QMenu *menu = new QMenu(w);
	QObject::connect(menu, &QObject::destroyed, [itemsPtr](){ delete itemsPtr; });
	auto separator = SeparatorMenuItem();
	for(auto &item:itemsRef)
	{
		if(item.text == separator.text)
		{
			menu->addSeparator();
		}
		else if(item.text.contains(DisabledItemMarker()))
		{
			item.text.remove(DisabledItemMarker());
			QAction *action = new QAction(item.text, menu);
			action->setEnabled(false);
			menu->addAction(action);
		}
		else
		{
			if(!item.worker) { QMbError("nullptr worker in action " + item.text); continue; }

			QAction *action = new QAction(item.text, menu);
			MenuItem *itemPtr = &item;
			menu->addAction(action);
			QObject::connect(action, &QAction::triggered, [menu, itemsPtr, itemPtr](){
				if(itemPtr->worker) itemPtr->worker();
				else QMbError("nullptr worker executed");

				menu->hide();
				menu->deleteLater();
			});
		}
	}
	menu->exec(w->mapToGlobal(QPoint(0, w->height())));
}

void MyQDialogs::MenuUnderWidget(QWidget *w, QStringList texts, std::vector<std::function<void()>> workers)
{
	if(texts.size() != (int)workers.size()) { QMbError("MenuUnderWidget menuItems and workers different sizes"); return; }
	std::vector<MenuItem> items;
	for(int i=0; i<texts.size(); i++)
		items.emplace_back(    std::move(texts[i]), std::move(workers[i])    );

	MenuUnderWidget(w, std::move(items));
}

MyQDialogs::InputTextRes MyQDialogs::InputText(QString captionDialog, QString startText,  uint w, uint h)
{
	QDialog dialog;
	InputTextRes res;
	dialog.setWindowTitle(captionDialog);
	QVBoxLayout *vloAll  = new QVBoxLayout(&dialog);
	QTextEdit *textEdit = new QTextEdit;
	textEdit->setTabStopDistance(40);
	textEdit->setText(startText);
	vloAll->addWidget(textEdit);

	auto hloBtns = new QHBoxLayout;
	vloAll->addLayout(hloBtns);

	hloBtns->addStretch();
	hloBtns->addWidget(new QPushButton(Accept()));
	QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&res, &dialog, textEdit](){
		res.accepted=true;
		res.text = textEdit->toPlainText();
		dialog.close();
	});
	hloBtns->addWidget(new QPushButton(Cansel()));
	QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&dialog](){ dialog.close(); });

	dialog.resize(w, h);
	dialog.exec();

	return res;
}

MyQDialogs::InputTextRes MyQDialogs::InputLine(QString captionDialog, QString textDialog, QString startText, uint w)
{
	auto resExt = InputLineExt(captionDialog, textDialog, startText, {Accept(),Cansel()}, w);
	if(resExt.button == Accept()) return InputTextRes(std::move(resExt.text), true);
	else return InputTextRes("", false);
}

MyQDialogs::InputLineResExt MyQDialogs::InputLineExt(QString captionDialog, QString textDialog, QString startText,
													 QStringList buttons, uint w)
{
	QDialog dialog_obj;
	QDialog *dialog = &dialog_obj;
	InputLineResExt ret;
	ret.button = Undefined();
	dialog->setWindowTitle(captionDialog);
	QVBoxLayout *vloAll  = new QVBoxLayout(dialog);

	QLabel *label = new QLabel(textDialog);
	vloAll->addWidget(label);

	QLineEdit *lineEdit = new QLineEdit;
	lineEdit->setText(startText);
	vloAll->addWidget(lineEdit);

	auto hloBtns = new QHBoxLayout;
	vloAll->addLayout(hloBtns);

	hloBtns->addStretch();
	for(auto &btnText:buttons)
	{
		btnText.prepend(' ').append(' ');
		hloBtns->addWidget(new QPushButton(btnText));
		QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&dialog, lineEdit, btnText, &ret](){
			ret.button = btnText;
			ret.button.chop(1);
			ret.button.remove(0,1);
			ret.text = lineEdit->text();
			dialog->close();
		});
	}

	dialog->setFixedWidth(w);
	dialog->exec();

	return ret;
}

MyQDialogs::ListDialogRes MyQDialogs::ListDialog(QString caption, QStringList valuesList,
                                                 QString acceptButton, QString canselButton, uint w, uint h)
{
	ListDialogRes res(-1,"", false);
	QDialog dialog_obj;
	QDialog *dialog = &dialog_obj;
	dialog->resize(w, h);
	dialog->setWindowTitle(caption);
	QVBoxLayout *vloMain  = new QVBoxLayout(dialog);
	QListWidget *listWidget = new QListWidget;
	listWidget->addItems(valuesList);
	vloMain->addWidget(listWidget);

	auto acceptAction = [&dialog, listWidget, &res]()
	{
		if(auto item = listWidget->currentItem())
		{
			res.chosenIndex = listWidget->currentRow();
			res.chosenText = item->text();
			res.accepted = true;
			dialog->close();
		}
		else QMbError("Choose value or press cansel or close");
	};
	QObject::connect(listWidget, &QListWidget::itemDoubleClicked, acceptAction);

	acceptButton.prepend(' ').append(' ');
	canselButton.prepend(' ').append(' ');

	auto hloBtns = new QHBoxLayout();
	vloMain->addLayout(hloBtns);
	hloBtns->addStretch();
	hloBtns->addWidget(new QPushButton(acceptButton));
	QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, acceptAction);
	hloBtns->addWidget(new QPushButton(canselButton));
	QObject::connect(LastAddedWidget(hloBtns,QPushButton), &QPushButton::clicked, [&dialog]() { dialog->close(); });

	dialog->exec();
	return res;
}

MyQDialogs::ListDialogRes MyQDialogs::ListDialog(QString caption, QString valuesList, QString splitter,
                                                 QString acceptButton, QString canselButton, uint w, uint h)
{
	if(valuesList.endsWith(splitter)) valuesList.chop(splitter.size());
	return ListDialog(std::move(caption), valuesList.split(splitter), std::move(acceptButton), std::move(canselButton), w, h);
}

MyQDialogs::CheckBoxDialogResult MyQDialogs::CheckBoxDialog(const QString &caption, std::vector<CheckBoxDialogItem> items, uint w, uint h)
{
	QStringList values;
	std::vector<bool> startCheched;
	std::vector<bool> enabled;
	for(auto &item : items)
	{
		values += "";
		values.back() = std::move(item.text);
		startCheched.emplace_back(item.checkState);
		enabled.emplace_back(item.enabled);
	}
	return CheckBoxDialog(caption, values, startCheched, enabled, false, w, h);
}

MyQDialogs::CheckBoxDialogResult MyQDialogs::CheckBoxDialog(const QString &caption,
                                                            const QStringList & values,
                                                            const std::vector<bool> & startChecked,
															const std::vector<bool> & enabled,
                                                            bool startAllChecked,
															uint w, uint h)
{
	CheckBoxDialogResult result;

	QDialog dialog_obj;
	QDialog *dialog = &dialog_obj;
	dialog->resize(w, h);
	dialog->setWindowTitle(caption);
	auto vloMain = new QVBoxLayout(dialog);
	auto hloHeader = new QHBoxLayout;
	auto listWidget = new QListWidget;
	auto chBoxAllCheck = new QCheckBox;
	auto chBoxAllUncheck = new QCheckBox;

	auto hloBottom = new QHBoxLayout;
	auto btnOk = new QPushButton(Accept());
	auto btnCansel = new QPushButton(Cansel());
	QObject::connect(btnOk,&QPushButton::clicked,[&dialog, &result](){ result.accepted = true; dialog->hide(); });
	QObject::connect(btnCansel,&QPushButton::clicked,[&dialog, &result](){ result.accepted = false; dialog->hide();});

	vloMain->addLayout(hloHeader);
	hloHeader->addWidget(chBoxAllCheck);
	hloHeader->addWidget(chBoxAllUncheck);
	hloHeader->addStretch();
	vloMain->addWidget(listWidget);
	vloMain->addLayout(hloBottom);
	hloBottom->addStretch();
	hloBottom->addWidget(btnOk);
	hloBottom->addWidget(btnCansel);

	// all check и all uncheck
	chBoxAllCheck->setChecked(true);
	QObject::connect(chBoxAllCheck, &QCheckBox::clicked,[listWidget, chBoxAllCheck](){
		for(int i=0; i<listWidget->count(); i++)
		{
			if(listWidget->item(i)->flags().testFlag(Qt::ItemIsEnabled))
				listWidget->item(i)->setCheckState(Qt::Checked);
		}
		chBoxAllCheck->setChecked(true);
	});

	chBoxAllUncheck->setChecked(false);
	QObject::connect(chBoxAllUncheck, &QCheckBox::clicked,[listWidget,chBoxAllUncheck](){
		for(int i=0; i<listWidget->count(); i++)
		{
			if(listWidget->item(i)->flags().testFlag(Qt::ItemIsEnabled))
				listWidget->item(i)->setCheckState(Qt::Unchecked);
		}
		chBoxAllUncheck->setChecked(false);
	});

	// двойной клик на строку
	QObject::connect(listWidget, &QListWidget::itemDoubleClicked, [](QListWidgetItem *item){
		if(item->checkState() == Qt::Checked) item->setCheckState(Qt::Unchecked);
		else item->setCheckState(Qt::Checked);
	});

	// механика работы зажатого Shift
	int prevRow = -1;
	bool disableSignalItemChanged = false;
	QObject::connect(listWidget, &QListWidget::itemChanged, listWidget,
					 [listWidget, &prevRow, &disableSignalItemChanged](QListWidgetItem *item){
		if(disableSignalItemChanged) return;

		if (QApplication::keyboardModifiers() & Qt::ShiftModifier
				and item->checkState() == Qt::Checked
				and prevRow != -1)
		{
			int startRow, endRow;
			int currentRow = listWidget->row(item);
			if(currentRow == prevRow) { prevRow = -1; return; }
			if(currentRow > prevRow) { startRow = prevRow; endRow = currentRow; }
			else { startRow = currentRow; endRow = prevRow; }
			prevRow = -1;

			disableSignalItemChanged = true;
			for(int row=startRow; row<=endRow; row++)
			{
				listWidget->item(row)->setCheckState(Qt::Checked);
			}
			disableSignalItemChanged = false;
			return;
		}

		if(item->checkState() == Qt::Checked) prevRow = listWidget->row(item);
		else prevRow = -1;
	});

	// добавление и настройка строк
	uint i=0;
	for(auto str:values)
	{
		listWidget->addItem(str);

		listWidget->item(i)->setCheckState(Qt::Unchecked);
		if(startAllChecked) listWidget->item(i)->setCheckState(Qt::Checked);
		if(startChecked.size() > i and startChecked[i]) listWidget->item(i)->setCheckState(Qt::Checked);

		if(enabled.size() > i and enabled[i] == false) listWidget->item(i)->setFlags(listWidget->item(i)->flags() ^ Qt::ItemIsEnabled);

		i++;
	}

	// запуск диалога
	dialog->exec();

	// подготовка результата
	if(result.accepted)
	{
		for(int i=0; i<listWidget->count(); i++)
		{
			result.allItems.emplace_back(listWidget->item(i)->text(),
			                             listWidget->item(i)->checkState() == Qt::Checked,
			                             listWidget->item(i)->flags().testFlag(Qt::ItemIsEnabled));
			if(result.allItems.back().checkState)
			{
				result.checkedItems.emplace_back(result.allItems.back());
				result.checkedIndexes.emplace_back(i);
				result.checkedTexts += result.allItems.back().text;
			}
		}
	}

	return result;
}

MyQDialogs::TableDialogRes MyQDialogs::Table(const QString &caption, const std::vector<QStringList> &rows,
											 QStringList horisontalHeader, QStringList verticalHeader,
											 bool autoColWidths, bool readOnly, uint w, uint h)
{
	QDialog dialog_obj;
	QDialog *dialog = &dialog_obj;
	dialog->setWindowTitle(caption);
	if(0) CodeMarkers::to_do("кнопки вверх, вниз, отменить изменения, автоподгонка ширины колонок");
	if(!w) w = 640;
	if(!h) h = 480;
	dialog->resize(w, h);

	TableDialogRes res;

	QVBoxLayout *vlo_main  = new QVBoxLayout(dialog);

	QTableWidget *table = new QTableWidget;
	res.table = std::unique_ptr<QTableWidget>(table);

	if(!readOnly)
	{
		QHBoxLayout *hlo1 = new QHBoxLayout;
		vlo_main->addLayout(hlo1);

		auto btnAdd = new QPushButton("+");
		MyQWidget::SetFontBold(btnAdd, true);
		btnAdd->setFixedWidth(24);
		hlo1->addWidget(btnAdd);
		btnAdd->connect(btnAdd, &QPushButton::clicked, [table](){
			if(table->columnCount() == 0) table->setColumnCount(1);

			if(table->currentRow() == -1) table->insertRow(0);
			else table->insertRow(table->currentRow());
		});

		auto btnRemove = new QPushButton("-");
		MyQWidget::SetFontBold(btnRemove, true);
		btnRemove->setFixedWidth(24);
		hlo1->addWidget(btnRemove);
		btnRemove->connect(btnRemove, &QPushButton::clicked, [table](){
			if(table->currentRow() != -1) table->removeRow(table->currentRow());
		});

		auto btnUp = new QPushButton("🡅");
		btnUp->setFixedWidth(24);
		hlo1->addWidget(btnUp);
		btnUp->connect(btnUp, &QPushButton::clicked, [table](){
			int currentRow = table->currentRow();
			if(currentRow >= 1)
			{
				MyQTableWidget::SwapRows(table, currentRow, currentRow-1);
				table->setCurrentCell(currentRow-1, table->currentColumn());
			}
		});

		auto btnDown = new QPushButton("🡇");
		btnDown->setFixedWidth(24);
		hlo1->addWidget(btnDown);
		btnDown->connect(btnDown, &QPushButton::clicked, [table](){
			int currentRow = table->currentRow();
			if(currentRow < table->rowCount()-1)
			{
				MyQTableWidget::SwapRows(table, currentRow, currentRow+1);
				table->setCurrentCell(currentRow+1, table->currentColumn());
			}
		});

		hlo1->addStretch();
	}

	vlo_main->addWidget(table);

	QHBoxLayout *hlo2 = new QHBoxLayout;
	vlo_main->addLayout(hlo2);

	hlo2->addStretch();

	if(!readOnly)
	{
		auto btnAccept = new QPushButton(Accept());
		btnAccept->setDefault(true);
		hlo2->addWidget(btnAccept);
		btnAccept->connect(btnAccept, &QPushButton::clicked, [&dialog, &res](){ res.accepted = true; dialog->close(); });

		auto btnCansel = new QPushButton(Cansel());
		hlo2->addWidget(btnCansel);
		btnCansel->connect(btnCansel, &QPushButton::clicked, [&dialog](){ dialog->close(); });
	}
	else
	{
		auto btnAccept = new QPushButton(Close());
		btnAccept->setDefault(true);
		hlo2->addWidget(btnAccept);
		btnAccept->connect(btnAccept, &QPushButton::clicked, [&dialog](){ dialog->close(); });
	}

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
			if(table->columnCount() == 0)
				return;
			int oneColWidth = (table->width() - 45) / table->columnCount();
			for(int col=0; col<table->columnCount(); col++)
			{
				table->setColumnWidth(col, oneColWidth);
			}
		});
	}

	if(horisontalHeader.isEmpty()) table->horizontalHeader()->hide();
	else
	{
		if(table->columnCount() < horisontalHeader.size())
			table->setColumnCount(horisontalHeader.size());
		table->setHorizontalHeaderLabels(horisontalHeader);
	}
	if(verticalHeader.isEmpty()) table->verticalHeader()->hide();
	else table->setVerticalHeaderLabels(verticalHeader);

	dialog->exec();

	table->setParent(nullptr);

	return res;
}

MyQDialogs::TableDialogRes MyQDialogs::Table(const QString &caption, QString content, QString colSplitter, QString rowSplitter,
											 QStringList horisontalHeader, QStringList verticalHeader,
											 bool autoColWidths, bool readOnly, uint w, uint h)
{
	if(content.endsWith(colSplitter+rowSplitter)) content.chop(colSplitter.size()+rowSplitter.size());
	std::vector<QStringList> rowsTmp;
	QStringList splitByRows = content.split(rowSplitter);
	for(auto &row:splitByRows)
	{
		if(row.endsWith(colSplitter)) row.chop(colSplitter.size());
		rowsTmp.emplace_back(row.split(colSplitter));
	}
	return Table(caption, rowsTmp, horisontalHeader, verticalHeader, autoColWidths, readOnly, w, h);
}

MyQDialogs::TableDialogRes MyQDialogs::TableOneCol(const QString &caption, QStringList rows,
												   QStringList horisontalHeader, QStringList verticalHeader,
												   bool autoColWidths, bool readOnly, uint w, uint h)
{
	std::vector<QStringList> rowsTmp;
	for(auto &row:rows)
		rowsTmp.emplace_back(std::move(row));
	return Table(caption, rowsTmp, horisontalHeader, verticalHeader, autoColWidths, readOnly, w, h);
}

void MyQDialogs::InfoBar(const QString &message, QWidget *widgetToShowIn)
{
	auto widget = new QWidget(widgetToShowIn);
	widget->setAttribute(Qt::WA_DeleteOnClose);
	widget->setStyleSheet("background-color: #e0f7fa; border: 1px solid #00acc1; padding: 5px;");

	QHBoxLayout *hloMain = new QHBoxLayout(widget);
	hloMain->setContentsMargins(10, 1, 10, 1);
	hloMain->setSpacing(10);

	QVBoxLayout *vloClose = new QVBoxLayout;
	vloClose->setContentsMargins(0,10,0,0);

	QLabel *label = new QLabel(message, widgetToShowIn);
	label->setWordWrap(true);
	label->setStyleSheet("border: none;");

	QPushButton *closeButton = new QPushButton("×");
	closeButton->setFixedSize(20, 20);
	closeButton->setStyleSheet("QPushButton { background-color: #00acc1; color: white; border-radius: 10px; }"
							   "QPushButton:hover { background-color: #00838f; }");
	QObject::connect(closeButton, &QPushButton::clicked, widget, [widget](){ widget->close(); widget->deleteLater(); });

	hloMain->addWidget(label);
	hloMain->addStretch();
	hloMain->addLayout(vloClose);
	vloClose->addWidget(closeButton, 0, Qt::AlignTop);

	auto widgetSA = dynamic_cast<QAbstractScrollArea*>(widgetToShowIn);
	if(widgetSA) hloMain->addSpacing(widgetSA->verticalScrollBar()->width());

	// хранилище
	static std::map<QWidget *, std::vector<QWidget*>> barsInWidgets;

	// расчет координаты y исходя из последнего бара в этом виджете
	int y = 0;
	auto findRes = barsInWidgets.find(widgetToShowIn);
	if(findRes != barsInWidgets.end())
		y = findRes->second.back()->y()+findRes->second.back()->height();

	// вставка этого бара в хранилище и обработчик его удаления
	barsInWidgets[widgetToShowIn].push_back(widget);
	QObject::connect(widget, &QObject::destroyed, widget, [widgetToShowIn, widget](){
		auto &barsVect = barsInWidgets[widgetToShowIn];
		for(uint i=0; i<barsVect.size(); i++)
			if(barsVect[i] == widget)
				barsVect.erase(barsVect.begin()+i);
		if(barsVect.empty()) barsInWidgets.erase(widgetToShowIn);
		else
		{
			// сдвиг оставшихся вверх
			int y = 0;
			for(auto &bar:barsVect)
			{
				bar->move(0,y);
				y += bar->height();
			}
		}
	});

	widget->show(); // обязательно до resize иначе показывается криво
	widget->move(0, y);
	widget->resize(widgetToShowIn->width(), widget->height());
}

void MyQDialogs::ShowAllStandartIcons()
{
	QWidget *w = new QWidget;
	MyQWidget::SetFontPointSize(w, 10);
	w->setAttribute(Qt::WA_DeleteOnClose);
	auto glo = new QGridLayout(w);
	static std::vector<QPixmap*> pixmaps;
	pixmaps.clear();

	if(QString(QT_VERSION_STR) == "5.12.10") ; // все норм, enum проверен
	else { QMbWarning("enum QStyle::SP_TitleBarMenuButton not checked for Qt " QT_VERSION_STR); }

	int start = QStyle::SP_TitleBarMenuButton;
	int max = QStyle::SP_LineEditClearButton;
	for(int i=start, row = 0, col = 0; i<=max; i++)
	{
		pixmaps.emplace_back(new QPixmap);
		QPixmap& pixmapRef = *pixmaps.back();

		auto icon = QApplication::style()->standardIcon((QStyle::StandardPixmap)i);
		if(!icon.isNull()) pixmapRef = icon.pixmap(50,50);

		auto labelImage = new QLabel;
		labelImage->setAlignment(Qt::AlignCenter);
		if(!icon.isNull()) labelImage->setPixmap(pixmapRef);
		else { labelImage->setText("null"); }

		QString caption = MyQString::AsDebug((QStyle::StandardPixmap)i).remove("QStyle::");
		if(caption.size() > 20) caption = caption.left(18).append("...");
		auto labelCaption = new QLabel(caption);

		glo->addWidget(labelImage, row, col);
		glo->addWidget(labelCaption, row, col+1);

		row++;
		if(row == 10) { row=0; col+=2; }
	}
	w->move(200,200);
	w->show();
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif // MYQDIALOGS_H
//------------------------------------------------------------------------------------------------------------------------------------------
