#ifndef MYQTABLEVIEW_H
#define MYQTABLEVIEW_H

#include <set>

#include <QDebug>
#include <QTableView>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QKeyEvent>
#include <QStandardItemModel>

#include "MyQShortings.h"
#include "MyQString.h"

class ColoriserDelegate;

class MyQTableView : public QTableView
{
	Q_OBJECT
public:
	inline explicit MyQTableView(QWidget *parent = nullptr);
	inline virtual ~MyQTableView() = default;

	///\brief окрашивает ячейки по правилу и в цвет задаваемые в ColoriserDelegate
	/// для вызова отрисовки можно использовать tableView->viewport()->update();
	/// 	или tableView->model()->dataChanged(left, right, {Qt::DisplayRole, Qt::BackgroundRole});
	ColoriserDelegate *coloriserDelegate;

	///\brief для отключения стандартного поиска в QTableView при нажатии букв
	/// обработка происходит в keyPressEvent
	bool disableKeyboardSearch = true;

	enum wheelScrollBehaviors { moveScrollBar, moveCurrentIndex };
	wheelScrollBehaviors wheelScrollBehavior = moveScrollBar; // обработка происходит в wheelEvent

	inline int RowsCount(bool do_fetch);

	inline auto currentRow() { return currentIndex().row(); }
	inline auto cellData(int row, int col) { return model()->index(row, col).data(); }
	inline auto cellDataStr(int row, int col) { return cellData(row, col).toString(); }

	inline auto currentRecordData(int col) { return model()->index(currentIndex().row(), col).data(); }
	inline auto currentRecordDataStr(int col) { return currentRecordData(col).toString(); }

	inline int FindRowByValue(int fieldIndex, const QString &fieldValue);
	inline bool Locate(const QString &fieldName, const QString &fieldValue, int columnToSet = -1);
	inline bool Locate(int fieldIndex, const QString &fieldValue, int columnToSet = -1);
	inline bool LocateRow(int row, int columnToSet = -1); // col = -1 и останется в текущей колонке

	inline QStringList Field(int index);

	///\brief for empty feildsIndexes returns all fields
	inline std::vector<QStringList> ToTable(std::vector<int> feildsIndexes = {}); // QStringList = row

	std::set<int> editableColsIndexes;
	std::set<QString> editableColsNames;

	///\brief для активации редактируемых колонок
	inline void setModel(QAbstractItemModel *model) override;

protected:
	inline void keyPressEvent(QKeyEvent* event) override;
	inline void wheelEvent(QWheelEvent* event) override;
	inline void keyboardSearch(const QString &search) override;

private:
	inline bool IsKeyArrow(int key);
	inline bool IsKeyEditTrigger(int key);
private slots:
	inline void ActivateEditableCols(const QModelIndex &current, const QModelIndex &);
private:
	EditTriggers specialEditTriggers;
};

//------------------------------------------------------------------------------------------------------------------------------------------

class ColoriserDelegate : public QStyledItemDelegate
{
public:
	using highliteRule = std::function<QColor(const QModelIndex &currentIndex)>;
	std::vector<highliteRule> highliteRules;

	ColoriserDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		for(auto &rule:highliteRules)
			if(rule)
				if(auto color = rule(index); color.isValid())
					painter->fillRect(option.rect, color); // окрашивание
		// вызов базового метода для отрисовки ячейки
		QStyledItemDelegate::paint(painter, option, index);
	}
};

//------------------------------------------------------------------------------------------------------------------------------------------

MyQTableView::MyQTableView(QWidget *parent) : QTableView{parent}
{
	coloriserDelegate = new ColoriserDelegate(this); // имеет parent, можно new
	setItemDelegate(coloriserDelegate);
}

int MyQTableView::RowsCount(bool do_fetch)
{
	QAbstractItemModel *model = this->model();
	if(do_fetch)
	{
		while (model->canFetchMore(QModelIndex())) {
			model->fetchMore(QModelIndex());
		}
	}
	return model->rowCount();
}

int MyQTableView::FindRowByValue(int fieldIndex, const QString &fieldValue)
{
	QAbstractItemModel *model = this->model();
	if (!model) return -3;

	if(fieldIndex >= model->columnCount()) return -2;

	for (int row = 0; row < model->rowCount(); ++row)
	{
		QModelIndex index = model->index(row, fieldIndex);
		if(index.data().toString() == fieldValue) return row;

		// догрузка записей
		if(row >= model->rowCount()-2 && model->canFetchMore(QModelIndex()))
			model->fetchMore(QModelIndex());
	}

	return -1;
}

inline bool MyQTableView::Locate(const QString &fieldName, const QString &fieldValue, int columnToSet)
{
	QAbstractItemModel *model = this->model();
	if (!model) return false;

	// определение колонки для этого поля
	int columnOfField = -1;
	for (int i = 0; i < model->columnCount(); ++i)
	{
		if (model->headerData(i, Qt::Horizontal).toString() == fieldName)
		{
			columnOfField = i;
			break;
		}
	}
	if (columnOfField == -1) return false;

	return Locate(columnOfField, fieldValue, columnToSet);
}

bool MyQTableView::Locate(int fieldIndex, const QString &fieldValue, int columnToSet)
{
	QAbstractItemModel *model = this->model();
	if (!model) return false;

	if(columnToSet == -1) columnToSet = this->currentIndex().column();
	if(columnToSet == -1) columnToSet = 0;

	int row = FindRowByValue(fieldIndex, fieldValue);
	if(row >= 0) return LocateRow(row, columnToSet);
	else return false;
}

inline bool MyQTableView::LocateRow(int row, int columnToSet)
{
	QAbstractItemModel *model = this->model();
	if (!model) return false;

	if(columnToSet == -1) columnToSet = this->currentIndex().column();
	if(columnToSet == -1) columnToSet = 0;

	while(row >= model->rowCount() && model->canFetchMore(QModelIndex()))
		model->fetchMore(QModelIndex());
	QModelIndex index = model->index(row, columnToSet);
	if(index.isValid())
	{
		setCurrentIndex(index);
		scrollTo(index, QAbstractItemView::PositionAtCenter);
		return true;
	}
	return false;
}

QStringList MyQTableView::Field(int index)
{
	auto table = ToTable({index});
	QStringList res = MyQString::QStringListSized(table.size());
	for(uint i=0; i<table.size(); i++)
		res[i] = std::move(table[i][0]);
	return res;
}

std::vector<QStringList> MyQTableView::ToTable(std::vector<int> feildsIndexes)
{
	QAbstractItemModel *model = this->model();
	if (!model) return {};
	std::vector<QStringList> table;
	int colCount = model->columnCount();
	if(feildsIndexes.empty())
	{
		for(int column=0; column<colCount; column++)
			feildsIndexes.emplace_back(column);
	}

	auto removeRes = std::remove_if(feildsIndexes.begin(),feildsIndexes.end(),[colCount](int n){ return n>=colCount || n<0; });
	if(removeRes != feildsIndexes.end()) QMbError("Wrong indexes in feildsIndexes ["+MyQString::AsDebug(feildsIndexes)+"]");
	feildsIndexes.erase(removeRes, feildsIndexes.end());
	int size = feildsIndexes.size();
	for (int row = 0; row < model->rowCount(); ++row)
	{
		auto &retRow = table.emplace_back();
		for(int i=0; i<size; i++)
		{
			retRow += model->index(row, feildsIndexes[i]).data().toString();
		}

		// догрузка записей
		if(row >= model->rowCount()-2 && model->canFetchMore(QModelIndex()))
			model->fetchMore(QModelIndex());
	}

	return table;
}

void MyQTableView::setModel(QAbstractItemModel *model)
{
	if (selectionModel())
		disconnect(selectionModel(), &QItemSelectionModel::currentChanged, this, &MyQTableView::ActivateEditableCols);

	QTableView::setModel(model);

	int columnCount = model->columnCount();
	for (int col = 0; col < columnCount; ++col)
	{
		QString colName = model->headerData(col, Qt::Horizontal).toString();
		if(editableColsNames.count(colName) > 0)
		{
			editableColsIndexes.insert(col);
		}
	}

	if(selectionModel())
		connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &MyQTableView::ActivateEditableCols);
}

void MyQTableView::keyPressEvent(QKeyEvent *event)
{
	// если это edit trigger
	if(IsKeyEditTrigger(event->key()))
	{
		// если это Enter
		if((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
				state() != QAbstractItemView::EditingState) // и сейчас не редактирование
		{
			// Принудительно запускаем редактирование текущей ячейки
				// потому что с какого-то хера в Qt Enter не запускает редактирование
			QModelIndex current = currentIndex();
			if (current.isValid()) {
				edit(current);
				event->accept();
				return;
			}
		}
		// в остальных случаях - отрабатывается как обычно
		else { QTableView::keyPressEvent(event); return; }
	}

	QTableView::keyPressEvent(event);
}

void MyQTableView::wheelEvent(QWheelEvent *event) {
	if(wheelScrollBehavior == moveScrollBar)
	{
		QTableView::wheelEvent(event);
		return;
	}
	else if(wheelScrollBehavior == moveCurrentIndex)
	{
		QModelIndex current = currentIndex();
		int row = current.row();

		// Направление прокрутки
		int delta = event->angleDelta().y(); // положительное — вверх, отрицательное — вниз

		int newRow = row + (delta < 0 ? 1 : -1);
		int rowCount = model()->rowCount();

		// Проверка границ
		if (newRow >= 0 && newRow < rowCount) {
			QModelIndex newIndex = model()->index(newRow, current.column());
			setCurrentIndex(newIndex);
			scrollTo(newIndex);
		}
		return;
	}
}

void MyQTableView::keyboardSearch(const QString &search)
{
	if(disableKeyboardSearch) return;
	QAbstractItemView::keyboardSearch(search);
}

bool MyQTableView::IsKeyArrow(int key)
{
	if(key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right)
		return true;
	return false;
}

bool MyQTableView::IsKeyEditTrigger(int key)
{
	QAbstractItemView::EditTriggers currentTriggers = editTriggers();

	// если стоят триггеры AllEditTriggers или AnyKeyPressed
	if(currentTriggers == AllEditTriggers || currentTriggers & AnyKeyPressed)
	{
		// нужно именно  == AllEditTriggers потому что если & AllEditTriggers любой триггер будет выдавать true
		return true;
	}

	if(currentTriggers & EditKeyPressed) // если стоят триггеры EditKeyPressed
	{
		if(key == Qt::Key_F2 || key == Qt::Key_Enter || key == Qt::Key_Return)
			return true;
	}

	//qdbg << "not edit trugger; " << currentTriggers;
	return false;
}

void MyQTableView::ActivateEditableCols(const QModelIndex &current, const QModelIndex &)
{
	if(editTriggers() != QAbstractItemView::NoEditTriggers) specialEditTriggers = editTriggers();

	if(editableColsIndexes.count(current.column()) == 0)
	{
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}
	else
	{
		setEditTriggers(specialEditTriggers);
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif // MYQTABLEVIEW_H
//------------------------------------------------------------------------------------------------------------------------------------------
