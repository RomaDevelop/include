#ifndef CODE_TESTS_H
#define CODE_TESTS_H

#include <QMessageBox>

#include "MyCppDifferent.h"
#include "Code.h"

struct CodeTests
{
	inline static bool DoCodeTests();

	inline static bool TestGetTextsInSquareBrackets();

	inline static bool TestGetPrevWord();
	inline static bool TestNormalize();
	inline static bool TestTextToCommands();
	inline static QStringList TestTextToStatements();
};

bool CodeTests::DoCodeTests()
{
	bool correct = true;
	if(!TestGetPrevWord()) correct = false;
	if(!TestNormalize()) correct = false;
	if(!TestTextToCommands()) correct = false;
	if(!TestGetTextsInSquareBrackets()) correct = false;
	if(auto res = TestTextToStatements(); !res.isEmpty())
	{
		qdbg << "DoCodeTests errors in TestTextToStatements:";
		for(auto &row:res)
		{
			qdbg << row;
		}
		correct = false;
	}
	return correct;
}

bool CodeTests::TestGetTextsInSquareBrackets()
{
	MyCppDifferent::any_guard guarg(
				[](){CodeLogs::ActivateTestMode(true);},
				[](){CodeLogs::ActivateTestMode(false);});

	bool result = true;
	struct Test { int number; QString text; QStringList retult; QString error; };
	std::vector<Test> tests {
		{1, "",{}, ""},
		{2, "[1234]",{"1234"}, ""},
		{3, "ssss[1234] sdgvdsfv [21]",{"1234","21"}, ""},
		{4, "arr[1234][21]",{"1234","21"}, ""},
		{5, "arr[1234][21][FORWARD] 'some [123] text'",{"1234","21","FORWARD"}, ""},

		{6, "arr[12'34][2'1]",{}, "quats found inside indexing"},
		{7, "arr[1234][21] 'text",{}, "not closed quats"},
		{8, "arr[1234][[21]",{}, "not closed brackets"},
	};
	for(auto &test:tests)
	{
		CodeLogs::error.countInTestMode = 0;
		auto res = Code::GetTextsInSquareBrackets(test.text);
		if(res != test.retult)
		{
			result = false;
			qdbg << "TestGetTextsInSquareBrackets "+QSn(test.number)+" not passed, expected ["+
					test.retult.join(',')+"], but reuslt is ["+res.join(',')+"]";
		}
		if(test.error.isEmpty() and CodeLogs::error.countInTestMode == 0) ; // ok
		else
		{
			if(test.error.isEmpty() == false and CodeLogs::error.countInTestMode == 1)
			{
				if(CodeLogs::error.GetTexts(1).contains(test.error)) continue;
			}

			result = false;
			qdbg << "TestGetTextsInSquareBrackets "+QSn(test.number)+" not passed, expected ["+
					test.error+"] error, but get: ";
			qdbg << CodeLogs::error.GetTexts(CodeLogs::error.countInTestMode);
		}
	}

	if(CodeLogs::warning.countInTestMode != 0)
	{
		result = false;
		qdbg << "TestGetTextsInSquareBrackets not passed, unexpected warnings ["+
				CodeLogs::warning.GetTexts(CodeLogs::warning.countInTestMode)+"]";
	}

	return result;
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
		CodeLogs::Error("bad sizes TestGetPrevWord");
		return false;
	}

	for(int i=0; i<inputsTexts.size(); i++)
	{
		auto result = Code::GetPrevWord(inputsTexts[i],inputsIndexes[i].toInt());
		if(result != resultsPrevMustBe[i])
		{
			correct = false;
			CodeLogs::Error("Тест "+QSn(i+1)+" Code::GetPrevWord(\""+inputsTexts[i]+"\", "+inputsIndexes[i]
							+") выдал ошибку!"
						  +"\n\tрезультат <" + result + "> а ожидается <" + resultsPrevMustBe[i]+ ">");
		}
		else if(printResAlways) CodeLogs::Log("Тест Code::GetPrevWord(\""+inputsTexts[i]+"\", "+inputsIndexes[i]+") "
											 +"выдал ожидаемый результат <" + result + ">");
	}

	for(int i=0; i<inputsTexts.size(); i++)
	{
		auto result = Code::GetNextWord(inputsTexts[i],inputsIndexes[i].toInt());
		if(result != resultsNextMustBe[i])
		{
			correct = false;
			CodeLogs::Error("Тест "+QSn(i+1)+" Code::GetNextWord(\""+inputsTexts[i]+"\", "+inputsIndexes[i]
							+") выдал ошибку!"
						  +"\n\tрезультат <" + result + "> а ожидается <" + resultsNextMustBe[i]+ ">");
		}
		else if(printResAlways) CodeLogs::Log("Тест Code::GetNextWord(\""+inputsTexts[i]+"\", "+inputsIndexes[i]+") "
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

	inputsTexts			+= "23+2;-    33 *2 /3%34";
	resultMustBe	+= "23 + 2 ; - 33 * 2 / 3 % 34";

	inputsTexts			+= "3=3==2 >34< 3 <=323 > = 343!=2";
	resultMustBe	+= "3 = 3 == 2 > 34 < 3 <= 323 >= 343 != 2";

	inputsTexts			+= "obj. prop [2] (12,34 , 32 )[12 ]";
	resultMustBe	+= "obj . prop [ 2 ] ( 12 , 34 , 32 ) [ 12 ]";

	if(inputsTexts.size() != resultMustBe.size())
	{
		CodeLogs::Error("bad sizes TestGetPrevWord");
		return false;
	}

	for(int i=0; i<inputsTexts.size(); i++)
	{
		auto result = inputsTexts[i];
		Code::Normalize(result);
		if(result != resultMustBe[i])
		{
			correct = false;
			CodeLogs::Error("Тест "+QSn(i+1)+" Code::Normalize(\""+inputsTexts[i]+"\") выдал ошибку!"
						  +"\n\tрезультат <" + result + "> а ожидается <" + resultMustBe[i]+ ">");
		}
		else if(printResAlwause) CodeLogs::Log("Тест Code::Normalize(\""+inputsTexts[i]+"\") "
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
			CodeLogs::Error(errors.back());
		}
		else if(printResAlwause) CodeLogs::Log("Тест TextToCommands(\""+inputsTexts[i]+"\") "
											 +"выдал ожидаемый результат <" + result.join(";") + ">");
	}

	if(!errors.isEmpty()) QMbInfo(errors.join('\n'));

	return correct;
}

QStringList CodeTests::TestTextToStatements()
{
	std::vector<QString> inputsTexts;
	std::vector<Statement> resultsMustBe;
	bool printResAlwause = false;
	//printResAlwause = true;

	QStringList errorsAllTests;
	auto test = [&errorsAllTests, &printResAlwause]
			(int i, QString &text, Statement &resultMustBe, int countErrorsMustBe, int countWarningsMustBe, bool resultShouldBe = true)
	{

		QStringList errorsThisTest;
		MyCppDifferent::any_guard guarg(
					[](){CodeLogs::ActivateTestMode(true);},
					[](){CodeLogs::ActivateTestMode(false);});
		int countErrInLogBefore = CodeLogs::error.countInTestMode;
		int countWrnInLogBefore = CodeLogs::warning.countInTestMode;
		auto result = Code::TextToStatements(text);
		int eggorsGetedCountFromTest = CodeLogs::error.countInTestMode - countErrInLogBefore;
		int wrnGetedCountFromTest = CodeLogs::warning.countInTestMode - countWrnInLogBefore;
		if(eggorsGetedCountFromTest != countErrorsMustBe or wrnGetedCountFromTest != countWarningsMustBe)
		{
			errorsThisTest += "Тест i"+QSn(i)+" TestTextToStatements(\""+text+"\") wrong count prints!"
						+ "\nошибок <" + QSn(eggorsGetedCountFromTest) + "> а ожидается <" + QSn(countErrorsMustBe) + ">"
				+ "\nпредупреждений <" + QSn(wrnGetedCountFromTest) + "> а ожидается <" + QSn(countWarningsMustBe) + ">";

			if(eggorsGetedCountFromTest != 0)
				errorsThisTest += "Errors: " + CodeLogs::error.GetTexts(eggorsGetedCountFromTest);

			if(wrnGetedCountFromTest != 0)
				errorsThisTest += "Warnings: " + CodeLogs::warning.GetTexts(eggorsGetedCountFromTest);
		}

		QString resultCmpDetails;
		if(Statement::CmpStatement(result, resultMustBe, &resultCmpDetails) != resultShouldBe)
		{
			if(!errorsThisTest.isEmpty()) errorsThisTest +=	"============================================================================";
			errorsThisTest += "Тест i"+QSn(i)+" TestTextToStatements(\""+text+"\") выдал ошибку!"
					+"\nрезультат <\n" + result.PrintStatement()
					+ ">\nа ожидается <\n" + resultMustBe.PrintStatement()+ ">";

			if(resultCmpDetails.isEmpty()) errorsThisTest += "resultCmpDetails.isEmpty";
			else errorsThisTest += "resultCmpDetails: " + resultCmpDetails;
		}
		else if(printResAlwause) CodeLogs::Log("Тест TestTextToStatements(\""+text+"\") "
											 +"выдал ожидаемый результат <" + result.PrintStatement() + ">");

		if(!errorsThisTest.isEmpty())
		{
			errorsThisTest += "============================================================================";
			errorsThisTest += "============================================================================";
		}

		errorsAllTests += errorsThisTest;
	};

	// 0 простой тест 1
	inputsTexts.push_back("command1;c2w1 c2w2    c2w3     ;   command3");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{"command1", "c2w1 c2w2 c2w3", "command3"});
	test(0, inputsTexts.back(), resultsMustBe.back(), 0, 0);

	// 1 простой тест 2
	inputsTexts.push_back("if(a==5) FIVE(); next   command;");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{"if ( a == 5 ) FIVE ( )", "next command"});
	test(1, inputsTexts.back(), resultsMustBe.back(), 0, 0);

	// 11 простой тест 2 - проверка ложно-положительного
	inputsTexts.push_back("if(a==5) FIVE(); next   command;");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{"if ( a==5 ) FIVE ( )", "next command"});
	test(11, inputsTexts.back(), resultsMustBe.back(), 0, 0, false);

	// 12 простой тест 2 - проверка ложно-положительного 2
	inputsTexts.push_back("if(a==5) FIVE(); next   command;");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{"if ( a == 5 ) FIVE ( )", "next coOmmand"});
	test(12, inputsTexts.back(), resultsMustBe.back(), 0, 0, false);

	// 13 простой тест 3 - пустые команды
	inputsTexts.push_back("op1;op2;;;op3;");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{"op1","op2","op3",});
	test(13, inputsTexts.back(), resultsMustBe.back(), 0, 2, true);

	// 2 простой тест + тест игнорирования комментария
	inputsTexts.push_back("command1;c2w1 c2w2    c2w3     ;  //  command3;other;\"sdvsdv\";//;\nrow2 /");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{"command1", "c2w1 c2w2 c2w3", "row2 /"});
	test(2, inputsTexts.back(), resultsMustBe.back(), 0, 0);

	// 3 тест с блоком команд
	inputsTexts.push_back("if(a==5) { a=10; next command; cmd3 1 2 34; }");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{Statement{"if ( a == 5 )", Statement::VectorStatementOrQString{
												"a = 10", "next command", "cmd3 1 2 34"}}});
	test(3, inputsTexts.back(), resultsMustBe.back(), 0, 0);

	// 5 тест с блоком команд и вложенным блоком 2
	inputsTexts.push_back("if(a==5) { FIVE(); Six(); if(b==5) { nested block; } }");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{Statement{"if ( a == 5 )", Statement::VectorStatementOrQString{
												"FIVE ( )", "Six ( )",
												Statement("if ( b == 5 )", Statement::VectorStatementOrQString{"nested block"})
												}}});
	test(5, inputsTexts.back(), resultsMustBe.back(), 0, 0);

	// 51 тест с блоком команд и вложенным блоком 2 - с ошибкой, отсутсвующ точка с запятой
	inputsTexts.push_back("if(a==5) { FIVE(); Six(); if(b==5) { nested block } }");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{Statement{"if ( a == 5 )", Statement::VectorStatementOrQString{
												"FIVE ( )", "Six ( )",
												Statement("if ( b == 5 )", Statement::VectorStatementOrQString{})
												}}});
	test(51, inputsTexts.back(), resultsMustBe.back(), 1, 0);

	// 52 тест с блоком команд и вложенным блоком 2 - с ошибкой, отсутсвующ точка с запятой перед if и отсутсвующ точка с запятой в блоке
/// данная проверка должна происходить уже при распознавании слов инструкции
//	inputsTexts.push_back("if(a==5) { FIVE(); Six() if(b==5) { nested block } }");
//	resultsMustBe.push_back({ Statement2("if ( a == 5 )", {Statement2("FIVE ( )"), Statement2("Six ( )"),
//		Statement2("if ( b == 5 )", std::vector<Statement2>())}) });
//	test(52, inputsTexts.back(), resultsMustBe.back(), 2);

	// 6 тест с блоком команд и вложенным блоком, командами до и после
	inputsTexts.push_back("pred op; if(a==5) { FIVE(); Six(); if(b==5) { nested op1; nested op2; } end command in a== 5; } after op;");
	resultsMustBe.emplace_back("", Statement::VectorStatementOrQString{
			 "pred op",
			 Statement("if ( a == 5 )", Statement::VectorStatementOrQString{
							   "FIVE ( )",
							   "Six ( )",
							   Statement("if ( b == 5 )", Statement::VectorStatementOrQString{"nested op1", "nested op2"}),
							   "end command in a == 5"
						}),
			 "after op"
			});
	test(6, inputsTexts.back(), resultsMustBe.back(), 0, 0);

	auto test2 = [&errorsAllTests, &printResAlwause]
			(int i, QStringList &resEtalon, QStringList &resFakt)
	{
		QStringList errorsThisTest;
		if(resEtalon != resFakt)
		{
			errorsThisTest += "Тест i"+QSn(i)+" ForEach(...) выдал ошибку!"
					+"\nрезультат <\n" + resFakt.join(", ")
					+">\nа ожидается <\n" + resEtalon.join(", ") + ">";
		}
		else if(printResAlwause) CodeLogs::Log(QString("Тест i"+QSn(i)+" ForEach(...) ")
											 +"выдал ожидаемый результат <" + resEtalon.join(", ") + ">");

		if(!errorsThisTest.isEmpty())
		{
			errorsThisTest += "============================================================================";
		}

		errorsAllTests += errorsThisTest;
	};

	// тест полный ForEach
	bool stExitFlag=0, returnFlag=0;
	QStringList ForEachResEtalon = QStringList{"", "pred op", "if ( a == 5 )", "FIVE ( )", "Six ( )", "if ( b == 5 )", "nested op1",
									"nested op2", "end command in a == 5", "after op"};
	QStringList ForEachRes = QStringList{};
	resultsMustBe.back().ForEach([&ForEachRes](std::pair<Statement*,QString*> item){
		if(item.first) ForEachRes += item.first->header;
		if(item.second) ForEachRes += *item.second;
	}, stExitFlag, returnFlag);

	test2(7, ForEachResEtalon, ForEachRes);

	// тест ForEach с returnFlag 1
	stExitFlag=0, returnFlag=0;
	ForEachResEtalon = QStringList{"", "pred op", "if ( a == 5 )", "FIVE ( )"};
	ForEachRes = QStringList{};
	resultsMustBe.back().ForEach([&ForEachRes, &returnFlag](std::pair<Statement*,QString*> item){
		if(item.second && *item.second == "Six ( )") { returnFlag=true; return; }
		if(item.first) ForEachRes += item.first->header;
		if(item.second) ForEachRes += *item.second;
	}, stExitFlag, returnFlag);

	test2(8, ForEachResEtalon, ForEachRes);

	// тест ForEach с returnFlag 2
	stExitFlag=0, returnFlag=0;
	ForEachResEtalon = QStringList{"", "pred op", "if ( a == 5 )", "FIVE ( )", "Six ( )", "if ( b == 5 )", "nested op1"};
	ForEachRes = QStringList{};
	resultsMustBe.back().ForEach([&ForEachRes, &returnFlag](std::pair<Statement*,QString*> item){
		if(item.second && *item.second == "nested op2") { returnFlag=true; return; }
		if(item.first) ForEachRes += item.first->header;
		if(item.second) ForEachRes += *item.second;
	}, stExitFlag, returnFlag);

	test2(9, ForEachResEtalon, ForEachRes);

	// pred op; if(a==5) { FIVE(); Six(); if(b==5) { nested op1; nested op2; } end command in a== 5; } after op;"

	// тест ForEach с stExitFlag 1
	stExitFlag=0, returnFlag=0;
	ForEachResEtalon = QStringList{"", "pred op", "if ( a == 5 )", "FIVE ( )", "after op"};
	ForEachRes = QStringList{};
	resultsMustBe.back().ForEach([&ForEachRes, &stExitFlag](std::pair<Statement*,QString*> item){
		if(item.second && *item.second == "Six ( )") { stExitFlag=true; return; }
		if(item.first) ForEachRes += item.first->header;
		if(item.second) ForEachRes += *item.second;
	}, stExitFlag, returnFlag);

	test2(10, ForEachResEtalon, ForEachRes);

	// тест ForEach с stExitFlag 2
	stExitFlag=0, returnFlag=0;
	ForEachResEtalon = QStringList{"", "pred op", "if ( a == 5 )", "FIVE ( )", "Six ( )", "if ( b == 5 )", "nested op1",
										"end command in a == 5", "after op"};
	ForEachRes = QStringList{};
	resultsMustBe.back().ForEach([&ForEachRes, &stExitFlag](std::pair<Statement*,QString*> item){
		if(item.second && *item.second == "nested op2") { stExitFlag=true; return; }
		if(item.first) ForEachRes += item.first->header;
		if(item.second) ForEachRes += *item.second;
	}, stExitFlag, returnFlag);

	test2(11, ForEachResEtalon, ForEachRes);

//	resultsMustBe.back().Remove_child_if([](std::pair<Statement*,QString*> item){
//		if(item.second and item.second->contains("FIVE")) return true;
//		if(item.first and item.first->header.contains("a == 5")) return true;
//		return false;
//	});

//	if(!errorsAllTests.isEmpty()) MyQDialogs::ShowText(errorsAllTests);

	return errorsAllTests;
}


#endif // CODE_TESTS_H
