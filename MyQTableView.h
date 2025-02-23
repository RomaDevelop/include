#ifndef MYQTABLEVIEW_H
#define MYQTABLEVIEW_H

#include <QDebug>
#include <QTableView>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QPainter>

#include "MyQShortings.h"
#include "MyQString.h"

class ColoriserDelegate;

class MyQTableView : public QTableView
{
	Q_OBJECT
public:
	inline explicit MyQTableView(QWidget *parent = nullptr);
	virtual ~MyQTableView() = default;

	ColoriserDelegate *coloriserDelegate;

	inline QString saveColumnWidths()
	{
		QString ret;
		for (int column = 0; column < model()->columnCount(); ++column) {
			ret += QSn(columnWidth(column)) + ";";
		}
		return ret;
	}
	inline void restoreColumnWidths(const QString &widthsStr)
	{
		QStringList widths = widthsStr.split(";", QString::SkipEmptyParts);
		for(int i=0; i<widths.size() && i<model()->columnCount(); i++)
		{
			bool ok;
			int width = widths[i].toInt(&ok);
			if(ok) setColumnWidth(i, width);
			else QMbError("restoreColumnWidths wrong widthsStr: [" + widthsStr + "]");
		}
	}

	inline int RowsCount(bool do_fetch);

	inline auto currentRow() { return currentIndex().row(); }
	inline auto cellData(int row, int col) { return model()->index(row, col).data(); }
	inline auto cellDataStr(int row, int col) { return cellData(row, col).toString(); }

	inline auto currentRecordData(int col) { return model()->index(currentIndex().row(), col).data(); }
	inline auto currentRecordDataStr(int col) { return currentRecordData(col).toString(); }

	inline bool Locate(const QString &fieldName, const QString &fieldValue, int columnToSet = -1);
	inline bool LocateRow(int row, int column = -1); // col = -1 и останется в текущей колонке

	///\brief for empty feildsIndexes return all fields
	inline std::vector<QStringList> ToTable(std::vector<int> feildsIndexes)
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
};

class ColoriserDelegate : public QStyledItemDelegate {
public:
	ColoriserDelegate(QObject *parent = nullptr)
		: QStyledItemDelegate(parent) {}

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
		// Проверяем, является ли текущая строка выделенной
		if (highliteRule && highliteRule(index)) {
			painter->fillRect(option.rect, color); // Цвет выделения
		}
		// Вызываем базовый метод для отрисовки ячейки
		QStyledItemDelegate::paint(painter, option, index);
	}

	std::function<bool(const QModelIndex &currentIndex)> highliteRule;
	QColor color;
};

MyQTableView::MyQTableView(QWidget *parent) : QTableView{parent}
{
	coloriserDelegate = new ColoriserDelegate(this);
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

#endif // MYQTABLEVIEW_H
