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
	explicit EditableQueryModel(QString name, QObject *parent = nullptr): QSqlQueryModel(parent)
	{
		InitCacheWriter(std::move(name));
	}

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
	std::map<RowCol, QVariant> updated;
	std::vector<std::function<void(int row, int col, QVariant newValue)>> updaters;
	mutable thread_box cacheWriter;
	inline void InitCacheWriter(QString name)
	{

		std::string stdName = name.toStdString();
		stdName += "_cacheWriter";
		cacheWriter.set_name(std::move(stdName));
		cacheWriter.start([this](){
			while (!cacheWriter.stopper) {
				MyCppDifferent::sleep_ms(10);
				DO_ONCE(qdbg << "EditableQueryModel::InitCacheWriter нужно использовать cv здесь, а не sleep_ms(10)";);

				while (!dataCache.empty())
				{
					cacheWriter.mtx.lock();
					auto &frontCache = *dataCache.begin();
					int row = frontCache.first.first, col = frontCache.first.second;
					updated[{row, col}] = frontCache.second;
					cacheWriter.mtx.unlock();

					QMetaObject::invokeMethod(this, [this, row, col](){
						std::lock_guard(cacheWriter.mtx);
						QVariant &value = updated[{row, col}];
						for(auto &updater:updaters)
							updater(row, col, value);
					});

					std::lock_guard(cacheWriter.mtx);
					dataCache.erase(dataCache.begin());
				}
			}
		});
	}


	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override {
		if (!index.isValid() || role != Qt::EditRole)
			return false;

		cacheWriter.mtx.lock();
		dataCache[{index.row(), index.column()}] = value;
		cacheWriter.mtx.unlock();

		emit dataChanged(index, index);

		return true; // Сообщаем, что редактирование прошло успешно (визуально)
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		if (index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole))
		{
			std::lock_guard(cacheWriter.mtx);
			if(auto findRes = dataCache.find({index.row(), index.column()}); findRes != dataCache.end())
			{
				return findRes->second;
			}

			if(auto findRes = updated.find({index.row(), index.column()}); findRes != updated.end())
			{
				return findRes->second;
			}

			return QSqlQueryModel::data(index, role);
		}
		return QSqlQueryModel::data(index, role);
	}
};

#endif
