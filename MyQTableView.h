#ifndef MYQTABLEVIEW_H
#define MYQTABLEVIEW_H

#include <QDebug>
#include <QTableView>
#include <QMessageBox>

#include "MyQShortings.h"

class MyQTableView : public QTableView
{
	Q_OBJECT
public:
	explicit MyQTableView(QWidget *parent = nullptr) : QTableView{parent} {}

	QString saveColumnWidths()
	{
		QString ret;
		for (int column = 0; column < model()->columnCount(); ++column) {
			ret += QSn(columnWidth(column)) + ";";
		}
		return ret;
	}
	void restoreColumnWidths(const QString &widthsStr)
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

	auto currentRow() { return currentIndex().row(); }
	auto cellData(int row, int col) { return model()->index(row, col).data(); }
	auto cellDataStr(int row, int col) { return cellData(row, col).toString(); }

	auto currentRecordData(int col) { return model()->index(currentIndex().row(), col).data(); }
	auto currentRecordDataStr(int col) { return currentRecordData(col).toString(); }

	bool Locate(const QString &fieldName, const QString &fieldValue);
	bool LocateRow(int row, int col);
};

inline bool MyQTableView::Locate(const QString &fieldName, const QString &fieldValue)
{
	QAbstractItemModel *model = this->model();
	if (!model) return false;

	int column = -1;
	for (int i = 0; i < model->columnCount(); ++i)
	{
		if (model->headerData(i, Qt::Horizontal).toString() == fieldName)
		{
			column = i;
			break;
		}
	}
	if (column == -1) return false;

	for (int row = 0; row < model->rowCount(); ++row)
	{
		QModelIndex index = model->index(row, column);
		if (index.isValid() && model->data(index).toString() == fieldValue)
		{
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

inline bool MyQTableView::LocateRow(int row, int col)
{
	QAbstractItemModel *model = this->model();
	if (!model) return false;

	while(row >= model->rowCount() && model->canFetchMore(QModelIndex()))
		model->fetchMore(QModelIndex());
	QModelIndex index = model->index(row, col);
	if(index.isValid())
	{
		setCurrentIndex(index);
		scrollTo(index, QAbstractItemView::PositionAtTop);
		return true;
	}
	return false;
}

#endif // MYQTABLEVIEW_H
