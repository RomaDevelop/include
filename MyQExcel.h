#ifndef MYQEXCEL_H
#define MYQEXCEL_H

#include <vector>

#include <QDebug>
#include <QAxObject>
#include <QTextBrowser>

#include "MyQShortings.h"

class IExcelSheet
{
public:
	virtual ~IExcelSheet() = default;

	virtual QString Name() const = 0;
	/// col, row нумерация с 1
	virtual QString Cell(int col, int row) const = 0;
	/// col, row нумерация с 1
	virtual void SetCell(int col, int row, const QString &value) = 0;
	virtual void AllRows(std::vector<QStringList> &rows) const = 0;
	virtual QString AllRows() const = 0;

	virtual void ClearContentsSheet() = 0;

	static QString EndCell() { return "[endCell]"; }
	static QString EndRow()	{ return "[endRow]"; }
	static QString EndSheetName(){ return "[endSheetName]"; }
	static QString EndSheet(){ return "[endSheet]"; }
};

class ExcelSheet: public IExcelSheet
{
	QAxObject* sheet;

public:
	ExcelSheet(QAxObject* sheet): sheet{sheet} {}
	void Init(QAxObject* sheet) { this->sheet = sheet; }

	virtual ~ExcelSheet() override = default;

	virtual QString Name() const override { return sheet->property("Name").toString(); }
	/// col, row нумерация с 1
	virtual QString Cell(int col, int row) const override { return sheet->querySubObject("Cells(int,int)", row, col)->property("Value").toString(); }
	/// col, row нумерация с 1
	virtual void SetCell(int col, int row, const QString &value) override { sheet->querySubObject("Cells(int,int)", row, col)->setProperty("Value",value); }
	virtual void AllRows(std::vector<QStringList> &rows) const override;
	virtual QString AllRows() const override;

	void ClearContentsSheet() override;
};

class ExcelQStringSheet: public IExcelSheet
{
	QString name;
	std::vector<QStringList> rows;

public:
	ExcelQStringSheet(const QString &sheet) { Init(sheet); }
	void Init(const QString &sheet);

	virtual ~ExcelQStringSheet() override { };

	QString Name() const override { return name; }
	/// col, row нумерация с 1
	QString Cell(int col, int row) const override;
	/// col, row нумерация с 1
	void SetCell(int col, int row, const QString &value) override { rows[row-1][col-1] = value; }
	void AllRows(std::vector<QStringList> &rows_) const override { rows_ = rows; }
	QString AllRows() const override;
};

class IExcelWorkbook
{
public:
	virtual ~IExcelWorkbook() = default;

	virtual IExcelSheet* Sheet(int index) const = 0;
	virtual int SheetCount() const = 0;
	virtual QString ToStr() const = 0;
};

class ExcelWorkbook: public IExcelWorkbook
{
	QAxObject* excel {nullptr};
	QAxObject* workbooks {nullptr};
	QAxObject* wb {nullptr};
	bool opened{false};
	bool readOnly{false};
	std::vector<IExcelSheet*> sheets;

public:
	ExcelWorkbook();
	ExcelWorkbook(QString file);
	~ExcelWorkbook() override;
	bool Open(QString file);
	void Save();
	void CloseNoSave();
	IExcelSheet* Sheet(int index) const override;
	int SheetCount() const override { return wb->querySubObject("Worksheets")->property("Count").toInt(); }
	QString ToStr() const override ;
};

class ExcelQStringWorkbook: public IExcelWorkbook
{
	std::vector<ExcelQStringSheet> sheets;
	std::vector<IExcelSheet*> sheetsPt;

public:
	IExcelSheet* Sheet(int number) const override;
	int SheetCount() const override { return sheets.size(); }
	QString ToStr() const override;
	void FromStr(const QString &workBookAsStr);
};

class MyQExcelHasher
{
	IExcelWorkbook *wbPt {nullptr};
	QString *labelForMsgs = nullptr;

	static QString GetFileName(int fileIndex);

	int LoadFromHashedFile(QString fileName);

public:
	~MyQExcelHasher() { if(wbPt) delete wbPt; }

	IExcelWorkbook* Get() { return wbPt; }

	int InitFile(QString directory, QString fileName);

	void Log(const QString &str)
	{
		qDebug() << str;
		if(labelForMsgs) *labelForMsgs = str;
	}

	enum errors {noError, unknown, cantCrDir, cantOpenExcelFile, cantOpenFileHashes, cantOpenFileHash, cantOpenFileNew, cantOpenNewFileHash};
	static QString ErrCodeToStr(int errCode);
};


#endif // MYQEXCEL_H
