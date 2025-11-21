#ifndef CODE_H
#define CODE_H

#include <string_view>
#include <set>

#include <QString>
#include <QStringList>
#include <QDebug>

#include "CodeMarkers.h"
#include "MyQShortings.h"
#include "declare_struct.h"

//#include "logs.h"

namespace CodeKeyWords
{
	const char commandSplitter = ';';
	const char colon = ':';

	const char wordsSplitter = ' ';

	const char quatsSymbol1 = '\'';
	const char quatsSymbol2 = '\"';
	const int quatsSymbolLength = 1;

	const QString true_str = "true";
	const QString false_str = "false";

	const QString commentMarker = "//";

	const QChar dot = '.';
	const QChar comma = ',';

	const QChar blockOpener = '{';
	const QChar blockCloser = '}';

	const QChar assign = '=';

	constexpr std::string_view binCodePrefix = "0b";
	constexpr std::string_view hexCodePrefix = "0x";
	const int encodingPrefixSize = binCodePrefix.size();
	static_assert (binCodePrefix.size() == hexCodePrefix.size(), "encodingPrefixSizes are different");

	namespace Encodings
	{
		inline const char * bin = "bin";
		inline const char * dec = "dec";
		inline const char * hex = "hex";
	}

	const std::set<QChar> hexSymbols { '0','1','2','3','4','5','6','7','8','9','0','a','b','c','d','e','f' };
};

namespace CodeKeyWordsAdditional
{
	const QString trueOrFalse = CodeKeyWords::true_str + "|" + CodeKeyWords::false_str;
};

struct TextConstant
{
	struct Result { bool checkResult = false; QString value; };

	static QString GetTextConstVal(const QString &txtConst, bool doCheck);
	static Result GetTextConstVal(const QString &txtConst);

	static bool IsItTextConstant(const QString &text, bool printLog);
	template<typename T>
	static bool IsItQuateSybol(const T& symbol)
	{
		return (symbol == CodeKeyWords::quatsSymbol1 || symbol == CodeKeyWords::quatsSymbol2);
	}
	static bool ContainsQuate(const QString &str);
	static QString AddQuates(const QString &text, char quates = CodeKeyWords::quatsSymbol1);
	static void RemoveQuates(QString &text);
};

///\brief Тип данных для представления множественных индексов
/// var[...][...]...
using AllIndexes = std::vector<std::vector<int>>;

struct Statement
{
	using StatementOrQString = std::variant<Statement, QString>;
	using VectorStatementOrQString = std::vector<StatementOrQString>;

	QString header;
	VectorStatementOrQString nestedStatements;

	Statement() = default;
	explicit Statement(QString header, VectorStatementOrQString nestedStatements);
	explicit Statement(QString singleInstruction_);
	explicit Statement(QString header, QStringList blockSingleInstructions);

	static QString PrintStatements(std::vector<Statement> statements, const QString &indent = {});
	QString PrintStatement(const QString &indent = {}) const;
	void ForEach(const std::function<void(std::pair<Statement*,QString*>)> &function, bool &statementExitFlag, bool &returnFlag);
	void Remove_child_if(const std::function<bool(std::pair<Statement*,QString*>)> &condition);

	static bool CmpStatement(const Statement &lhs, const Statement &rhs, QString *resultDetails);
};

class Code
{
public:
	///\brief Нормализация текста
	/// символы переноса строки и табуляции заменяются пробелами
	/// добавляются пробелы до и после опереаторов ( a+b -> a + b )
	/// лишние пробелы удаляются ( a  = b -> a = b )
	static void Normalize(QString &text);
	static QStringList TextToCommands(const QString &text);	// внутри вызывается Normalize; гарантируется отсутсвие пустых команд в return
	static QStringList CommandToWords(const QString &command, bool canContainCommandSplitter = false);
	// гарантируется возвращение непустого списка
	// если передана пустая command вернет список с одним словом-индикатором ошибки

	///\brief Парсинг текста в выражения
	/// при парсинге текст нормализуется (вызывается Normalize)
	/// параметры nestedBlockOpener и nestedBlockCloser предназначены для внутреннего рекурсивного вызова
	/// при внешнем вызове nestedBlockOpener и nestedBlockCloser не используются, оставлять значения по-умолчанию
	static Statement TextToStatements(const QString &text, int nestedBlockOpener = -1, int *nestedBlockCloser = {});

	static QString GetFirstWord(const QString &text);
	static QString GetPrevWord(const QString &text, int charIndexInText);
	static QString GetNextWord(const QString &text, int charIndexInText);

	/// извлекает содержимое первого блока (блок может содержать вложенные блоки)
	/// всё что до него и сам блок (включая {}) из words будут удалены
	static QStringList TakeBlock(QStringList &words);

	static std::vector<int> DecodeStrNumbers(const QString &strNumbers, bool printErrorIfEmpty);

	static QStringList GetTextsInSquareBrackets(const QString &text);
	static AllIndexes GetAllIndexes(const QString &text);

	declare_struct_4_fields_move(InitParsed, QString, error, QStringList, wordsBefore, QStringList, wordsInit, QStringList, wordsAfter);
	static InitParsed ParseInitialisation(QStringList words);

	static bool IsInteger(const QString &str);
	static bool IsUnsigned(const QString &str);
	static bool IsFloating(const QString &str);
	static bool IsBinCode(const QString &str);
	static bool IsHexCode(const QString &str);
	static bool IsNumber(const QString &str);
};

struct CodeTests
{
	static bool DoCodeTests();

	static bool TestGetTextsInSquareBrackets();

	static bool TestGetPrevWord();
	static bool TestNormalize();
	static bool TestTextToCommands();
	static QStringList TestTextToStatements();
};

struct LogFunction
{
	std::function<void(const QString& text)> m_function;

	int countInTestMode;
	std::list<QString> textsInTestMode;
	std::function<void(const QString& text)> fucnctionBackup;

	explicit LogFunction(std::function<void(const QString& text)> a_function):
		m_function{ a_function ? std::move(a_function) : [](const QString& text){ qdbg << text; } } {}

	void ActivateTestMode(bool active);

	QString GetTexts(int count);
};

struct CodeLogs
{
public:
	static LogFunction log;
	static LogFunction warning;
	static LogFunction error;

	static void SetLogFunction(std::function<void(const QString& logText)> &&logFucnction);
	static void SetWarningFunction(std::function<void(const QString& warningText)> &&warningFucnction);
	static void SetErrorFunction(std::function<void(const QString& errorText)> &&errorLogFucnction);

	static void Log(const QString& logText);
	static void Warning(const QString& text);
	static void Error(const QString& errorText);

	static void ActivateTestMode(bool active);
};

#endif // CODE_H
