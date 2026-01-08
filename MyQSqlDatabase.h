#ifndef MyQSqlDatabase_H
#define MyQSqlDatabase_H

#include <algorithm>
#include <set>

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDir>

#include "declare_struct.h"
#include "MyQShortings.h"
#include "MyQString.h"
#include "MyQFileDir.h"

using QStringPair = std::pair<QString,QString>;
using QStringPairVector = std::vector<QStringPair>;
using QStringListVector = std::vector<QStringList>;
using QStringRefWr_c = std::reference_wrapper<const QString>;

struct BaseData
{
	QString baseName;
	QString baseFilePathName;
	QString baseFileNoPath;
	QString pathDataBase;
	QString pathBackup;
	QString storagePath;

	BaseData() = default;
	BaseData(QString baseName, QString baseFilePathName, QString storagePath):
	    baseName{baseName},
	    baseFilePathName{baseFilePathName},
	    pathDataBase{ QFileInfo(baseFilePathName).path() },
	    pathBackup{ QFileInfo(baseFilePathName).path() + "/backup" },
	    storagePath{storagePath}
	{
		this->baseFilePathName = QDir::toNativeSeparators(this->baseFilePathName);
		this->baseFileNoPath = QFileInfo(baseFilePathName).fileName();
	}
};

//-------------------------------------------------------------------------------------------------------------------------------------

class MyQSqlDatabase
{
public:
	/// объект основной БД программы, инициализируется вызовом Init
	inline static QSqlDatabase QSqlDbMain;

	/// указатель на текущий объект БД, все запросы в БД вызываеются через него
	/// при вызове Init в указатель записываеся QSqlDbMain
	/// в дальнейшем, если нужно провести работу с другой БД, сюда записать указатель на неё, а потом вернуть обратно QSqlDbMain
	inline static QSqlDatabase *currentQSqlDb = nullptr;

private:
	inline static BaseData baseDataMain;
	inline static BaseData baseDataCurrent;
	//inline static std::vector<BaseData> baseDataAddBases;
public:
	static const BaseData& BaseDataMain() { return baseDataMain; }
	static const BaseData& BaseDataCurrent() { return baseDataCurrent; }
	static void SetBaseDataCurrent(BaseData baseData) { baseDataCurrent = std::move(baseData); }

	using logWorkerFunction = std::function<void(const QString &str)>;
	inline static void Init(BaseData mainBase_, /*std::vector<BaseData> additionalBases_ = {},*/
	                        logWorkerFunction logWorker_ = {}, logWorkerFunction errorWorker_ = {});

	inline static void MakeBackupBase(bool showSuccessMsg);

	inline static void Log(const QString &str) { if(logWorker) logWorker(str); else qdbg << str; }
	inline static void Error(const QString &str) { if(errorWorker) errorWorker(str); else qdbg << str; }

	inline static logWorkerFunction logWorker;
	inline static logWorkerFunction errorWorker;

	/// binds = вектор пар строк - именованная привязка (в запросе размещаем :name)
	inline static QSqlQuery DoSqlQuery(const QString &strQuery, const QStringPairVector &binds = {},
	                                   bool doNextAfterExec = false, bool showErrorIfNextNotDid = false);
	/// binds = массив строк - неименованная привязка по порядку (в запросе размещаем ?)
	inline static QSqlQuery DoSqlQuery2(const QString &strQuery, const QStringList &binds = {},
	                                   bool doNextAfterExec = false, bool showErrorIfNextNotDid = false);
	declare_struct_2_fields_move(DoSqlQueryRes, QSqlQuery, query, QString, errors);
	inline static DoSqlQueryRes DoSqlQueryExt(const QString &strQuery, const QStringPairVector &binds = {},
	                                          bool doNextAfterExec = false, bool showErrorIfNextNotDid = false);
	inline static DoSqlQueryRes DoSqlQueryExt2(const QString &strQuery, const QStringList &binds = {},
	                                          bool doNextAfterExec = false, bool showErrorIfNextNotDid = false);
	inline static QString DoSqlQueryGetFirstCell(const QString &strQuery, const QStringPairVector &binds = {},
	                                             bool showErrorIfEmptyQuery = true);
	inline static QStringList DoSqlQueryGetFirstRec(const QString &strQuery, const QStringPairVector &binds = {});
	inline static QStringList DoSqlQueryGetFirstField(const QString &strQuery, const QStringPairVector &binds = {});
	inline static QStringList DoSqlQueryGetFirstField2(const QString &strQuery, const QStringList &binds = {});
	inline static QStringPairVector DoSqlQueryGetFirstTwoFields(const QString &strQuery, const QStringPairVector &binds = {});
	inline static std::set<QString> DoSqlQueryGetFirstFieldAsSet(const QString &strQuery, const QStringPairVector &binds = {});
	inline static std::map<QString,QString> DoSqlQueryAndMakeMap(const QString &strQuery, const QStringPairVector &binds = {},
	                                                             int filedIndexForKey = 0, int filedIndexForValue = 1);
	///\brief QStringList = row
	inline static std::vector<QStringList> DoSqlQueryGetTable(const QString &strQuery, const QStringPairVector &binds = {});

	inline static std::pair<QString, QStringPairVector> MakeInsertRequest(QString table,
	                                                                      std::vector<QStringRefWr_c> fields,
	                                                                      std::vector<QString> values);
	inline static std::pair<QString, QStringPairVector> MakeUpdateRequest(QString table,
	                                                                      std::vector<QStringRefWr_c> fields,
	                                                                      std::vector<QString> values,
	                                                                      std::vector<QStringRefWr_c> whereFields,
	                                                                      std::vector<QString> whereValues);
	inline static std::pair<QString, QStringPairVector> MakeUpdateRequestOneField(QString table, QStringRefWr_c field, QString value,
	                                                                              QStringRefWr_c whereFields, QString whereValue);

	///\brief for empty feildsIndexes return all fields
	/// QStringList = row
	inline static std::vector<QStringList> QuetyToTable(QSqlQuery &query, std::vector<int> feildsIndexes = {});
	inline static QStringList FieldFromQuery(QSqlQuery &query, int filedIndex);
	inline static std::set<QString> FieldAsSetFromQuery(QSqlQuery &query, int filedIndex);
	inline static std::map<QString,QString> MapFromQuery(QSqlQuery &query, int filedIndexMakeKey, int filedIndexMakeValue);
	inline static double ToDouble(QSqlQuery &query, QString fieldName);
	inline static double ToDouble(QSqlQuery &query, int fieldIndex);

	inline static QString GenErrorText(QString error, const QString &strQuery, const QStringPairVector &binds = {});
	inline static QString GenErrorText(QString error, const QString &strQuery, const QStringList &binds = {});
	inline static void ShowErrorForQuery(QString generatedErrorText);
	inline static void ShowErrorForQuery(QString error, const QString &strQuery, const QStringPairVector &binds = {});
};

//-------------------------------------------------------------------------------------------------------------------------------------

void MyQSqlDatabase::Init(BaseData mainBase_, /*std::vector<BaseData> additionalBases_,*/
                          logWorkerFunction logWorker_, logWorkerFunction errorWorker_)
{
	logWorker = std::move(logWorker_);
	errorWorker = std::move(errorWorker_);

	baseDataMain = std::move(mainBase_);
	baseDataCurrent = baseDataMain;
	//baseDataAddBases = std::move(additionalBases_);

	QSqlDbMain = QSqlDatabase::addDatabase("QODBC");
	QSqlDbMain.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ="+baseDataCurrent.baseFilePathName+";");
	currentQSqlDb = &QSqlDbMain;

	if (!QSqlDbMain.isOpen() && !QSqlDbMain.open())
	{
		Error("MyQSqlDatabase::Init error: " + QSqlDbMain.lastError().text() + "\n(base: "+baseDataCurrent.baseFilePathName+")");
		return;
	}
	else Log("MyQSqlDatabase::Init " + baseDataCurrent.baseName + " success");
}

QSqlQuery MyQSqlDatabase::DoSqlQuery(const QString &strQuery, const QStringPairVector &binds,
                                     bool doNextAfterExec, bool showErrorIfNextNotDid)
{
	return DoSqlQueryExt(strQuery, binds, doNextAfterExec, showErrorIfNextNotDid).query;
}

QSqlQuery MyQSqlDatabase::DoSqlQuery2(const QString & strQuery, const QStringList & binds,
                                      bool doNextAfterExec, bool showErrorIfNextNotDid)
{
	return DoSqlQueryExt2(strQuery, binds, doNextAfterExec, showErrorIfNextNotDid).query;
}

MyQSqlDatabase::DoSqlQueryRes MyQSqlDatabase::DoSqlQueryExt(const QString &strQuery, const QStringPairVector &binds,
                                                            bool doNextAfterExec, bool showErrorIfNextNotDid)
{
	QString errors;
	QSqlQuery query(*currentQSqlDb);
	query.prepare(strQuery);
	for(auto &bind:binds)
		query.bindValue(bind.first, bind.second);
	if(query.exec())
	{
		if(doNextAfterExec)
		{
			if(!query.next())
			{
				errors = GenErrorText("Ошибка выполнения next: " + query.lastError().text(), strQuery, binds);
				if(showErrorIfNextNotDid)
				{
					ShowErrorForQuery(errors);
				}
			}
		}
		else
		{
			if(showErrorIfNextNotDid)
			{
				QString error = "Ошибка параметров: next не выполнялось, поскольку не установлен флаг выполения next. "
				                "Но флаг вывода ошибки если next не выполнено установлен";
				errors = GenErrorText(error, strQuery, binds);
				ShowErrorForQuery(errors);
			}
		}
	}
	else
	{
		errors = GenErrorText("Ошибка выполнения запроса: " + query.lastError().text(), strQuery, binds);
		ShowErrorForQuery(errors);
	}
	return DoSqlQueryRes(std::move(query), std::move(errors));
}

MyQSqlDatabase::DoSqlQueryRes MyQSqlDatabase::DoSqlQueryExt2(const QString & strQuery, const QStringList & binds,
                                                             bool doNextAfterExec, bool showErrorIfNextNotDid)
{
	QString errors;
	QSqlQuery query(*currentQSqlDb);
	query.prepare(strQuery);
	for(auto &bind:binds)
		query.addBindValue(bind);

	if(query.exec())
	{
		if(doNextAfterExec)
		{
			if(!query.next())
			{
				errors = GenErrorText("Ошибка выполнения next: " + query.lastError().text(), strQuery, binds);
				if(showErrorIfNextNotDid)
				{
					ShowErrorForQuery(errors);
				}
			}
		}
		else
		{
			if(showErrorIfNextNotDid)
			{
				QString error = "Ошибка параметров: next не выполнялось, поскольку не установлен флаг выполения next. "
				                "Но флаг вывода ошибки если next не выполнено установлен";
				errors = GenErrorText(error, strQuery, binds);
				ShowErrorForQuery(errors);
			}
		}
	}
	else
	{
		errors = GenErrorText("Ошибка выполнения запроса: " + query.lastError().text(), strQuery, binds);
		ShowErrorForQuery(errors);
	}
	return DoSqlQueryRes(std::move(query), std::move(errors));
}

QString MyQSqlDatabase::DoSqlQueryGetFirstCell(const QString &strQuery, const QStringPairVector &binds, bool showErrorIfEmptyQuery)
{
	auto query = DoSqlQuery(strQuery, binds);
	if(query.next()) return query.value(0).toString();
	else
	{
		if(showErrorIfEmptyQuery)
			ShowErrorForQuery("Результат запроса пуст.", strQuery, binds);
	}
	return {};
}

QStringList MyQSqlDatabase::DoSqlQueryGetFirstRec(const QString &strQuery, const QStringPairVector &binds)
{
	auto query = DoSqlQuery(strQuery, binds);
	QStringList record;
	if(query.next())
	{
		for(int i=0; i<query.record().count(); i++)
			record += query.value(i).toString();
	}
	return record;
}

QStringList MyQSqlDatabase::DoSqlQueryGetFirstField(const QString &strQuery, const QStringPairVector &binds)
{
	auto query = MyQSqlDatabase::DoSqlQuery(strQuery, binds);
	return MyQSqlDatabase::FieldFromQuery(query, 0);
}

QStringList MyQSqlDatabase::DoSqlQueryGetFirstField2(const QString & strQuery, const QStringList & binds)
{
	auto query = MyQSqlDatabase::DoSqlQuery2(strQuery, binds);
	return MyQSqlDatabase::FieldFromQuery(query, 0);
}

QStringPairVector MyQSqlDatabase::DoSqlQueryGetFirstTwoFields(const QString &strQuery, const QStringPairVector &binds)
{
	auto query = MyQSqlDatabase::DoSqlQuery(strQuery, binds);
	if(query.record().count() < 2) { Error("DoSqlQueryGetFirstTwoFields: fields in query < 2 ("+strQuery+")"); return {}; }
	QStringPairVector twoFields;
	while(query.next())
	{
		twoFields.emplace_back(query.value(0).toString(), query.value(1).toString());
	}
	return twoFields;
}

std::set<QString> MyQSqlDatabase::DoSqlQueryGetFirstFieldAsSet(const QString &strQuery, const QStringPairVector &binds)
{
	auto query = MyQSqlDatabase::DoSqlQuery(strQuery, binds);
	return MyQSqlDatabase::FieldAsSetFromQuery(query, 0);
}

std::map<QString, QString> MyQSqlDatabase::DoSqlQueryAndMakeMap(const QString &strQuery, const QStringPairVector &binds,
                                                                int filedIndexForKey, int filedIndexForValue)
{
	auto query = MyQSqlDatabase::DoSqlQuery(strQuery, binds);
	return MyQSqlDatabase::MapFromQuery(query, filedIndexForKey, filedIndexForValue);
}

std::vector<QStringList> MyQSqlDatabase::DoSqlQueryGetTable(const QString &strQuery, const QStringPairVector &binds)
{
	auto query = MyQSqlDatabase::DoSqlQuery(strQuery, binds);
	return QuetyToTable(query);
}

std::pair<QString, QStringPairVector> MyQSqlDatabase::MakeInsertRequest(QString table, std::vector<QStringRefWr_c> fields, std::vector<QString> values)
{
	if(fields.size() != values.size()) { Error("MakeInsertRequest fields values sizes differ"); return {}; }
	QString &sql = table;
	QStringPairVector binds;
	sql = "insert into " + table;
	if(!fields.empty())
	{
		sql += " (";
		for(auto &field:fields)
		{
			sql += field;
			sql += ", ";
		}
		sql.chop(2);
		sql += ")\n";
		sql += "values (";
		for(uint i=0; i<fields.size(); i++)
		{
			QStringRefWr_c &field = fields[i];
			sql += ':';
			sql += field;
			sql += ", ";

			binds.emplace_back(std::pair(":" + field, std::move(values[i])));
		}
		sql.chop(2);
		sql += ")\n";
	}
	return {std::move(sql), std::move(binds)};
}

std::pair<QString, QStringPairVector> MyQSqlDatabase::MakeUpdateRequest(QString table, std::vector<QStringRefWr_c> fields, std::vector<QString> values,
                                                                        std::vector<QStringRefWr_c> whereFields, std::vector<QString> whereValues)
{
	if(fields.size() != values.size()) { Error("MakeUpdateRequest fields values sizes differ"); return {}; }
	if(whereFields.size() != whereValues.size()) { Error("MakeUpdateRequest whereFields whereValues sizes differ"); return {}; }
	if(fields.empty()) { Error("MakeUpdateRequest empty update fields "); return {}; }

	bool contains = false;
	for(auto &field:fields)
	{
		for(auto &whereField:whereFields)
			if(whereField == field) {contains = true; break;}
	}
	if(contains) { Error("MakeUpdateRequest fields and whereFields have dubles"); return {}; }

	QString &sql = table;
	QStringPairVector binds;
	sql = "update "+table+" set ";
	for(uint i=0; i<fields.size(); i++)
	{
		QStringRefWr_c &field = fields[i];
		sql.append(field).append(" = :").append(field).append(", ");
		binds.emplace_back(std::pair(":" + field, std::move(values[i])));
	}
	if(!fields.empty()) sql.chop(2);
	if(!whereFields.empty())
	{
		sql.append("\nwhere ");
		for(uint i=0; i<whereFields.size(); i++)
		{
			QStringRefWr_c &wereField = whereFields[i];
			sql.append(wereField).append(" = :").append(wereField).append(" and ");
			binds.emplace_back(std::pair(":" + wereField, std::move(whereValues[i])));
		}
		if(!whereFields.empty()) sql.chop(5);
	}
	return {std::move(sql), std::move(binds)};
}

std::pair<QString, QStringPairVector> MyQSqlDatabase::MakeUpdateRequestOneField(QString table, QStringRefWr_c field, QString value,
                                                                                QStringRefWr_c whereFields, QString whereValue)
{
	return MakeUpdateRequest(std::move(table), {std::move(field)}, {std::move(value)}, {std::move(whereFields)}, {std::move(whereValue)});
}

std::vector<QStringList> MyQSqlDatabase::QuetyToTable(QSqlQuery &query, std::vector<int> feildsIndexes)
{
	std::vector<QStringList> table;
	int colCount = query.record().count();
	if(feildsIndexes.empty())
	{
		while(query.next())
		{
			auto &row = table.emplace_back();
			for(int i=0; i<colCount; i++)
			{
				row += query.value(i).toString();
			}
		}
	}
	else
	{
		auto removeRes = std::remove_if(feildsIndexes.begin(),feildsIndexes.end(),[colCount](int n){ return n>=colCount || n<0; });
		if(removeRes != feildsIndexes.end()) Error("Wrong indexes in feildsIndexes ["+MyQString::AsDebug(feildsIndexes)+"]");
		feildsIndexes.erase(removeRes, feildsIndexes.end());
		int size = feildsIndexes.size();
		while(query.next())
		{
			auto &row = table.emplace_back();
			for(int i=0; i<size; i++)
			{
				row += query.value(feildsIndexes[i]).toString();
			}
		}
	}
	return table;
}

QStringList MyQSqlDatabase::FieldFromQuery(QSqlQuery &query, int filedIndex)
{
	QStringList field;
	while(query.next())
	{
		field += query.value(filedIndex).toString();
	}
	return field;
}

std::set<QString> MyQSqlDatabase::FieldAsSetFromQuery(QSqlQuery &query, int filedIndex)
{
	std::set<QString> field;
	while(query.next())
	{
		field.insert(query.value(filedIndex).toString());
	}
	return field;
}

std::map<QString, QString> MyQSqlDatabase::MapFromQuery(QSqlQuery &query, int filedIndexMakeKey, int filedIndexMakeValue)
{
	std::map<QString,QString> map;
	while(query.next())
	{
		map.insert(std::pair<QString,QString>(query.value(filedIndexMakeKey).toString(), query.value(filedIndexMakeValue).toString()));
	}
	return map;
}

double MyQSqlDatabase::ToDouble(QSqlQuery &query, QString fieldName)
{
	bool ch;
	double d = query.value(fieldName).toString().replace(',','.').toDouble(&ch);
	if(!ch) Error("Ошибка ToDouble ["+query.value(fieldName).toString()+"]");
	return d;
}

double MyQSqlDatabase::ToDouble(QSqlQuery &query, int fieldIndex)
{
	bool ch;
	double d = query.value(fieldIndex).toString().replace(',','.').toDouble(&ch);
	if(!ch) Error("Ошибка ToDouble ["+query.value(fieldIndex).toString()+"]");
	return d;
}

QString MyQSqlDatabase::GenErrorText(QString error, const QString &strQuery, const QStringPairVector &binds)
{
	error += "\n\nЗапрос:\n" + strQuery + "\n\nBinds" + ( binds.empty() ? " are empty" : ":\n" );
	for(auto &bind:binds)
		error += "\n" + bind.first + " -> " + bind.second;
	return error;
}

QString MyQSqlDatabase::GenErrorText(QString error, const QString & strQuery, const QStringList & binds)
{
	error += "\n\nЗапрос:\n" + strQuery + "\n\nBinds" + ( binds.isEmpty() ? " are empty" : ":\n" );
	for(auto &bind:binds)
		error += "\n" + bind;
	return error;
}

void MyQSqlDatabase::ShowErrorForQuery(QString generatedErrorText)
{
	auto errorSplitted = generatedErrorText.splitRef('\n');
	for(auto &error:errorSplitted)
		qdbg << error;
	Error(generatedErrorText);
}

void MyQSqlDatabase::ShowErrorForQuery(QString error, const QString &strQuery, const QStringPairVector &binds)
{
	error = GenErrorText(error, strQuery, binds);
	ShowErrorForQuery(error);
}

void MyQSqlDatabase::MakeBackupBase(bool showSuccessMsg)
{
	if(!QDir().mkpath(baseDataMain.pathBackup))
	{ Error("MakeBackubBase Can't mkpath " + baseDataMain.pathBackup); return; }
	MyQFileDir::RemoveOldFiles(baseDataMain.pathBackup,99);
	QString backupFile = baseDataMain.pathBackup + "/"
	        + QDateTime::currentDateTime().toString("yyyy.MM.dd hh-mm-ss-zzz") + " "
	        + QFileInfo(baseDataMain.baseFilePathName).fileName();
	if(!QFile::copy(baseDataMain.baseFilePathName, backupFile))
	{
		Error("MakeBackubBase Can't copy base to " + baseDataMain.pathBackup);
		return;
	}
	if(showSuccessMsg) Log("резервная копия создана\n\n" + backupFile);
}

#endif
