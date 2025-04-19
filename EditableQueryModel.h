#ifndef EditableQueryModel_H
#define EditableQueryModel_H

#include <QSqlQueryModel>
#include <QDebug>
#include <QSqlRecord>

class EditableQueryModel : public QSqlQueryModel {
    Q_OBJECT
public:
    using QSqlQueryModel::QSqlQueryModel;

    Qt::ItemFlags flags(const QModelIndex &index) const override {
        if (!index.isValid())
            return Qt::NoItemFlags;
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

	struct cacheItem{
		int row;
		int col;
		QVariant data;
	};
	QVector<cacheItem> dataCache;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
        if (!index.isValid() || role != Qt::EditRole)
            return false;

//        qDebug() << "[EditableQueryModel] Edited cell at row" << index.row()
//                 << ", column" << index.column() << "-> new value:" << value;

		dataCache.push_back({index.row(), index.column(), value});

		emit dataChanged(index, index);

        return true; // Сообщаем, что редактирование прошло успешно (визуально)
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		if (index.isValid() && role == Qt::DisplayRole) {
			for(int i=dataCache.size()-1; i>=0; i--)
				if(dataCache[i].row == index.row() && dataCache[i].col == index.column())
					return dataCache[i].data;
			return QSqlQueryModel::data(index, role);
		}
		return QSqlQueryModel::data(index, role);
	}
};

#endif
