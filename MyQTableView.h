#ifndef MYQTABLEVIEW_H
#define MYQTABLEVIEW_H

#include <set>

#include <QDebug>
#include <QTableView>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QKeyEvent>

#include "MyQShortings.h"
#include "MyQString.h"

class ColoriserDelegate;

class MyQTableView : public QTableView
{
	Q_OBJECT
public:
	inline explicit MyQTableView(QWidget *parent = nullptr);
	inline virtual ~MyQTableView() = default;

	ColoriserDelegate *coloriserDelegate;
	bool keyBoardSearch = true; // обработка происходит в keyPressEvent
	enum wheelScrollBehaviors { moveScrollBar, moveCurrentIndex };
	wheelScrollBehaviors wheelScrollBehavior = moveScrollBar; // обработка происходит в wheelEvent

	inline int RowsCount(bool do_fetch);

	inline auto currentRow() { return currentIndex().row(); }
	inline auto cellData(int row, int col) { return model()->index(row, col).data(); }
	inline auto cellDataStr(int row, int col) { return cellData(row, col).toString(); }

	inline auto currentRecordData(int col) { return model()->index(currentIndex().row(), col).data(); }
	inline auto currentRecordDataStr(int col) { return currentRecordData(col).toString(); }

	inline bool Locate(const QString &fieldName, const QString &fieldValue, int columnToSet = -1);
	inline bool LocateRow(int row, int column = -1); // col = -1 и останется в текущей колонке

	///\brief for empty feildsIndexes returns all fields
	inline std::vector<QStringList> ToTable(std::vector<int> feildsIndexes = {}); // QStringList = row

	std::set<int> editableColsIndexes;
	std::set<QString> editableColsNames;

	void setModel(QAbstractItemModel *model) override
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
	inline void keyPressEvent(QKeyEvent* event) override;
	inline void wheelEvent(QWheelEvent* event) override;
private:
	inline bool CheckArrows(QKeyEvent* event);
	inline bool CheckEditTriggers(QKeyEvent* event);
private slots:
	void ActivateEditableCols(const QModelIndex &current, const QModelIndex &)
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
private:
	EditTriggers specialEditTriggers;
};

//------------------------------------------------------------------------------------------------------------------------------

class ColoriserDelegate : public QStyledItemDelegate
{
public:
	std::function<bool(const QModelIndex &currentIndex)> highliteRule;
	QColor color;

	ColoriserDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		if (highliteRule && highliteRule(index)) // проверка правила
			painter->fillRect(option.rect, color); // окрашивание
		// вызов базового метода для отрисовки ячейки
		QStyledItemDelegate::paint(painter, option, index);
	}
};

//------------------------------------------------------------------------------------------------------------------------------

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

inline bool MyQTableView::Locate(const QString &fieldName, const QString &fieldValue, int columnToSet)
{
	QAbstractItemModel *model = this->model();
	if (!model) return false;

	if(columnToSet == -1) columnToSet = this->currentIndex().column();
	if(columnToSet == -1) columnToSet = 0;

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

	for (int row = 0; row < model->rowCount(); ++row)
	{
		QModelIndex index = model->index(row, columnOfField);
		if(index.data().toString() == fieldValue)
		{
			index = model->index(row, columnToSet);
			this->setCurrentIndex(index);
			this->scrollTo(index, QAbstractItemView::PositionAtTop);
			return true;
		}

		// догрузка записей
		if(row >= model->rowCount()-2 && model->canFetchMore(QModelIndex()))
			model->fetchMore(QModelIndex());
	}
	return false;
}

inline bool MyQTableView::LocateRow(int row, int column)
{
	QAbstractItemModel *model = this->model();
	if (!model) return false;

	if(column == -1) column = this->currentIndex().column();
	if(column == -1) column = 0;

	while(row >= model->rowCount() && model->canFetchMore(QModelIndex()))
		model->fetchMore(QModelIndex());
	QModelIndex index = model->index(row, column);
	if(index.isValid())
	{
		setCurrentIndex(index);
		scrollTo(index, QAbstractItemView::PositionAtTop);
		return true;
	}
	return false;
}

std::vector<QStringList> MyQTableView::ToTable(std::vector<int> feildsIndexes)
{
	QAbstractItemModel *model = this->model();
	if (!model) return {};
	std::vector<QStringList> table;
	int colCount = model->columnCount();
	if(feildsIndexes.empty())
	{
		for (int row = 0; row < model->rowCount(); ++row)
		{
			auto &retRow = table.emplace_back();
			for(int column=0; column<colCount; column++)
			{

				retRow += model->index(row, column).data().toString();
			}

			// догрузка записей
			if(row >= model->rowCount()-2 && model->canFetchMore(QModelIndex()))
				model->fetchMore(QModelIndex());
		}
	}
	else
	{
		auto removeRes = std::remove_if(feildsIndexes.begin(),feildsIndexes.end(),[colCount](int n){ return n>=colCount || n<0; });
		if(removeRes != feildsIndexes.end()) QMbError("Wring indexes in feildsIndexes ["+MyQString::AsDebug(feildsIndexes)+"]");
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
	}
	return table;
}

void MyQTableView::keyPressEvent(QKeyEvent *event)
{
	if(!keyBoardSearch) // если отключен поиск
	{
		if(!CheckArrows(event)) // если нажатые клавиши не стрелки
			if(!CheckEditTriggers(event)) // если нажатые клавиши не активируют редактирование ячейки
			{
				event->ignore(); // игнорируем событие
				//qdbg << "event->ignore()";
				return;
			}
	}
	QTableView::keyPressEvent(event); // иначе, запускается стандартный обработчик
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

bool MyQTableView::CheckArrows(QKeyEvent *event)
{
	int key = event->key();
	if(key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_Left || key == Qt::Key_Right)
		return true;
	return false;
}

bool MyQTableView::CheckEditTriggers(QKeyEvent *event)
{
	QAbstractItemView::EditTriggers currentTriggers = editTriggers();
	if(currentTriggers == NoEditTriggers) // если нет триггеров редактирования
		return false;

	int key = event->key();
	//Qt::KeyboardModifiers modifiers = event->modifiers();

	if(currentTriggers & EditKeyPressed) // если стоят триггеры EditKeyPressed
	{
		if(key == Qt::Key_F2 || key == Qt::Key_Enter || key == Qt::Key_Return)
			return true;
		else return false;
	}

	if(currentTriggers & AnyKeyPressed) // если стоят триггеры EditKeyPressed
	{
		return true;
	}

	static bool pritedError = 0;
	if(!pritedError) qdbg << "unrealesed case MyQTableView::CheckEditTriggers" << currentTriggers << QSn(key,16);

	return true;

	//		// Проверка AnyKeyPressed — любая клавиша без модификаторов (Ctrl, Alt, Meta)
	//		if (currentTriggers & QAbstractItemView::AnyKeyPressed) {
	//			// Если клавиша не модификаторная (не Ctrl, Alt, Meta)
	//			if (!(modifiers & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
	//				return true;  // Любая клавиша без модификаторов
	//			}
	//		}

	//		// Проверка EditKeyPressed — клавиши, активирующие редактирование
	//		if (currentTriggers & QAbstractItemView::EditKeyPressed) {
	//			// Разрешённые клавиши: цифры, буквы, русские буквы (если требуется), спецсимволы
	//			if ((key >= Qt::Key_0 && key <= Qt::Key_9) ||   // Цифры
	//				(key >= Qt::Key_A && key <= Qt::Key_Z) ||   // Буквы (латиница)
	//				//(key >= Qt::Key_А && key <= Qt::Key_Я) ||   // Русские буквы (если актуально)
	//				key == Qt::Key_Backspace || key == Qt::Key_Delete ||  // Клавиши удаления
	//				key == Qt::Key_Space || key == Qt::Key_Enter || key == Qt::Key_Return ||  // Пробел, Enter, Return
	//				key == Qt::Key_Tab ||  // Tab
	//				key == Qt::Key_Plus || key == Qt::Key_Minus || key == Qt::Key_Asterisk ||  // Операторы
	//				key == Qt::Key_Slash || key == Qt::Key_Period || key == Qt::Key_Comma ||  // Прочие символы
	//				key == Qt::Key_Semicolon || key == Qt::Key_Colon) {
	//				return true;  // Клавиши, активирующие редактирование
	//			}
	//		}

	//		// Проверка AllEditTriggers — если все триггеры разрешают редактирование
	//		if (currentTriggers == QAbstractItemView::AllEditTriggers) {
	//			return true;
	//		}

	//		return false;  // Никакой триггер не активирует редактирование
}




//------------------------------------------------------------------------------------------------------------------------------
#endif // MYQTABLEVIEW_H
