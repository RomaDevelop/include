#include "Code.h"

#include <QMessageBox>

namespace ckw = CodeKeyWords;

void Code::Normalize(QString &text)
{
	if(0) CodeMarkers::need_optimisation("избавиться от лишних проходов по text");
	auto quats = false;
	QChar currQuats = ckw::quatsSymbol1;
	// replace '\n' '\r' '\t' by spaces
	// add spaces before and after operators
	for(int i = text.length()-1; i>=0; i--)
	{
		if(!quats && TextConstant::IsItQuatsSybol(text[i])) { quats = true; currQuats = text[i]; continue; }
		if(text[i] == currQuats && quats) { quats = false; continue; }

		if(!quats)
		{
			if(text[i] == '\n' || text[i] == '\r' || text[i] == '\t')
			{
				text[i] = ' ';
				continue;
			}

			if(text[i] == ',' || text[i] == '.'
						  || text[i] == '(' || text[i] == ')'
						  || text[i] == '[' || text[i] == ']'
						  || text[i] == '{' || text[i] == '}'
						  || text[i] == '=' || text[i] == '+' || text[i] == '-' || text[i] == '*' || text[i] == '/'
						  || text[i] == '~'
						  || text[i] == '>' || text[i] == '<' || text[i] == '!')
			{
				text.insert(i+1,' ');
				text.insert(i,' ');
				continue;
			}
		}
	}

	// remove spaces at the begining and in the end
	// это должно быть тут, постому что на этапе выше \n заменяются пробелами и оказываются в начале
	while(text.length() && text[0] == ' ') text.remove(0,1);
	while(text.length() && text[text.length()-1] == ' ') text.remove(text.length()-1,1);

	//remove double spaces
	quats = false;
	for(int i = text.length()-1; i>=0; i--)
	{
		if(!quats && TextConstant::IsItQuatsSybol(text[i])) { quats = true; currQuats = text[i]; continue; }
		if(text[i] == currQuats && quats) { quats = false; continue; }

		if(!quats && (i > 0 && text[i] == ' ' && text[i-1] == ' ')) { text.remove(i,1); continue; }
	}

	// remove spaces betwieen two-sybolic operands
	quats = false;
	for(int i = text.length()-1; i>=0; i--)
	{
		if(!quats && TextConstant::IsItQuatsSybol(text[i])) { quats = true; currQuats = text[i]; continue; }
		if(text[i] == currQuats && quats) { quats = false; continue; }

		if(!quats && text[i] == ' ')
		{
			if(i == 0 || i == text.length()-1)
			{
				Logs::ErrorSt("space is first or last symbol in text ["+text+"]");
			}
			else
			{
				if(text[i-1] == '=' && text[i+1] == '=') { text.remove(i,1); i--; continue; }
				if(text[i-1] == '!' && text[i+1] == '=') { text.remove(i,1); i--; continue; }
				if(text[i-1] == '>' && text[i+1] == '=') { text.remove(i,1); i--; continue; }
				if(text[i-1] == '<' && text[i+1] == '=') { text.remove(i,1); i--; continue; }

				if(text[i-1] == '+' && text[i+1] == '=') { text.remove(i,1); i--; continue; }
				if(text[i-1] == '-' && text[i+1] == '=') { text.remove(i,1); i--; continue; }
				if(text[i-1] == '*' && text[i+1] == '=') { text.remove(i,1); i--; continue; }
				if(text[i-1] == '/' && text[i+1] == '=') { text.remove(i,1); i--; continue; }

				if(text[i-1] == '/' && text[i+1] == '/') { text.remove(i,1); i--; continue; } // двойной слеш (комментарий)
			}
		}
	}

	// remove spaces in floating numbers
	quats = false;
	for(int i=0; i<text.size(); i++)
	{
		if(!quats && TextConstant::IsItQuatsSybol(text[i])) { quats = true; currQuats = text[i]; continue; }
		if(text[i] == currQuats && quats) { quats = false; continue; }

		if(!quats && text[i] == '.')
		{
			auto prevWord = GetPrevWord(text,i), nextWord = GetNextWord(text,i);
			if(prevWord.isEmpty() || nextWord.isEmpty()) { Logs::ErrorSt(". is first or last word in text ["+text+"]"); continue; }

			if(IsNumber(std::move(prevWord)) && IsNumber(std::move(nextWord)))
			{
				text.remove(i+1,1);
				text.remove(i-1,1);
			}
		}
	}
}

QStringList Code::TextToCommands(const QString &text)
{
	QStringList commands;
	bool quats = false;
	bool commented = false;
	QChar currQuats = CodeKeyWords::quatsSymbol1;
	QString command;
	int sizeText = text.size();
	for(int i = 0; i<sizeText; i++)
	{
		// логика игнорирования комментария
		if(commented)  // если сейчас комментарий
		{
			if(text[i] == '\n') // если встретили перенос на сл.строку - прекращаем игнорировать
				commented = false;
			else
				continue;  // продолжаем игнорировать
		}
		// определяем, что встретили маркер закомментированного
		else if(!quats && text[i] == CodeKeyWords::commentMarker[0] && i+1 < sizeText && text[i+1] == CodeKeyWords::commentMarker[1])
		{
			commented = true;
			continue;
		}
		// конец логики игнорирования комментария

		// завершение команды
		if(!quats && text[i] == CodeKeyWords::commandSplitter)
		{
			commands.push_back(command);
			command.clear();
			continue;
		}
		command += text[i];

		if(!quats && TextConstant::IsItQuatsSybol(text[i])) { quats = true; currQuats = text[i]; continue; }
		if(text[i] == currQuats && quats) { quats = false; continue; }
	}
	if(command.size())
	{
		//Logs::WarningSt("missing ending symbol; code:\n\t" + QString(text).replace('\t', "\t\t"));
		commands.push_back(std::move(command));
	}

	int size = commands.size();
	for(int i=size-1; i>=0; i--)
	{
		Normalize(commands[i]);
		if(commands[i] == "") { commands.removeAt(i); continue;

//			QString textToLog = "empty command found in text: ";
//			textToLog += text.size() < 500 ? text : text.left(500) + "...";
//			textToLog += "\n\ncommands:\n";
//			textToLog += commands.join("\n").remove('\r').replace("\n\n", "\n");
//			Logs::WarningSt(textToLog);
			/// вывод предупреждений о пустых командах отключен из-за ложных срабатываний
			/// избежать ложных срабатываний затруднительно, поскольку пустая команда может легально образоваться от:
			///		комментария после последней ;
			///		от пробелов после последней ;
			///		от \n или др. спец.символов после последней ;
			///		возможно что-то ещё
			///			можно предусмотреть эти случаи
		}
	}
	return commands;
}

QStringList Code::CommandToWords(const QString &command)
{
	QStringList retWords;
	int size = command.size();
	QString word;
	bool quatsNow = false;
	QChar currentQuats = ckw::quatsSymbol1;
	for(int i=0; i<size; i++)
	{
		if(!quatsNow && TextConstant::IsItQuatsSybol(command[i]))
		{
			word += command[i];
			currentQuats = command[i];
			quatsNow = true;
			continue;
		}
		if(quatsNow  && command[i] == currentQuats)
		{
			if(i != 0 && command[i-1] == '\\' )
				continue;

			word += command[i];
			quatsNow = false;
			continue;
		}

		if(!quatsNow && command[i] == CodeKeyWords::commandSplitter)
			Logs::ErrorSt("Error!!! Compiler::CommandToWords command [" + command + "] contains ;");

		if(!quatsNow && command[i] == CodeKeyWords::wordsSplitter) { retWords += word; word = ""; continue; }
		word += command[i];
	}
	if(word != "") retWords += word;
	if(quatsNow) Logs::ErrorSt("Code::CommandToWords not closed text constant in command [" + command + "]");

	return retWords;
}

std::vector<Statement> Code::TextToStatements(const QString &text)
{
	std::vector<Statement> statements;
	Statement* currentStatement = nullptr;
	bool quats = false;
	bool block = false;
	bool commented = false;
	QChar currQuats = CodeKeyWords::quatsSymbol1;
	QString current;
	int sizeText = text.size();
	for(int i = 0; i<sizeText; i++)
	{
		// логика игнорирования комментария
		if(commented)  // если сейчас комментарий
		{
			if(text[i] == '\n') // если встретили перенос на сл.строку - прекращаем игнорировать
				commented = false;
			else
				continue;  // продолжаем игнорировать
		}
		// определяем, что встретили маркер закомментированного
		else if(!quats && text[i] == CodeKeyWords::commentMarker[0] && i+1 < sizeText && text[i+1] == CodeKeyWords::commentMarker[1])
		{
			commented = true;
			continue;
		}
		// конец логики игнорирования комментария

		// начало блока
		if(!quats && text[i] == CodeKeyWords::blockOpener)
		{
			if(block) { Logs::ErrorSt("nested blocks are forbidden, text: " + text); current.clear(); break; }

			block = true;
			Normalize(current);
			currentStatement = &statements.emplace_back();
			currentStatement->header = std::move(current);
			current.clear();
			continue;
		}

		// завершение одиночной команды без блока
		if(!quats && !block && text[i] == CodeKeyWords::commandSplitter)
		{
			Normalize(current);
			statements.emplace_back();
			statements.back().blockInstructions += std::move(current);
			current.clear();
			continue;
		}

		// завершение блока
		if(!quats && block && text[i] == CodeKeyWords::blockCloser)
		{
			currentStatement->blockInstructions = Code::TextToCommands(current);
			currentStatement = nullptr;
			current.clear();

			block = false;
			continue;
		}

		current += text[i];

		if(!quats && TextConstant::IsItQuatsSybol(text[i])) { quats = true; currQuats = text[i]; continue; }
		if(text[i] == currQuats && quats) { quats = false; continue; }
	}

	if(current.size())
	{
		if(block) Logs::ErrorSt("missing ending symbol; text: " + text);
		else
		{
			Normalize(current);
			if(!current.isEmpty())
			{
				statements.emplace_back();
				statements.back().blockInstructions += std::move(current);
			}
		}
	}

	return statements;
}

QString Code::GetFirstWord(const QString &text)
{
	return text.left(text.indexOf(CodeKeyWords::wordsSplitter));
}

QString Code::GetPrevWord(const QString & text, int charIndexInText)
{
	if(charIndexInText < 0 || charIndexInText >= text.length()) return "";

	int closingIndex = charIndexInText;
	if(text[charIndexInText] != ' ')
	{
		while(text[closingIndex] != CodeKeyWords::wordsSplitter)
		{
			closingIndex--;
			if(closingIndex == -1) return "";
		}
	}

	while(closingIndex > 0 && text[closingIndex-1] == CodeKeyWords::wordsSplitter)
	{
		closingIndex--;
	}
	if(closingIndex == 0) return "";

	int openingIndex = closingIndex-1;
	while(openingIndex >= 0 && text[openingIndex] != CodeKeyWords::wordsSplitter)
	{
		openingIndex--;
	}

	return text.mid(openingIndex+1,closingIndex-openingIndex-1);
}

QString Code::GetNextWord(const QString & text, int charIndexInText)
{
	if(charIndexInText < 0 || charIndexInText >= text.length()) return "";

	int openerIndex = charIndexInText;
	if(text[charIndexInText] != ' ')// если передан индекс внутри слова(не пробел)
	{
		while(text[openerIndex] != CodeKeyWords::wordsSplitter) // двигаемся вперед пока не встретим символ пробел
		{
			openerIndex++;
			if(openerIndex >= text.length()) return "";
		}
	}

	// двигаемся вперед пока не закончатся пробелы, т.е. находим начало след слова
	while(openerIndex+1 < text.length() && text[openerIndex+1] == CodeKeyWords::wordsSplitter)
	{
		openerIndex++;
	}
	if(openerIndex == text.length()-1) return "";

	// двигаеся вперед пока идут буквы, т.е находим конец искомого слова
	int closerIndex = openerIndex+1;
	while(closerIndex < text.length() && text[closerIndex] != CodeKeyWords::wordsSplitter)
	{
		closerIndex++;
	}

	return text.mid(openerIndex+1, closerIndex-openerIndex-1);
}

QStringList Code::GetTextsInSquareBrackets(const QString &text)
{
	QStringList result;
	result.push_back(QString());
	int opSize = text.size();
	bool indexesNow = false;
	bool quats = false;
	QChar currQuats = ckw::quatsSymbol1;
	int nestedIndexes = 0;
	for(int i=0; i<opSize; i++)
	{
		if(indexesNow && text[i] == ']' && nestedIndexes == 0)
		{
			indexesNow = false;
			result.push_back(QString());
		}

		if(indexesNow) result.back() += text[i];
		if(indexesNow && text[i] == '[') nestedIndexes++;
		if(indexesNow && text[i] == ']') nestedIndexes--;

		if(!quats && TextConstant::IsItQuatsSybol(text[i]))
		{ quats = true; currQuats = text[i]; continue; }
		if(quats && text[i] == currQuats) { quats = false; continue; }

		if(!quats && text[i] == '[') indexesNow = true;
	}
	if(result.back().isEmpty()) result.removeLast();

	if(quats) Logs::ErrorSt("not closed quats in text [" + text + "]");

	if(indexesNow) Logs::ErrorSt("not closed brackets in text [" + text + "]");

	return result;

	if(0) // test for this function
	{
		QStringList tests;
		tests << "" << "'sacsd[ds]sdvsdv' ['s[]d']  [[]]   [123]  ['sdvsdvsdv[]']   ['[]sdvsdv']"
				 /*<< "[1]" << "[123]" << "[][1]" << "[1][]" << "[][][]" << "[1][2][3][4" << "[1][2][3][4]" << "[]["*/;
		for(auto &test:tests)
		{
			auto res = GetTextsInSquareBrackets(test);
			qdbg << "test" << test;
			qdbg << "content" << res.size() << res.join(" ; ");
			qdbg << " ";
		}
	}

	return result;
}

AllIndexes Code::GetAllIndexes(QString text)
{
	AllIndexes result;
	auto textsInBrackets = GetTextsInSquareBrackets(text);

	for(auto &subText:textsInBrackets)
	{
		if(!subText.isEmpty())
		{
			auto indexes = DecodeStrNumbers(subText,true);
			result.indexes.push_back(indexes);
		}
		else result.indexes.push_back({}); // если скобки пустые - добавляем пустой вектор
	}

	if(0)
	{
		QStringList tests;
		tests << "" << "[[]]   [123]  ";
		tests << "sdsdv[123]sdvsdv[][1][1,2,3,4][1-5,7-8]";
		tests << "[5-2]";
		tests << "[5-5]";
		for(auto &test:tests)
		{
			auto res = GetAllIndexes(test);
			qdbg << "test" << test;
			for(auto &indexes:res.indexes)
			{
				QString indexesStr;
				for(auto &index:indexes)
					indexesStr += QSn(index) + " ";
				qdbg << "indexes" << indexesStr;
			}
			qdbg << " ";
		}
	}

	return result;
}

void Code::RemoveEmulatorServis(QStringList & commands, int codeCurrent)
{
	for(int i=commands.size()-1; i>=0; i--)
	{
		bool startsWithEmulator = commands[i].startsWith(CodeKeyWords::emulatorStr);
		bool startsWithServis = commands[i].startsWith(CodeKeyWords::servisStr);
		if(startsWithEmulator && codeCurrent != CodeKeyWords::emulatorInt) commands.removeAt(i);
		if(startsWithServis && codeCurrent != CodeKeyWords::servisInt) commands.removeAt(i);
		if(startsWithEmulator && codeCurrent == CodeKeyWords::emulatorInt) commands[i].remove(0,CodeKeyWords::emulatorStr.size()+1);
		if(startsWithServis && codeCurrent == CodeKeyWords::servisInt) commands[i].remove(0,CodeKeyWords::servisStr.size()+1);
	}
}

QString Code::GetInitialisationStr(const QString &command, bool printErrorIfNoInitialisation)
{
	QString initialisation;
	if(command.count('{') == 1 && command.count('}') == 1)
	{
		initialisation = command;
		initialisation.remove(0,initialisation.indexOf('{')+1);
		initialisation = initialisation.left(initialisation.indexOf('}'));

		while(initialisation.size() && initialisation[initialisation.size()-1] == ' ') initialisation.chop(1);
		while(initialisation.size() && initialisation[0] == ' ') initialisation.remove(0,1);
	}
	else if(auto words = CommandToWords(command); words.count("=") == 1)
	{
		while(words.first() != "=") words.removeFirst();
		words.removeFirst();
		initialisation = words.join(' ');
	}
	else if(printErrorIfNoInitialisation)
	{
		Logs::ErrorSt("GetInitialisation wrong initialisation " + command);
		return initialisation;
	}

	if(printErrorIfNoInitialisation && initialisation.isEmpty())
		Logs::ErrorSt("No initialisation in command" + command);

	if(initialisation.startsWith('-'))
		while(initialisation.size() >= 2 && initialisation[1] == ' ') initialisation.remove(1,1);

	return initialisation;
}

bool Code::IsNumber(const QString &str)
{
	if(IsInteger(str) || IsFloating(str)) return true;
	else return false;
}

bool Code::IsInteger(const QString &str)
{
	bool ok {false};
	if(str.size())
	{
		if(str[0]=='-') str.toLongLong(&ok);
		else str.toULongLong(&ok);
	}
	return ok;
}

bool Code::IsUnsigned(const QString & str)
{
	bool ok {false};
	str.toULongLong(&ok);
	return ok;
}

bool Code::IsFloating(const QString &str)
{
	bool ok;
	str.toDouble(&ok);
	return ok;
}

bool Code::IsBinCode(const QString & str)
{
	if(str.size() >= 3)
	{
		if(str.left(2) == CodeKeyWords::binCodePrefix.data())
		{
			for(int i=CodeKeyWords::encodingPrefixSize; i<str.size(); i++)
				if(str[i] != '0' && str[i] != '1')
					return false;
			return true;
		}
	}
	return false;
}

bool Code::IsHexCode(const QString & str)
{
	if(str.size() >= 3)
	{
		if(str.left(2) == CodeKeyWords::hexCodePrefix.data())
		{
			for(int i=CodeKeyWords::encodingPrefixSize; i<str.size(); i++)
				if(!CodeKeyWords::hexSymbols.contains(str[i].toLower()))
					return false;
			return true;
		}
	}
	return false;
}

std::vector<int> Code::DecodeStrNumbers(const QString &strNumbers, bool printErrorIfEmpty)
{
	std::vector<int> result;
	auto elements = strNumbers.split(',',QString::SkipEmptyParts);
	QString error;
	for(auto &element:elements)
	{
		bool ok;
		int intElement = element.toInt(&ok);
		if(ok) result.push_back(intElement);

		if(!ok)
		{
			if(element.count('-') == 1)
			{
				QString first = element.left(element.indexOf('-'));
				QString secnd = element.remove(0,first.length()+1);
				bool ok1, ok2;
				int start = first.toInt(&ok1);
				int end = secnd.toInt(&ok2);
				if(ok1 && ok2)
				{
					if(end <= start) error = "DecodeStrNumbers error end("+QSn(end)+") <= start("+QSn(start)+") " + strNumbers;
					else
					{
						for(int i=start; i<=end; i++) result.push_back(i);
					}
				}
				else error = "DecodeStrNumbers error toInt " + strNumbers;
			}
			else error = "DecodeStrNumbers wrong numbers " + strNumbers;
		}
		if(!error.isEmpty()) break;
	}

	if(!error.isEmpty())
	{
		Logs::ErrorSt(error);
		result.clear();
	}

	if(printErrorIfEmpty && result.empty()) Logs::ErrorSt("DecodeStrNumbers empty numbers: [" + strNumbers + "]");

	return result;
}

QString TextConstant::GetTextConstVal(const QString &txtConst, bool doCheck)
{
	if(doCheck && !IsItTextConstant(txtConst, false))
	{
		Logs::ErrorSt("GetTxtConstVal wrong text constant [" + txtConst + "]");
		return "";
	}

	QString ret = txtConst;
	ret.chop(ckw::quatsSymbolLength);		// безопасно даже для пустой строки
	ret.remove(0,ckw::quatsSymbolLength);	// безопасно даже для пустой строки
	return ret;
}

TextConstant::Result TextConstant::GetTextConstVal(const QString & txtConst)
{
	Result res;
	res.checkResult = IsItTextConstant(txtConst, false);
	if(res.checkResult) res.value = GetTextConstVal(txtConst, false);
	return res;
}

bool TextConstant::IsItTextConstant(const QString &text, bool printLog = false)
{
	if(text.size() > 1)
	{
		if(TextConstant::IsItQuatsSybol(text[0]))
		{
			auto currQuats = text[0];
			if(text[text.length()-1] == currQuats)
			{
				if(text.count(currQuats) == 2) return true;
				else if(printLog) Logs::ErrorSt("IsItTextConstant: " + text + " mock. Multiquats unrealesed.");
			}
			else if(printLog) Logs::LogSt("IsItTextConstant: in text ["+text+"] ends not with text constant");
		}
		else if(printLog) Logs::LogSt("IsItTextConstant: in text ["+text+"] starts not with text constant");
	}
	else if(printLog) Logs::LogSt("IsItTextConstant: wrong text ["+text+"] size");

	return false;
}

template<typename T>
bool TextConstant::IsItQuatsSybol(const T& symbol)
{
	return (symbol == CodeKeyWords::quatsSymbol1 || symbol == CodeKeyWords::quatsSymbol2);
}

bool TextConstant::ContainsSplitter(const QString & str)
{
	for(auto &c:str)
		if(TextConstant::IsItQuatsSybol(c))
			return true;
	return false;
}

QString TextConstant::AddQuates(const QString & str, char quates)
{
	return QString(str).prepend(quates).append(quates);
}

QString CodeKeyWords::TypeToStr(int type)
{
	if(type == emulatorInt) return "Эмулятор";
	if(type == servisInt) return "Сервис";
	if(type == codeUndefined) return "undefined";
    Logs::ErrorSt("TypeToStr unknown type" + QSn(type));
    return "unknown type";
}

bool CodeTests::DoCodeTests()
{
	bool correct = true;
	if(!TestGetPrevWord()) correct = false;
	if(!TestNormalize()) correct = false;
	if(!TestTextToCommands()) correct = false;
	if(!TestTextToStatements()) correct = false;
	return correct;
}

bool CodeTests::TestGetPrevWord()
{
	QStringList inputsTexts;
	QStringList inputsIndexes;
	QStringList resultsPrevMustBe;
	QStringList resultsNextMustBe;
	bool correct = true;
	bool printResAlways = false;
	//printResAlways = true;

	//						012345678
	QString test1		=  "     ";
	inputsTexts			+= {test1,test1,test1,test1};
	inputsIndexes		+= {QString("-10"), "20", "0", "4"};
	resultsPrevMustBe	+= {QString(""), "", "", ""};
	resultsNextMustBe	+= {QString(""), "", "", ""};

	//						01234567
	QString test2		=  "1 2 3";
	inputsTexts			+= {test2,test2,test2,test2,test2,test2};
	inputsIndexes		+= {QString("0"), "1", "2", "3", "4", "5"};
	resultsPrevMustBe	+= {QString(""), "1", "1", "2", "2", ""};
	resultsNextMustBe	+= {QString("2"),"2", "3", "3", "", ""};

	//						012345678901
	QString test3		=  " 1 2 3 " ;
	inputsTexts			+= {test3,test3,test3,test3,test3,test3,test3,test3};
	inputsIndexes		+= {QString("0"), "1", "2", "3", "4", "5", "6", "7"};
	resultsPrevMustBe	+= {QString(""),  "",  "1", "1", "2", "2", "3", ""};
	resultsNextMustBe	+= {QString("1"), "2", "2", "3", "3", "",  "",  ""};

	//						0123456789012345678901234567890
	QString test4		=  "   123   234    345  453326  ";
	inputsTexts			+= {test4,test4,test4,test4,test4,test4,test4,test4,test4};
	inputsIndexes		+= {QString("0"),	"2",	"4",	"5",	"7",	"9",	"20",		"23",	"28"};
	resultsPrevMustBe	+= {QString(""),	"",		"",		"",		"123",	"123",	"345",		"345",  "453326"};
	resultsNextMustBe	+= {QString("123"), "123",	"234",	"234",	"234",	"345",	"453326",	"",		""};

	if(!(inputsTexts.size() == inputsIndexes.size()
		 && inputsTexts.size() == resultsPrevMustBe.size()
		 && inputsTexts.size() == resultsNextMustBe.size()))
	{
		Logs::ErrorSt("bad sizes TestGetPrevWord");
		return false;
	}

	for(int i=0; i<inputsTexts.size(); i++)
	{
		auto result = Code::GetPrevWord(inputsTexts[i],inputsIndexes[i].toInt());
		if(result != resultsPrevMustBe[i])
		{
			correct = false;
			Logs::ErrorSt("Тест "+QSn(i+1)+" Code::GetPrevWord(\""+inputsTexts[i]+"\", "+inputsIndexes[i]+") выдал ошибку!"
						  +"\n\tрезультат <" + result + "> а ожидается <" + resultsPrevMustBe[i]+ ">");
		}
		else if(printResAlways) Logs::LogSt("Тест Code::GetPrevWord(\""+inputsTexts[i]+"\", "+inputsIndexes[i]+") "
											 +"выдал ожидаемый результат <" + result + ">");
	}

	for(int i=0; i<inputsTexts.size(); i++)
	{
		auto result = Code::GetNextWord(inputsTexts[i],inputsIndexes[i].toInt());
		if(result != resultsNextMustBe[i])
		{
			correct = false;
			Logs::ErrorSt("Тест "+QSn(i+1)+" Code::GetNextWord(\""+inputsTexts[i]+"\", "+inputsIndexes[i]+") выдал ошибку!"
						  +"\n\tрезультат <" + result + "> а ожидается <" + resultsNextMustBe[i]+ ">");
		}
		else if(printResAlways) Logs::LogSt("Тест Code::GetNextWord(\""+inputsTexts[i]+"\", "+inputsIndexes[i]+") "
											 +"выдал ожидаемый результат <" + result + ">");
	}

	return correct;
}

bool CodeTests::TestNormalize()
{
	QStringList inputsTexts;
	QStringList resultMustBe;
	bool correct = true;
	bool printResAlwause = false;
	//printResAlwause = true;

	inputsTexts			+= " Эмулятор float \n\t\r {5.45   } ";
	resultMustBe	+= "Эмулятор float { 5.45 }";

	inputsTexts			+= "23+2 -    33 *2 /3";
	resultMustBe	+= "23 + 2 - 33 * 2 / 3";

	inputsTexts			+= "3=3==2 >34< 3 <=323 > = 343!=2";
	resultMustBe	+= "3 = 3 == 2 > 34 < 3 <= 323 >= 343 != 2";

	inputsTexts			+= "obj. prop [2] (12,34 , 32 )[12 ]";
	resultMustBe	+= "obj . prop [ 2 ] ( 12 , 34 , 32 ) [ 12 ]";

	if(inputsTexts.size() != resultMustBe.size())
	{
		Logs::ErrorSt("bad sizes TestGetPrevWord");
		return false;
	}

	for(int i=0; i<inputsTexts.size(); i++)
	{
		auto result = inputsTexts[i];
		Code::Normalize(result);
		if(result != resultMustBe[i])
		{
			correct = false;
			Logs::ErrorSt("Тест "+QSn(i+1)+" Code::Normalize(\""+inputsTexts[i]+"\") выдал ошибку!"
						  +"\n\tрезультат <" + result + "> а ожидается <" + resultMustBe[i]+ ">");
		}
		else if(printResAlwause) Logs::LogSt("Тест Code::Normalize(\""+inputsTexts[i]+"\") "
											 +"выдал ожидаемый результат <" + result + ">");
	}

	return correct;
}

bool CodeTests::TestTextToCommands()
{
	std::vector<QString> inputsTexts;
	std::vector<QStringList> resultsMustBe;
	bool correct = true;
	bool printResAlwause = false;
	//printResAlwause = true;

	// простой тест
	inputsTexts.push_back(					"command1;c2w1 c2w2    c2w3     ;   command3");
	resultsMustBe.push_back(QStringList({	"command1", "c2w1 c2w2 c2w3", "command3"}));

	// тест игнорирования комментария
	inputsTexts.push_back(					"command1;c2w1 c2w2    c2w3     ;  //  command3;other;\"sdvsdv\";//;\nrow2 /");
	resultsMustBe.push_back(QStringList({	"command1", "c2w1 c2w2 c2w3","row2 /"}));

	// проверка с пустыми командами
	inputsTexts.push_back(					"command1;   ;    ;    command3");
	resultsMustBe.push_back(QStringList({	"command1", "command3"}));

	QStringList errors;

	for(uint i=0; i<inputsTexts.size(); i++)
	{
		auto result = Code::TextToCommands(inputsTexts[i]);
		if(result != resultsMustBe[i])
		{
			correct = false;
			errors += "Тест Code::TextToCommands(\""+inputsTexts[i]+"\") выдал ошибку!"
					+"\n\tрезультат <" + result.join(";") + "> а ожидается <" + resultsMustBe[i].join(";")+ ">";
			Logs::ErrorSt(errors.back());
		}
		else if(printResAlwause) Logs::LogSt("Тест TextToCommands(\""+inputsTexts[i]+"\") "
											 +"выдал ожидаемый результат <" + result.join(";") + ">");
	}

	if(!errors.isEmpty()) QMbInfo(errors.join('\n'));

	return correct;
}

bool CmpStatements(std::vector<Statement> &lhs, std::vector<Statement> &rhs)
{
	if(lhs.size() != rhs.size()) return false;
	for(auto itl = lhs.begin(), itr = rhs.begin(); itl!=lhs.end(); ++itl, ++itr)
		if(itl->header != itr->header || itl->blockInstructions != itr->blockInstructions)
			return false;
	return true;
}

bool CodeTests::TestTextToStatements()
{
	std::vector<QString> inputsTexts;
	std::vector<std::vector<Statement>> resultsMustBe;
	bool correct = true;
	bool printResAlwause = false;
	//printResAlwause = true;

	QStringList errors;
	auto test = [&correct, &errors, &printResAlwause](int i, QString &text, std::vector<Statement> &resultMustBe, int countErrorsMustBe)
	{
		int countErrorsOutputsBefore = errors.size();
		Logs::mute_debug = true;
		int countErrInLogBefore = Logs::ErrorsCount();
		auto result = Code::TextToStatements(text);
		int eggorsGetedCountFromTest = Logs::ErrorsCount() - countErrInLogBefore;
		if(eggorsGetedCountFromTest != countErrorsMustBe)
		{
			correct = false;
			errors += "Тест i"+QSn(i)+" TestTextToStatements(\""+text+"\") wrong countErrors!\n"
						+ "ошибок <" + QSn(Logs::ErrorsCount() - countErrInLogBefore) + "> а ожидается <" + QSn(countErrorsMustBe) + ">";

			if(eggorsGetedCountFromTest != 0) errors += "Errors:";
			auto it = Logs::errors.end();
			for(int i=0; i<eggorsGetedCountFromTest; i++) --it;
			for (int i = 0; i < eggorsGetedCountFromTest; ++i) {
				errors += **it;
				++it;
			}
		}
		Logs::mute_debug = false;
		Logs::ResetCounts();
		Logs::ResetLists();

		if(!CmpStatements(result, resultMustBe))
		{
			correct = false;
			errors += "Тест i"+QSn(i)+" TestTextToStatements(\""+text+"\") выдал ошибку!"
					+"\nрезультат <\n" + Statement::PrintStatements(result) + "> а ожидается <\n" + Statement::PrintStatements(resultMustBe)+ ">";
			Logs::ErrorSt(errors.back());
		}
		else if(printResAlwause) Logs::LogSt("Тест TestTextToStatements(\""+text+"\") "
											 +"выдал ожидаемый результат <" + Statement::PrintStatements(result) + ">");

		if(countErrorsOutputsBefore != errors.size())
			errors += "";
	};

	// 0 простой тест 1
	inputsTexts.push_back("command1;c2w1 c2w2    c2w3     ;   command3");
	//resultsMustBe.push_back({ Statement2("",""), Statement2(), Statement2() });
	resultsMustBe.push_back({ Statement("",{"command1"}), Statement("",{"c2w1 c2w2 c2w3"}), Statement("",{"command3"}) });
	test(0, inputsTexts.back(), resultsMustBe.back(), 0);

	// 1 простой тест 2
	inputsTexts.push_back("if(a==5) FIVE(); next   command;");
	resultsMustBe.push_back({ Statement("",{"if ( a == 5 ) FIVE ( )"}), Statement("",{"next command"}) });
	test(1, inputsTexts.back(), resultsMustBe.back(), 0);

	// 2 простой тест + тест игнорирования комментария
	inputsTexts.push_back("command1;c2w1 c2w2    c2w3     ;  //  command3;other;\"sdvsdv\";//;\nrow2 /");
	resultsMustBe.push_back({ Statement("",{"command1"}), Statement("",{"c2w1 c2w2 c2w3"}), Statement("",{"row2 /"}) });
	test(2, inputsTexts.back(), resultsMustBe.back(), 0);

	// 3 тест с блоком команд
	inputsTexts.push_back("if(a==5) { a=10; next command; cmd3 1 2 34; }");
	resultsMustBe.push_back({ Statement("if ( a == 5 )", {"a = 10","next command","cmd3 1 2 34"}) });
	test(3, inputsTexts.back(), resultsMustBe.back(), 0);

	// 4 тест с блоком команд и вложенным блоком
	inputsTexts.push_back("if(a==5) { FIVE(); if(b==5) { next command } }");
	resultsMustBe.push_back({ Statement("if ( a == 5 )", {}) });
	test(4, inputsTexts.back(), resultsMustBe.back(), 1);

	if(!errors.isEmpty()) QMbInfo(errors.join('\n'));
	//else QMbInfo("TestTextToStatements ok");

	return correct;
}

QString Statement::PrintStatements(std::vector<Statement> statements)
{
	QString str;
	for(auto &s:statements)
	{
		if(s.header.isEmpty()) str += "empty header\n";
		else str += s.header+"  // header\n";

		str += "{\n\t"+s.blockInstructions.join("\n\t")+"\n} // end block\n\n";
	}
	return str;
}
