#ifndef MYQTABLEVIEW_H
#define MYQTABLEVIEW_H

#include <QTableView>

class MyQTableView : public QTableView
{
	Q_OBJECT
public:
	explicit MyQTableView(QWidget *parent = nullptr) : QTableView{parent} {}

	auto currentRow() { return currentIndex().row(); }
	auto cellData(int row, int col) { return model()->index(row, col).data(); }
	auto cellDataStr(int row, int col) { return cellData(row, col).toString(); }

	auto currentRecordData(int col) { return model()->index(currentIndex().row(), col).data(); }
	auto currentRecordDataStr(int col) { return currentRecordData(col).toString(); }
};

#endif // MYQTABLEVIEW_H
