//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MYQTABLEWIDGET_H
#define MYQTABLEWIDGET_H
//------------------------------------------------------------------------------------------------------------------------------------------
#include <memory>
#include <set>

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
	/// SwapRows doesn't support cellWidgets because of strange QTableWidget work with them.
	/// You can't just take or move cellWidget correct. Even with crutches: try create buf row or even buf QTableWidget.
	inline static bool SwapRows(QTableWidget *table, int row1, int row2);
	inline static void SetItemEditableState(QTableWidgetItem *item, bool editableNewState);
	inline static void FitColsWidths(QTableWidget *table); // не проверено!!!

	inline static std::set<int> RowsInViewPort(QTableWidget* table);

	/// Row is selected if at least one cell in that row is selected.
	inline static std::set<int> SelectedRows(QTableWidget *table, bool onlyVisibleRows);
	inline static void SelectRows(QTableWidget *table, int from, int to, bool scrollToFromRow);

	inline static int FindNearestVisibleRow(QTableWidget *table, int row);

public:
	inline explicit MyQTableWidget(QWidget *parent = nullptr);
	inline void CreateContextMenu();
	virtual ~MyQTableWidget() {}

	///\brief для отключения стандартного поиска в QTableView при нажатии букв
	/// обработка происходит в keyboardSearch
	bool disableKeyboardSearch = false;

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

	struct PastePolicy {
		bool pasteNullItemDeletesDstItem = false; // false - item text will be just cleared
		bool allowCreateRows = true;
		bool allowCreateCols = true;
		bool allowCreateItems = true;
		bool showMessageIfTryedNotAllowed = true;
	};
	PastePolicy pastePolicy;
private:
	inline void Copy(bool cut);
	inline void Paste();
	inline void ReplaceForCopyPaste(QString &text, bool copy);
	inline static const char *nullItemMarker = "null_item";
	const std::vector<std::pair<QString, QString>> replaces {
		{"\"", "_mqtw_q_"},
		{"\t", "_mqtw_t_"},
		{"\r", "_mqtw_r_"},
		{"\n", "_mqtw_n_"},
	};

	QMenu* menu = nullptr;

protected:
	inline void keyboardSearch(const QString &search) override;
};

//------------------------------------------------------------------------------------------------------------------------------------------

bool MyQTableWidget::SwapRows(QTableWidget *table, int row1, int row2)
{
	if(!table || row1 < 0 || row1 >= table->rowCount() || row2 < 0 || row2 >= table->rowCount()) return false;
	for(int col=0; col<table->columnCount(); col++)
	{
		if(table->cellWidget(row1, col) || table->cellWidget(row2, col))
			qCritical() << "MyQTableWidget::SwapRows: there are widgets in cells, but cellWidgetsSupport disabled";

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

std::set<int> MyQTableWidget::SelectedRows(QTableWidget *table, bool onlyVisibleRows)
{
	std::set<int> rows;
	auto ranges = table->selectedRanges();
	for(auto &range:ranges)
	{
		for(int row=range.topRow(); row<=range.bottomRow(); row++)
		{
			if(onlyVisibleRows && table->isRowHidden(row)) continue;
			rows.insert(row);
		}
	}
	return rows;
}

std::set<int> MyQTableWidget::RowsInViewPort(QTableWidget *table)
{
	int visibleTop = table->viewport()->geometry().y();
	int visibleBottom = visibleTop + table->viewport()->height();

	int firstVisibleRow = table->rowAt(visibleTop);
	int lastVisibleRow = table->rowAt(visibleBottom);

	if (firstVisibleRow == -1) return {}; // нет видимых строк вообще
	if (lastVisibleRow >= table->rowCount())
	{
		qCritical() << "MyQTableWidget::VisibleRows error: invalid lastVisibleRow "+QSn(lastVisibleRow);
		return {};
	}
	if (lastVisibleRow == -1) // внизу таблицы нет видимой строки, значит строки не доходят до низа
	{
		lastVisibleRow = table->rowCount()-1; // проверим все сколько есть
	}

	std::set<int> setOfVisibleRows;
	for (int row = firstVisibleRow; row <= lastVisibleRow; ++row)
	{
		if(table->isRowHidden(row) == false)
			setOfVisibleRows.insert(row);
	}

	return setOfVisibleRows;
}

void MyQTableWidget::SelectRows(QTableWidget *table, int from, int to, bool scrollToFromRow)
{
	QItemSelectionModel *selectionModel = table->selectionModel();
	selectionModel->clearSelection();

	if(from < 0 or from >= table->rowCount()) { qdbg << "MyQTableWidget::SelectRows invalid 'from' value"<<from; return; }
	if(to < 0 or to >= table->rowCount()) { qdbg << "MyQTableWidget::SelectRows invalid 'to' value"<<to; return; }

	QModelIndex topLeft = table->model()->index(from, 0);
	QModelIndex bottomRight = table->model()->index(to, table->columnCount() - 1);

	QItemSelection selection(topLeft, bottomRight);

	selectionModel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);

	if(scrollToFromRow) table->scrollToItem(table->item(from, 0), QAbstractItemView::PositionAtCenter);
}

int MyQTableWidget::FindNearestVisibleRow(QTableWidget *table, int row)
{
	int rowCount = table->rowCount();
	if (rowCount == 0 or row < 0 or row >= rowCount) return -1;

	// Ограничиваем входную строку границами таблицы
	int offset = 0;

	// Цикл продолжается, пока мы не найдем видимую строку или не выйдем за границы всей таблицы
	while (offset < rowCount) {
		// Проверяем строку "выше"
		int upRow = row - offset;
		if (upRow >= 0 && !table->isRowHidden(upRow)) {
			return upRow;
		}

		// Проверяем строку "ниже"
		int downRow = row + offset;
		if (downRow < rowCount && !table->isRowHidden(downRow)) {
			return downRow;
		}

		offset++;
	}

	return -1; // Если все строки скрыты
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

	connect(cutAction, &QAction::triggered, this, [this](){ Copy(true); });
	connect(copyAction, &QAction::triggered, this, [this](){ Copy(false); });
	connect(pasteAction, &QAction::triggered, this, &MyQTableWidget::Paste);

	cutAction->setShortcut(QKeySequence::Cut);
	copyAction->setShortcut(QKeySequence::Copy);
	pasteAction->setShortcut(QKeySequence::Paste);

	cutAction->setShortcutContext(Qt::WidgetShortcut);
	copyAction->setShortcutContext(Qt::WidgetShortcut);
		// пришлось добавить чтобы не перехватывал Ctrl+C из ReadOnly QTextEdit-а
	pasteAction->setShortcutContext(Qt::WidgetShortcut);

	addAction(cutAction);
	addAction(copyAction);
	addAction(pasteAction);

	connect(this, &QWidget::customContextMenuRequested, [this](const QPoint &pos){ menu->exec(viewport()->mapToGlobal(pos)); });
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

void MyQTableWidget::Copy(bool cut)
{
	QItemSelectionModel *selection = selectionModel();
	if (!selection->hasSelection()) return;

	QString copiedData;
	QList<QTableWidgetSelectionRange> ranges = selectedRanges();
	if(ranges.size() > 1) { QMbInfo("More one selection, can't copy."); return; }

	if(cut) itemStatesBeforeCut.clear();

	auto &range = ranges[0];
	for (int row = range.topRow(); row <= range.bottomRow(); ++row)
	{
		for (int col = range.leftColumn(); col <= range.rightColumn(); ++col)
		{
			auto itemToCopy = item(row, col);
			if(itemToCopy)
			{
				QString itemText = itemToCopy->text();
				ReplaceForCopyPaste(itemText, true);
				copiedData += "\"" + itemText + "\"";
				if(cut)
				{
					itemStatesBeforeCut.emplace_back(row, col, itemToCopy->text());
					itemToCopy->setText("");
				}
			}
			else
			{
				copiedData += nullItemMarker;
			}
			if (col != range.rightColumn()) copiedData += '\t';
		}
		copiedData += '\n';
	}

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(copiedData);

	if(cut) emit SignalAfterCut();
}

void MyQTableWidget::Paste()
{
	const QClipboard *clipboard = QApplication::clipboard();
	if (!clipboard) return;

	QString pastedData = clipboard->text();
	if (pastedData.isEmpty()) return;

	itemStatesBeforePaste.clear();
	int startRow = currentRow();
	int startCol = currentColumn();

	if (startRow < 0  or startCol < 0) return;

	while (pastedData.endsWith('\n') or pastedData.endsWith('\r')) pastedData.chop(1);

	QStringList rows = pastedData.split('\n');

	QString message;

	for (int i = 0; i < rows.size(); ++i)
	{
		int currentRowIdx = startRow + i;

		if (currentRowIdx >= rowCount())
		{
			if(not pastePolicy.allowCreateRows) { message += "- creating new rows is disabled\n"; break; }
			setRowCount(currentRowIdx + 1);
		}

		QStringList cols = rows[i].split('\t');

		for (int j = 0; j < cols.size(); ++j)
		{
			int currentColIdx = startCol + j;

			if (currentColIdx >= columnCount()) {
				if(not pastePolicy.allowCreateCols) { message += "- creating new columns is disabled\n"; break; }
				setColumnCount(currentColIdx + 1);
			}

			QString cellText = cols[j];
			QTableWidgetItem *currentItem = item(currentRowIdx, currentColIdx);

			QString textBeforePaste = currentItem ? currentItem->text() : ItemState::ItemWasNullptr();
			itemStatesBeforePaste.emplace_back(currentRowIdx, currentColIdx, textBeforePaste);

			if(cellText == nullItemMarker)
			{
				if(currentItem) currentItem->setText("");
				if(pastePolicy.pasteNullItemDeletesDstItem) delete takeItem(currentRowIdx, currentColIdx);
			}
			else
			{
				if(cellText.startsWith('"') && cellText.endsWith('"'))
				{
					cellText.remove(0,1);
					cellText.chop(1);
				}

				ReplaceForCopyPaste(cellText, false);

				if (!currentItem)
				{
					if(not pastePolicy.allowCreateItems)
					{
						message += "- creating new items (row="+QSn(currentRowIdx)+", column="+QSn(currentColIdx)+") "
								   "is disabled\n";
						continue;
					}
					currentItem = new QTableWidgetItem();
					setItem(currentRowIdx, currentColIdx, currentItem);
				}

				currentItem->setText(cellText);
			}
		}
	}

	emit SignalAfterPaste();

	if(pastePolicy.showMessageIfTryedNotAllowed and !message.isEmpty())
		QMbInfo("Some data could not be pasted due to table settings:\n\n"+message);
}

void MyQTableWidget::ReplaceForCopyPaste(QString &text, bool copy)
{
	if(copy)
	{
		for(auto &[from, to]:replaces) text.replace(from, to);
	}
	else
	{
		for(auto &[to, from]:replaces) text.replace(from, to);
	}
}

void MyQTableWidget::keyboardSearch(const QString &search)
{
	if(disableKeyboardSearch) return;
	QAbstractItemView::keyboardSearch(search);
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif // MYQTABLEWIDGET_H
//------------------------------------------------------------------------------------------------------------------------------------------
