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

class MyQTableWidget : public QTableWidget {
	Q_OBJECT
public:
	MyQTableWidget(QWidget *parent = nullptr) : QTableWidget(parent) {
		// Подключение контекстного меню
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, &QWidget::customContextMenuRequested, this, &MyQTableWidget::showContextMenu);
		// Горячие клавиши
		addAction(createAction(tr("Cut"), QKeySequence::Cut, this, SLOT(cut())));
		addAction(createAction(tr("Copy"), QKeySequence::Copy, this, SLOT(copy())));
		addAction(createAction(tr("Paste"), QKeySequence::Paste, this, SLOT(paste())));
		setSelectionMode(QAbstractItemView::ContiguousSelection);  // Режим выделения
	}
protected:
	void keyPressEvent(QKeyEvent *event) override {
		if (event->matches(QKeySequence::Cut)) {
			cut();
		} else if (event->matches(QKeySequence::Copy)) {
			copy();
		} else if (event->matches(QKeySequence::Paste)) {
			paste();
		} else {
			QTableWidget::keyPressEvent(event);
		}
	}
private slots:
	void showContextMenu(const QPoint &pos) {
		QMenu menu(this);
		QAction *cutAction = menu.addAction("Cut");
		QAction *copyAction = menu.addAction("Copy");
		QAction *pasteAction = menu.addAction("Paste");
		connect(cutAction, &QAction::triggered, this, &MyQTableWidget::cut);
		connect(copyAction, &QAction::triggered, this, &MyQTableWidget::copy);
		connect(pasteAction, &QAction::triggered, this, &MyQTableWidget::paste);
		menu.exec(mapToGlobal(pos));
	}
	void cut() {
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

#endif // MYQTABLEWIDGET_H
