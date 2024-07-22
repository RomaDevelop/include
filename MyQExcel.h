#ifndef MYQEXCEL_H
#define MYQEXCEL_H

#include <vector>

#include <QAxObject>
#include <QTextBrowser>

#include "MyQShortings.h"

#include "logs.h"

class IExcelSheet
{
public:
	virtual ~IExcelSheet() = default;

	virtual QString Name() const = 0;
	virtual QString Cell(int col, int row) const = 0;
	virtual void SetCell(int col, int row, const QString &value) = 0;
	virtual void AllRows(std::vector<QStringList> &rows) const = 0;
	virtual QString AllRows() const = 0;

	static QString EndCell() { return "[endCell]"; }
	static QString EndRow()	{ return "[endRow]"; }
	static QString EndSheetName(){ return "[endSheetName]"; }
	static QString EndSheet(){ return "[endSheet]"; }
};

class ExcelSheet: public IExcelSheet
{
	QAxObject* sh;

public:
	ExcelSheet(QAxObject* sheet): sh{sheet} {}
	void Init(QAxObject* sheet) { sh = sheet; }

	virtual ~ExcelSheet() override = default;

	QString Name() const override { return sh->property("Name").toString(); }
	QString Cell(int col, int row) const override { return sh->querySubObject("Cells(int,int)", row, col)->property("Value").toString(); }
	void SetCell(int col, int row, const QString &value) override { sh->querySubObject("Cells(int,int)", col, row)->setProperty("Value",value); }
	void AllRows(std::vector<QStringList> &rows) const override;
	QString AllRows() const override;
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
	QString Cell(int col, int row) const override;
	void SetCell(int col, int row, const QString &value) override { rows[row-1][col-1] = value; }
	void AllRows(std::vector<QStringList> &rows_) const override { rows_ = rows; }
	QString AllRows() const override;
};

class IExcelWorkbook: public LogedClass
{
public:
	virtual ~IExcelWorkbook() = default;

	virtual IExcelSheet* Sheet(int number) const = 0;
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
	ExcelWorkbook() = default;
	ExcelWorkbook(QString file) { Open(file); }
	~ExcelWorkbook() override { if(opened) CloseNoSave(); }
	bool Open(QString file);
	void CloseNoSave();
	IExcelSheet* Sheet(int number) const override { return sheets[number-1]; }
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
