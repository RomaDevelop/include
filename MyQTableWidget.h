#ifndef MYQTABLEWIDGET_H
#define MYQTABLEWIDGET_H

#include <QApplication>
#include <QTableWidget>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

class MyQTableWidget : public QTableWidget
{
	Q_OBJECT
public:
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
			cutAction->setShortcut(QKeySequence::Cut);
			copyAction->setShortcut(QKeySequence::Copy);
			pasteAction->setShortcut(QKeySequence::Paste);
			connect(cutAction, &QAction::triggered, this, &MyQTableWidget::cut);
			connect(copyAction, &QAction::triggered, this, &MyQTableWidget::copy);
			connect(pasteAction, &QAction::triggered, this, &MyQTableWidget::paste);
		}
		mPtr->exec(mapToGlobal(pos));
	}
	void cut()
	{
		copy();  // Копируем сначала данные
		clearSelectionContent();  // Затем очищаем выделенные ячейки
	}
	void copy()
	{
		QItemSelectionModel *selection = selectionModel();
		if (!selection->hasSelection()) return;
		innerClipboard.clear();
		innerClipboard.emplace_back();
		QString copiedData;
		QList<QTableWidgetSelectionRange> ranges = selectedRanges();
		for (const auto &range : ranges) {
			for (int row = range.topRow(); row <= range.bottomRow(); ++row) {
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
				if (!item(row, col)) setItem(row, col, new QTableWidgetItem());
				item(row, col)->setText(columns[j]);
			}
		}
	}
	void pasteFromClip()
	{
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
				if (!item(row, col)) setItem(row, col, new QTableWidgetItem());
				item(row, col)->setText(columns[j]);
			}
		}
	}
private:
	inline static std::vector<QStringList> innerClipboard {};
	QAction *createAction(const QString &text, const QKeySequence &shortcut, QObject *receiver, const char *slot) {
		QAction *action = new QAction(text, this);
		action->setShortcut(shortcut);
		connect(action, SIGNAL(triggered()), receiver, slot);
		return action;
	}
	void clearSelectionContent() {
		QList<QTableWidgetSelectionRange> ranges = selectedRanges();
		for (const auto &range : ranges) {
			for (int row = range.topRow(); row <= range.bottomRow(); ++row) {
				for (int col = range.leftColumn(); col <= range.rightColumn(); ++col) {
					if (item(row, col)) {
						item(row, col)->setText("");
					}
				}
			}
		}
	}
};

void MyQTableWidget::SetItemEditableState(QTableWidgetItem * item, bool editableNewState)
{
	if(!item) { qCritical() << "MyQTableWidget::SetItemEditableState nullptr item"; return; }
	auto flags = item->flags();
	flags.setFlag(Qt::ItemIsEditable, editableNewState);
	item->setFlags(flags);
}

#endif // MYQTABLEWIDGET_H
