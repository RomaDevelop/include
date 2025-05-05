//--------------------------------------------------------------------------------------------------------------------------------
#ifndef MYQTABLEWIDGET_H
#define MYQTABLEWIDGET_H
//--------------------------------------------------------------------------------------------------------------------------------
#include <memory>

#include <QApplication>
#include <QTableWidget>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

#include "MyQShortings.h"

//--------------------------------------------------------------------------------------------------------------------------------

class MyQTableWidget : public QTableWidget
{
	Q_OBJECT
public:
	inline static bool SwapRows(QTableWidget *table, int row1, int row2);
	inline static void SetItemEditableState(QTableWidgetItem *item, bool editableNewState);

public:
	MyQTableWidget(QWidget *parent = nullptr) : QTableWidget(parent)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, &QWidget::customContextMenuRequested, this, &MyQTableWidget::showContextMenu);

		setSelectionMode(QAbstractItemView::ContiguousSelection);
	}

protected:
	void keyPressEvent(QKeyEvent *event) override
	{
		if (event->matches(QKeySequence::Cut)) cut();
		else if (event->matches(QKeySequence::Copy)) copy();
		else if (event->matches(QKeySequence::Paste)) paste();
		QTableWidget::keyPressEvent(event);
	}
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
	QString RestoreState(const std::vector<ItemState> &states)
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
private slots:
	void showContextMenu(const QPoint &pos)
	{
		static QMenu* mPtr = nullptr;
		if(!mPtr)
		{
			mPtr = new QMenu(this);
			QMenu &menu = *mPtr;
			QAction *cutAction = menu.addAction("Cut");
			QAction *copyAction = menu.addAction("Copy");
			QAction *pasteAction = menu.addAction("Paste");
			QAction *pasteActionFromOsClip = menu.addAction("Paste from OS clipboard");
			cutAction->setShortcut(QKeySequence::Cut);
			copyAction->setShortcut(QKeySequence::Copy);
			pasteAction->setShortcut(QKeySequence::Paste);
			connect(cutAction, &QAction::triggered, this, &MyQTableWidget::cut);
			connect(copyAction, &QAction::triggered, this, &MyQTableWidget::copy);
			connect(pasteAction, &QAction::triggered, this, &MyQTableWidget::paste);
			connect(pasteActionFromOsClip, &QAction::triggered, this, &MyQTableWidget::pasteFromOsClip);
		}
		mPtr->exec(mapToGlobal(pos));
	}
	void cut()
	{
		copy();
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
	void copy()
	{
		QItemSelectionModel *selection = selectionModel();
		if (!selection->hasSelection()) return;
		innerClipboard.clear();
		innerClipboard.emplace_back();
		innerClipboardVerticalHeaderValues.clear();
		innerClipboardAllColums.clear();

		QString copiedData;
		QList<QTableWidgetSelectionRange> ranges = selectedRanges();
		for (const auto &range : ranges) {
			for (int row = range.topRow(); row <= range.bottomRow(); ++row) {

				auto vHeaderItem = this->verticalHeaderItem(row);
				if(vHeaderItem) innerClipboardVerticalHeaderValues.emplace_back(vHeaderItem->text());
				else innerClipboardVerticalHeaderValues.emplace_back();

				innerClipboardAllColums.emplace_back();
				for (int col = 0; col <= columnCount(); ++col)
					if (item(row, col)) innerClipboardAllColums.back().push_back(item(row, col)->text());
					// else ... not need, innerClipboardAllColums.back() is empty by default

				for (int col = range.leftColumn(); col <= range.rightColumn(); ++col) {
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
	void paste()
	{
		itemStatesBeforePaste.clear();
		int startRow = currentRow();
		int startCol = currentColumn();
		for (uint i = 0; i < innerClipboard.size(); ++i)
		{
			QStringList &columns = innerClipboard[i];
			for (int j = 0; j < columns.size(); ++j) {
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
	void pasteFromOsClip()
	{
		itemStatesBeforePaste.clear();
		QClipboard *clipboard = QApplication::clipboard();
		QString clipboardData = clipboard->text();
		QStringList rows = clipboardData.split('\n', QString::SkipEmptyParts);
		int startRow = currentRow();
		int startCol = currentColumn();
		for (int i = 0; i < rows.size(); ++i) {
			QStringList columns = rows[i].split('\t');
			for (int j = 0; j < columns.size(); ++j) {
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
private:
	inline static std::vector<QStringList> innerClipboard {};
	inline static std::vector<QString> innerClipboardVerticalHeaderValues {};
	inline static std::vector<QStringList> innerClipboardAllColums {};
};

//--------------------------------------------------------------------------------------------------------------------------------

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

#endif // MYQTABLEWIDGET_H
