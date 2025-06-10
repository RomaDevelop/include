//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MYQTABLEWIDGET_H
#define MYQTABLEWIDGET_H
//------------------------------------------------------------------------------------------------------------------------------------------
#include <memory>

#include <QApplication>
#include <QTableWidget>
#include <QScrollBar>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

#include "MyQShortings.h"

//------------------------------------------------------------------------------------------------------------------------------------------

class MyQTableWidget : public QTableWidget
{
	Q_OBJECT
public:
	inline static bool SwapRows(QTableWidget *table, int row1, int row2);
	inline static void SetItemEditableState(QTableWidgetItem *item, bool editableNewState);
	inline static void FitColsWidths(QTableWidget *table); // не проверено!!!

public:
	inline explicit MyQTableWidget(QWidget *parent = nullptr);
	inline void CreateContextMenu();
	virtual ~MyQTableWidget() {}

public: signals:
	void SignalAfterCut();
	void SignalAfterPaste();
public:
	struct ItemState
	{
		int itemRow;
		int itemCol;
		QString text;

		ItemState(int itemRow, int itemCol, QString text):
			itemRow{itemRow}, itemCol{itemCol}, text{std::move(text)} {}
		static const QString& ItemWasNullptr() { static QString str = "ItemState::ItemWasNullptr()"; return str; }
	};
	std::vector<ItemState> itemStatesBeforePaste;
	std::vector<ItemState> itemStatesBeforeCut;
	inline QString RestoreState(const std::vector<ItemState> &states);
private slots:
	inline void Cut();
	inline void Copy();
	inline void Paste();
	inline void PasteFromOsClip();
private:
	QMenu* menu = nullptr;
	inline static std::vector<QStringList> innerClipboard {};
};

//------------------------------------------------------------------------------------------------------------------------------------------

bool MyQTableWidget::SwapRows(QTableWidget *table, int row1, int row2)
{
	if(!table || row1 < 0 || row1 >= table->rowCount() || row2 < 0 || row2 >= table->rowCount()) return false;
	for(int col=0; col<table->columnCount(); col++)
	{
		QTableWidgetItem *itemBuf = table->takeItem(row2, col);
		table->setItem(row2, col, table->takeItem(row1, col));
		table->setItem(row1, col, itemBuf);
	}
	return true;
}

void MyQTableWidget::SetItemEditableState(QTableWidgetItem * item, bool editableNewState)
{
	if(!item) { qCritical() << "MyQTableWidget::SetItemEditableState nullptr item"; return; }
	auto flags = item->flags();
	flags.setFlag(Qt::ItemIsEditable, editableNewState);
	item->setFlags(flags);
}

void MyQTableWidget::FitColsWidths(QTableWidget *table)
{
	if(table->columnCount() == 0) return;

	int tableSpace = table->width();
	if(table->verticalScrollBar()->isVisible()) tableSpace -= table->verticalScrollBar()->width();
	tableSpace -= 3;

	if(table->width() < 1) return;

	int currentColsWidth = 0;
	for(int i=0; i<table->columnCount(); i++) currentColsWidth += table->columnWidth(i);

	if (currentColsWidth == 0) return;

	for(int i=0; i<table->columnCount(); i++)
	{
		float persent = (float)table->columnWidth(i) / (float)currentColsWidth;
		table->setColumnWidth(i, tableSpace*persent);
	}
}

MyQTableWidget::MyQTableWidget(QWidget *parent) : QTableWidget(parent)
{
	setSelectionMode(QAbstractItemView::ContiguousSelection);

	CreateContextMenu();
}

void MyQTableWidget::CreateContextMenu()
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	menu = new QMenu(this);
	QAction *cutAction = menu->addAction("Cut");
	QAction *copyAction = menu->addAction("Copy");
	QAction *pasteAction = menu->addAction("Paste");
	QAction *pasteActionFromOsClip = menu->addAction("Paste from OS clipboard");

	connect(cutAction, &QAction::triggered, this, &MyQTableWidget::Cut);
	connect(copyAction, &QAction::triggered, this, &MyQTableWidget::Copy);
	connect(pasteAction, &QAction::triggered, this, &MyQTableWidget::Paste);
	connect(pasteActionFromOsClip, &QAction::triggered, this, &MyQTableWidget::PasteFromOsClip);

	cutAction->setShortcut(QKeySequence::Cut);
	copyAction->setShortcut(QKeySequence::Copy);
	pasteAction->setShortcut(QKeySequence::Paste);

	cutAction->setShortcutContext(Qt::WidgetShortcut);
	copyAction->setShortcutContext(Qt::WidgetShortcut);		// пришлось добавить чтобы не перехватывал Ctrl+C из ReadOnly QTextEdit-а
	pasteAction->setShortcutContext(Qt::WidgetShortcut);

	addAction(cutAction);
	addAction(copyAction);
	addAction(pasteAction);

	connect(this, &QWidget::customContextMenuRequested, [this](const QPoint &pos){ menu->exec(mapToGlobal(pos)); });
}

QString MyQTableWidget::RestoreState(const std::vector<MyQTableWidget::ItemState> &states)
{
	QString errors;
	for(auto &state:states)
	{
		auto changingItem = item(state.itemRow, state.itemCol);
		if(changingItem)
		{
			if(state.text == ItemState::ItemWasNullptr())
				setItem(state.itemRow, state.itemCol, nullptr);
			else changingItem->setText(state.text);
		}
		else errors += "restoring item ("+QSn(state.itemRow)+","+QSn(state.itemCol)+") is nullptr\n";
	}
	return errors;
}

void MyQTableWidget::Cut()
{
	Copy();
	itemStatesBeforeCut.clear();
	QList<QTableWidgetSelectionRange> ranges = selectedRanges();
	for (const auto &range : ranges) {
		for (int row = range.topRow(); row <= range.bottomRow(); ++row) {
			for (int col = range.leftColumn(); col <= range.rightColumn(); ++col) {

				auto itemToChange = item(row, col);
				if (itemToChange) {
					itemStatesBeforeCut.emplace_back(row, col, itemToChange->text());
					itemToChange->setText("");
				}
			}
		}
	}
	emit SignalAfterCut();
}

void MyQTableWidget::Copy()
{
	QItemSelectionModel *selection = selectionModel();
	if (!selection->hasSelection()) return;
	innerClipboard.clear();
	innerClipboard.emplace_back();

	QString copiedData;
	QList<QTableWidgetSelectionRange> ranges = selectedRanges();
	for (const auto &range : ranges)
	{
		for (int row = range.topRow(); row <= range.bottomRow(); ++row)
		{
			for (int col = range.leftColumn(); col <= range.rightColumn(); ++col)
			{
				if (item(row, col))
				{
					innerClipboard.back().push_back(item(row, col)->text());
					copiedData += "\"" + item(row, col)->text() + "\"";
				}
				if (col != range.rightColumn()) copiedData += '\t';
			}
			innerClipboard.emplace_back();
			copiedData += '\n';
		}
	}
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(copiedData);
}

void MyQTableWidget::Paste()
{
	itemStatesBeforePaste.clear();
	int startRow = currentRow();
	int startCol = currentColumn();
	for (uint i = 0; i < innerClipboard.size(); ++i)
	{
		QStringList &columns = innerClipboard[i];
		for (int j = 0; j < columns.size(); ++j)
		{
			int row = startRow + i;
			int col = startCol + j;
			// Автоматическое добавление строк и столбцов
			if (row >= rowCount()) insertRow(rowCount());
			if (col >= columnCount()) insertColumn(columnCount());

			auto itemToChange = item(row, col);
			if (!itemToChange)
			{
				itemToChange = new QTableWidgetItem();
				setItem(row, col, itemToChange);
				itemStatesBeforePaste.emplace_back(row, col, ItemState::ItemWasNullptr());
			}
			else itemStatesBeforePaste.emplace_back(row, col, itemToChange->text());
			itemToChange->setText(columns[j]);
		}
	}
	emit SignalAfterPaste();
}

void MyQTableWidget::PasteFromOsClip()
{
	itemStatesBeforePaste.clear();
	QClipboard *clipboard = QApplication::clipboard();
	QString clipboardData = clipboard->text();
	QStringList rows = clipboardData.split('\n', QString::SkipEmptyParts);
	int startRow = currentRow();
	int startCol = currentColumn();
	for (int i = 0; i < rows.size(); ++i)
	{
		QStringList columns = rows[i].split('\t');
		for (int j = 0; j < columns.size(); ++j)
		{
			int row = startRow + i;
			int col = startCol + j;
			// Автоматическое добавление строк и столбцов
			if (row >= rowCount()) insertRow(rowCount());
			if (col >= columnCount()) insertColumn(columnCount());


			auto itemToChange = item(row, col);
			if (!itemToChange)
			{
				itemToChange = new QTableWidgetItem();
				setItem(row, col, itemToChange);
				itemStatesBeforePaste.emplace_back(row, col, ItemState::ItemWasNullptr());
			}
			else itemStatesBeforePaste.emplace_back(row, col, itemToChange->text());
			itemToChange->setText(columns[j]);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif // MYQTABLEWIDGET_H
//------------------------------------------------------------------------------------------------------------------------------------------
