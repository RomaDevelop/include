#ifndef CODE_H
#define CODE_H

#include <string_view>

#include <QString>
#include <QStringList>
#include <QDebug>

#include "CodeMarkers.h"
#include "MyQShortings.h"

//#include "logs.h"

namespace CodeKeyWords
{
	const char commandSplitter = ';';
	const char wordsSplitter = ' ';

	const char quatsSymbol1 = '\'';
	const char quatsSymbol2 = '\"';
	const int quatsSymbolLength = 1;

	const QString true_str = "true";
	const QString false_str = "false";

	const QString commentMarker = "//";

	const QChar blockOpener = '{';
	const QChar blockCloser = '}';

	constexpr std::string_view binCodePrefix = "0b";
	constexpr std::string_view hexCodePrefix = "0x";
	const int encodingPrefixSize = binCodePrefix.size();
	static_assert (binCodePrefix.size() == hexCodePrefix.size(), "encodingPrefixSizes are different");

	const QString hexSymbols = "01234567890abcdef";

	const QString idFilter = "ID:";
	const QString dataFilter = "Data:";

	const QString emulatorStr = "Эмулятор";
	const QString servisStr = "Сервис";
	const int codeUndefined = 0;
	const int emulatorInt = 1;
	const int servisInt = 2;
	QString TypeToStr(int type);

	const QString dataOperand = "Data";
	const QString thisParam = "thisParam";
	const QString thisWorker = "thisWorker";
	const QString thisVariable = "thisVariable";

	const QString obrabotchik = "Обработчик";
	const QString transform = "Transform";
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
	inline static bool IsItQuatsSybol(const T& symbol);
	static bool ContainsSplitter(const QString &str);
	static QString AddQuates(const QString &text, char quates = CodeKeyWords::quatsSymbol1);
};

struct AllIndexes
{
	std::vector<std::vector<int>> indexes;
};
struct AllIndexesOld
{
	std::vector<int> first;
	std::vector<int> secnd;
};

struct Statement
{
	QString header;
	QStringList blockInstructions;
	Statement() = default;
	Statement(QString header, QStringList blockInstructions): header{header}, blockInstructions{blockInstructions} {}

	static QString PrintStatements(std::vector<Statement> statements);
};
struct Statement2
{
	bool singleInstruction = false;
	QString header;
	std::vector<Statement2> nestedStatements;
	Statement2() = default;
	explicit Statement2(QString header, std::vector<Statement2> nestedStatements): header{header}, nestedStatements{nestedStatements} {}
	explicit Statement2(QString singleInstruction_)
	{
		singleInstruction = true;
		header = std::move(singleInstruction_);
	}
	explicit Statement2(QString header, QStringList blockSingleInstructions): header{header} {
		for(auto &instruction:blockSingleInstructions)
		{
			nestedStatements.emplace_back(Statement2(std::move(instruction)));
		}
	}

	static QString PrintStatements(std::vector<Statement2> statements, const QString &indent = {});
};

class Code
{
public:
	static void Normalize(QString &text);
	static QStringList TextToCommands(const QString &text); // внутри вызывается Normalize; гарантируется отсутсвие пустых команд в return
	static QStringList CommandToWords(const QString &command);

	static std::vector<Statement> TextToStatements(const QString &text);  // внутри вызывается Normalize
	static std::vector<Statement2> TextToStatements2(const QString &text, int nestedBlockParsingStart = -1, int *nestedBlockFinish = {});

	static QString GetFirstWord(const QString &text);
	static QString GetPrevWord(const QString &text, int charIndexInText);
	static QString GetNextWord(const QString &text, int charIndexInText);

	static std::vector<int> DecodeStrNumbers(const QString &strNumbers, bool printErrorIfEmpty);

	static QStringList GetTextsInSquareBrackets(const QString &text);
	static AllIndexes GetAllIndexes(QString operand);

	static void RemoveEmulatorServis(QStringList &commands, int codeCurrent);
	static QString GetInitialisationStr(const QString &command, bool printErrorIfNoInitialisation);

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

	static bool TestGetPrevWord();
	static bool TestNormalize();
	static bool TestTextToCommands();
	static bool TestTextToStatements();
	static bool TestTextToStatements2();
};

struct CodeLogs
{
private:
	static std::function<void(const QString& logText)> logFucnction;
	static std::function<void(const QString& errorText)> errorLogFucnction;

public:
	static void SetLogFunction(std::function<void(const QString& logText)> &&logFucnction);
	static void SetErrorLogFunction(std::function<void(const QString& errorText)> &&errorLogFucnction);

	static void Log(const QString& logText) { logFucnction(logText); }
	static void Error(const QString& errorText) { errorLogFucnction(errorText); }

	inline static int errorsTestCount;
	inline static std::list<QString> errorsTestList;
	static void ActivateTestMode(bool active, bool callNativeLogInTestMode);
};

#endif // CODE_H
