#include <QCryptographicHash>
#include <QFile>
#include <QDir>

#include "MyQByteArray.h"

#include "MyQExcel.h"

void ExcelSheet::AllRows(std::vector<QStringList> &rows) const
{
	int countRows = sh->querySubObject("UsedRange")->querySubObject("Rows")->property("Count").toInt();
	int countCols = sh->querySubObject("UsedRange")->querySubObject("Columns")->property("Count").toInt();

	rows.resize(countRows);

	for(int r=1; r<=countRows; r++)
	{
		rows[r-1].clear();
		for(int c=1; c<=countCols; c++)
		{
			rows[r-1] += Cell(c,r);
		}
	}
}

QString ExcelSheet::AllRows() const
{
	int countRows = sh->querySubObject("UsedRange")->querySubObject("Rows")->property("Count").toInt();
	int countCols = sh->querySubObject("UsedRange")->querySubObject("Columns")->property("Count").toInt();

	QString ret;

	for(int r=1; r<=countRows; r++)
	{
		for(int c=1; c<=countCols; c++)
		{
			ret += Cell(c,r) + EndCell();
		}
		ret += EndRow();
	}

	return ret;
}

bool ExcelWorkbook::Open(QString file)
{
	if(opened) CloseNoSave();
	if(QFile(file).exists())
	{
		excel = new QAxObject("Excel.Application", 0);
		workbooks = excel->querySubObject("Workbooks");
		wb = workbooks->querySubObject("Open(const QString&)", file);

		if(!sheets.empty())
		{
			for(auto sh:sheets) delete sh;
			sheets.clear();
		}

		int shC = SheetCount();
		for(int i=1; i<=shC; i++)
		{
			ExcelSheet *sh = new ExcelSheet(wb->querySubObject("Worksheets")->querySubObject("Item(int)", i));
			sheets.push_back(sh);
		}
		opened = true;
		return true;
	}
	else Error("Open: file "+file+" not exists!");
	return false;
}

void ExcelWorkbook::CloseNoSave()
{
	if(opened)
	{
		wb->dynamicCall("Close()");
		excel->dynamicCall("Quit()");
		delete excel;
		excel = nullptr;
		workbooks = nullptr;
		wb = nullptr;
		delete excel;
		opened = false;
	}
}

QString ExcelWorkbook::ToStr() const
{
	QString ret;
	int shC = SheetCount();
	for(int shi = 1; shi<=shC; shi++)
	{
		IExcelSheet *sheet = Sheet(shi);
		ret += sheet->Name() +  IExcelSheet::EndSheetName();
		ret += sheet->AllRows() + IExcelSheet::EndSheet();
	}

	return ret;
}

void ExcelQStringSheet::Init(const QString &sheet)
{
	rows.clear();
	name = sheet.left(sheet.indexOf(EndSheetName()));
	QStringList rows_ = sheet.split(EndRow());
	if(rows_.size())
	{
		rows_[0].remove(0,rows_[0].indexOf(EndSheetName()) + EndSheetName().length());
		for(auto &r:rows_)
		{
			rows.push_back(r.split(EndCell()));
		}
	}
}

QString ExcelQStringSheet::Cell(int col, int row) const
{
	if(row > (int)rows.size() || col > rows[row-1].size()) Logs::ErrorSt("QString Cell(col=" + QSn(col) + ", row=" + QSn(row) + ")  out of boud");
	return rows[row-1][col-1];
}

QString ExcelQStringSheet::AllRows() const
{
	QString ret;

	int countRows = rows.size();
	if(rows.size())
	{
		int countCols = rows[0].size();

		for(int r=0; r<countRows; r++)
		{
			for(int c=0; c<countCols; c++)
			{
				ret += rows[r][c] + EndCell();
			}
			ret += EndRow();
		}
	}
	return ret;
}

IExcelSheet* ExcelQStringWorkbook::Sheet(int number) const
{
	if(number > (int)sheetsPt.size()) Logs::ErrorSt("IExcelSheet* Sheet(number=" + QSn(number) + ") out of boud");
	return sheetsPt[number-1];
}

QString ExcelQStringWorkbook::ToStr() const
{
	QString ret;
	for(auto &sh:sheets)
	{
		ret += sh.Name() +  IExcelSheet::EndSheetName();
		ret += sh.AllRows() + IExcelSheet::EndSheet();
	}
	return ret;
}
void ExcelQStringWorkbook::FromStr(const QString &workBookAsStr)
{
	QStringList sheets_ { workBookAsStr.split(IExcelSheet::EndSheet()) };
	for(int i=0; i<sheets_.size(); i++)
		sheets.push_back(sheets_[i]);

	for(uint i=0; i<sheets.size(); i++)
		sheetsPt.push_back(&sheets[i]);
}

QString MyQExcelHasher::ErrCodeToStr(int errCode)
{
	if(errCode == noError) return "No error";
	if(errCode == cantCrDir) return "Ошибка создания директории для файлов хеширования";
	if(errCode == cantOpenExcelFile) return "cant Open Excel File";
	if(errCode == cantOpenFileHashes) return "Can't open file /_hashes.txt";
	if(errCode == cantOpenFileHash) return "Can't open file hash";
	if(errCode == cantOpenFileNew) return "Can't open new file";
	if(errCode == cantOpenNewFileHash) return "Can't open new file hash";
	return "Wrong errCode "+QSn(errCode);
}

QString MyQExcelHasher::GetFileName(int fileIndex)
{
	QString indStr = QSn(fileIndex);
	while(indStr.length()<6) indStr = "0" + indStr;
	return "file_" + indStr + ".txt";
}

int MyQExcelHasher::LoadFromHashedFile(QString fileName)
{
	Log("Load from hashed file " + QFileInfo(fileName).fileName());
	QFile fileData(fileName);

	if(fileData.open(QFile::ReadOnly))
	{
		QString wbStr = fileData.readAll();
		fileData.close();

		ExcelQStringWorkbook *swb = new ExcelQStringWorkbook;
		wbPt = swb;
		swb->FromStr(wbStr);
	}
	else return cantOpenFileHash;

	return noError;
}

int MyQExcelHasher::InitFile(QString directory, QString fileName)
{
//	labelForMsgs = &labelForMsgs_;
	QDir dirHashedFiles(directory + "/hashed_files");
	if(!dirHashedFiles.exists()) dirHashedFiles.mkdir(dirHashedFiles.path());
	if(!dirHashedFiles.exists()) return cantCrDir;

	QCryptographicHash curHash(QCryptographicHash::Md5);
	QFile file(fileName);
	file.open(QFile::ReadOnly);
	curHash.addData(&file);
	file.close();

	QStringList hashesFileContent;
	QFile file_hashes(dirHashedFiles.path() + "/_hashes.txt");
	if(file_hashes.exists())
	{
		if(file_hashes.open(QFile::ReadWrite))
		{
			QString hashes = file_hashes.readAll();
			file_hashes.close();

			hashesFileContent = hashes.split("[endHashLine]\r\n", QString::SkipEmptyParts);
		}
		else return cantOpenFileHashes;
	}
	else { /*if file_hashes doesn't exists it's not error, it will be created later*/ }

	QString findFile;
	int fileIndex = 0;
	for(auto &hashFileLine:hashesFileContent)
	{
		QStringList hash1LineElements {hashFileLine.split("[endHashElem]")};
		QString hash = hash1LineElements[0];
		fileIndex = hash1LineElements[1].toInt();
		if(hash == MyQBA::QByteArrToStr(curHash.result())) findFile = GetFileName(fileIndex);
	}

	if(findFile != "")
	{
		return LoadFromHashedFile(dirHashedFiles.path() + "/" + findFile);
	}
	else
	{
		fileIndex++;
		QString newFile = GetFileName(fileIndex);
		Log("Hash not found, write new hash " + newFile);

		ExcelWorkbook *ewb = new ExcelWorkbook;
		if(ewb->Open(fileName))
		{
			if(file_hashes.open(QFile::Append))
			{
				QString curHashStr = MyQBA::QByteArrToStr(curHash.result());
				file_hashes.write((curHashStr+"[endHashElem]").toUtf8());
				file_hashes.write(QString(QSn(fileIndex)+"[endHashElem]").toUtf8());
				file_hashes.write(QString(fileName+"[endHashElem][endHashLine]\r\n").toUtf8());
				file_hashes.close();

				QFile fileData(dirHashedFiles.path() + "/" + newFile);
				if(fileData.open(QFile::WriteOnly))
				{
					fileData.write(ewb->ToStr().toUtf8());
					fileData.close();

					ewb->CloseNoSave();
					delete ewb;

					return LoadFromHashedFile(dirHashedFiles.path() + "/" + newFile);
				}
				else return cantOpenNewFileHash;
			}
			else return cantOpenFileHashes;
		}
		else return cantOpenExcelFile;
	}

	return noError;
}
