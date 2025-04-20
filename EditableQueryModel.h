#ifndef EditableQueryModel_H
#define EditableQueryModel_H

#include <queue>

#include <QSqlQueryModel>
#include <QDebug>
#include <QSqlRecord>
#include <QDateTime>

#include "MyCppDifferent.h"
#include "thread_box.h"
#include "MyQShortings.h"

class EditableQueryModel : public QSqlQueryModel {
	Q_OBJECT
public:
	explicit EditableQueryModel(QString /*name*/, QObject *parent = nullptr):
		QSqlQueryModel(parent)
	{ }

	Qt::ItemFlags flags(const QModelIndex &index) const override {
		if (!index.isValid())
			return Qt::NoItemFlags;
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
	}

	struct cacheItem{
		int row;
		int col;
		QVariant data;
		//QDateTime dtCached;
	};
	using RowCol = std::pair<int,int>;

	std::map<RowCol, QVariant> dataCache;
	std::vector<std::function<void(int row, int col, QVariant newValue)>> updaters;

	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
		if (!index.isValid() || role != Qt::EditRole)
			return false;

		dataCache[{index.row(), index.column()}] = value;

		for(auto &updater:updaters)
			updater(index.row(), index.column(), value);

		emit dataChanged(index, index);

		return true; // Сообщаем, что редактирование прошло успешно (визуально)
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		if (index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole))
		{
			if(auto findRes = dataCache.find({index.row(), index.column()}); findRes != dataCache.end())
			{
				return findRes->second;
			}

			return QSqlQueryModel::data(index, role);
		}
		return QSqlQueryModel::data(index, role);
	}
};

#endif
