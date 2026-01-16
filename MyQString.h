#ifndef MyQStrring_H
#define MyQStrring_H

#include <QDebug>
#include <QStringList>
#include <QFontMetrics>
#include <QRect>

#include "MyQDifferent.h"

//------------------------------------------------------------------------------------------------------------------------------------------

struct MyQString
{
	inline static QStringList QStringListSized(int size, const QString &value = "");
	inline static QStringList SizedQStringList(int size, const QString &value = "") { return QStringListSized(size, value); }
	inline static void EmplaceBack(QStringList &list, QString &&newString);

	inline static QStringList ArgsToStrList(int argc, char *argv[]);

	inline static void Append(QStringList &list, QString str) { QString &lastStr = *list.insert(list.end(),""); lastStr=std::move(str); }

	inline static QString GetRowOfLetter(const QString& str, int letterIndex);
	struct RowAndIndex { QString row; int indexInRow = -1; };
	inline static RowAndIndex GetRowOfLetterExt(const QString& str, int letterIndex);

	template<class String, class Splitter>
	inline static std::pair<QString, QString> SplitOnce(const String &string, const Splitter &splitter);

	/// Returns cout removed sybols in the beginginning and in the end
	inline static std::pair<int, int> TrimExt(QString &str);

	/// Finds the index of the n-th occurrence of a substring
	template<class T_substr>
	inline static int IndexOfNumbered(const QString &str, const T_substr &subStr, int n, int from = 0, bool subIntersections = false);

	struct FoundSubstr { QString substr; std::vector<int> positions; };
	static std::vector<FoundSubstr> FindSubstrings(const QString &text, const QStringList &keywords) {
		std::vector<FoundSubstr> result;

		for (const QString &word : keywords) {
			if (word.isEmpty()) continue;

			FoundSubstr found;
			found.substr = word;

			int pos = text.indexOf(word, 0);
			while (pos != -1) {
				found.positions.push_back(pos);
				pos = text.indexOf(word, pos + 1); // Ищем следующее вхождение
			}

			if (!found.positions.empty()) {
				result.push_back(found);
			}
		}

		return result;
	}

	inline static bool StartsWith(const QString &str, const std::string_view &str_view);

	template<typename... Args>
	inline static void Append(QString& s, const Args&... args) { (SmartAppend(s, args), ...); }
	template<typename Separator_t, typename Arg1, typename... ArgsTail>
	inline static void AppendSep(QString& s, const Separator_t &sep, const Arg1& arg1, const ArgsTail&... args)
	{ SmartAppend(s, arg1); ( (s.append(sep), SmartAppend(s, args)), ... ); }

	template<typename... Args>
	inline static QString Join(const Args&... args) { QString s; Append(s, args...); return s; }
	template<typename Separator_t, typename Arg1, typename... ArgsTail>
	inline static QString JoinSep(const Separator_t &sep, const Arg1& arg1, const ArgsTail&... args)
	{ QString s; AppendSep(s, sep, arg1, args...); return s; }

	template<class char_type>
	inline static QString& RightJistifie(QString &str_to_justifie, int width, char_type fill=' ', bool trunc = false);
	template<class char_type>
	inline static QString& LeftJistifie(QString &str_to_justifie, int width, char_type fill=' ', bool trunc = false);

	template<class QWidgetWithText>
	inline static int TextWidthInWidget(QWidgetWithText *widget)
	{ return QFontMetrics(widget->font()).boundingRect(widget->text()).width(); }

	template<class int_type>
	inline static QString AsNumberDigits(int_type n, QChar separator = ' ')
	{
		static_assert(std::is_integral<int_type>::value, "AsNumberDigits accepts only integral type");
		QString result = QString::number(n);
		for(int i=result.size()-1, j=1; i>=0; i--, j++)
		{
			if(j%3 == 0) result.insert(i, separator);
		}
		return result;
	}

	static QString BytesToString(uint64_t bytesCount) { return MyQDifferent::BytesToString(bytesCount); }

	template<class uint_type>
	inline static QString ToBincode(uint_type n, int output_width = -1)
	{
		static_assert(std::is_unsigned<uint_type>::value, "ToBincode accepts only unsigned arithmetic type");

		constexpr int typeBitLen = sizeof(uint_type)*8;
		int resBitLen = output_width == -1 ? typeBitLen : output_width;
		QString res(resBitLen , '0');
		for (int i = resBitLen-1; i >= 0; --i)
		{
			res[i] = (n & 1) ? '1' : '0'; // Берём младший бит
			n >>= 1; // Сдвиг вправо
		}
		return res;
	}

	template<class T>
	inline static QString AsDebug(const T& obj)
	{
		QString ret;
		ret.clear();
		QDebug customDebug(&ret);
		customDebug << obj;
		return ret;
	}

	using mapQCHar = std::map<QChar, QChar>;
	inline static std::pair<mapQCHar, mapQCHar> TranslitWrongLanguageMaps();
	inline static QString TranslitWrongLanguage(QString str);

	inline static QString TranslitToLatin(const QString &str);

	inline static QString ToSentenceCase(QString str);
	inline static QString ToUpperWordStartLetter(QString str);

private:
	static constexpr int get_length_universal(QChar) { return 1; }
	static constexpr int get_length_universal(char) { return 1; }
	static int get_length_universal(const char* s) { return s ? int(strlen(s)) : 0; }
	template<class S>
	static constexpr int get_length_universal(const S &s) { return int(s.size()); }

//	template<typename T>
//    inline void SmartAppend(QString& s, const T& arg) {
//        s.append(arg);
//    }

//    // Специализированная вспомогательная функция для числовых типов
//    template<typename T>
//    inline typename std::enable_if<std::is_arithmetic<T>::value>::type
//    SmartAppend(QString& s, const T& arg) {
//        s.append(QString::number(arg));
//    }
	template<typename T>
	inline static void SmartAppend(QString& s, const T& arg) {
		if constexpr (std::is_arithmetic_v<T> and not std::is_same_v<T, char>) {
			// Эта ветка компилируется только для чисел, кроме char
			s.append(QString::number(arg));
		} else {
			// Эта ветка компилируется только для остальных типов
			s.append(arg);
		}
	}
};

//-------------------------------------------------------------------------------------------------------------------------------

///\brief Класс для хранения строковых литералов
/// для класса определены операторы стравнения на равенство и неравенство, операторы словжения с QString
/// (для голого string_view определять их крайне не рекомендуется, поскольку в конце может не быть /0)
struct StringLiteral
{
	/// передавать только строковые литералы!!!
	template<std::size_t N>
	constexpr StringLiteral(const char (&str)[N]) : _str(str, N - 1) {}
	//constexpr StringLiteral(const char *str): _str(str) {}
	constexpr const char * Get() const { return _str.data(); }
	constexpr size_t Size() const { return _str.size(); }

	/// нужно улучшать: в конструкторе вместо constexpr поставить consteval (C++20),
	/// сделать проверку наличия \0 в конце
	/// возможно ли создать надёжную защиту от передачи в конструктор std::string("...").data() ???
	/// вообще возможно при сравнении QString с const char *str внутри происходит конфертация
	/// может создать QString_view который будет хранить на стеке строковые литералы?
	///		но он ведь всё равно будет принимать const char * и можно будет передать без \0
	///		зато оперция сравнения будет быстрее. Возможно. Будет ли?
	/// нужно отказаться от string_view потому что она плохо работает с кириллицей
	/// и невозможно сделать ранний выход в operator== по размеру
	///
	/// после широкого использования провести замер, насколько шаблонный конструктор замедляет и раздувает

private:
	const std::string_view _str;
};

inline bool operator== (const QString &lhs, const StringLiteral &rhs) {
//	if(lhs.size() != rhs.Size()) // нельзя!!! не будет работать с кириллицей
//		return false;
	return lhs==rhs.Get();
}
inline bool operator== (const StringLiteral &lhs, const QString &rhs) { return rhs==lhs; }
inline bool operator!= (const QString &lhs, const StringLiteral &rhs) { return !(lhs==rhs); }
inline bool operator!= (const StringLiteral &lhs, const QString &rhs) { return !(lhs==rhs); }

inline QString operator+ (const char* lhs, const StringLiteral &rhs) { return QString(lhs) + rhs.Get(); }
inline QString operator+ (const StringLiteral &lhs, const char* rhs) { return QString(lhs.Get()) + rhs; }

inline QString operator+ (const QString &lhs, const StringLiteral &rhs) { return lhs + rhs.Get(); }
inline QString operator+ (const StringLiteral &lhs, const QString &rhs) { return lhs.Get() + rhs; }

//-------------------------------------------------------------------------------------------------------------------------------

QStringList MyQString::QStringListSized(int size, const QString & value)
{
	QStringList ret;
	ret.reserve(size);
	for(int i=0; i<size; i++)
		ret.append(value);
	/// тут цикл!!! нельзя делать ret[i] = std::move(value)
	return ret;
}

void MyQString::EmplaceBack(QStringList &list, QString &&newString)
{
	list.append(QString());
	list.back() = std::move(newString);
}

QStringList MyQString::ArgsToStrList(int argc, char *argv[])
{
	return MyQDifferent::ArgsToStrList(argc, argv);
}

QString MyQString::GetRowOfLetter(const QString &str, int letterIndex)
{
	return GetRowOfLetterExt(str, letterIndex).row;
}

MyQString::RowAndIndex MyQString::GetRowOfLetterExt(const QString &str, int letterIndex)
{
	if (letterIndex >= str.length() || letterIndex < 0) { return {}; }

	int start = letterIndex;
	int indexInRow = 0;

	while (start > 0 && str[start - 1] != '\n') { start--; indexInRow++; }

	int end = str.indexOf('\n', letterIndex);

	if (end == -1) { end = str.length(); }

	return { str.mid(start, end - start), indexInRow };
}

std::pair<int, int> MyQString::TrimExt(QString &str)
{
	std::pair<int, int> res;

	// remove from the end (primarily for microoptimisation)
	if(str.isEmpty()) return res;
	int size = str.size();
	while(true)
	{
		int index = size - (1 + res.second);
		if(index < 0) break;
		auto curChar = str[index];
		if(curChar == ' ' or curChar == '\n' or curChar == '\r'  or curChar == '\t')
		{
			res.second++;
		}
		else break;
	}
	str.chop(res.second);

	// remove from the beginning
	if(str.isEmpty()) return res;
	while(true)
	{
		if(res.first >= str.size()) break;
		auto curChar = str[res.first];
		if(curChar == ' ' or curChar == '\n' or curChar == '\r'  or curChar == '\t')
		{
			res.first++;
		}
		else break;
	}
	str.remove(0,res.first);

	return res;
}

template<class T_substr>
int MyQString::IndexOfNumbered(const QString &str, const T_substr &subStr, int n, int from, bool subIntersections)
{
	if (n <= 0) return -1;

	int len = get_length_universal(subStr);
	if (len <= 0) return -1;
	int addToPos = subIntersections ? 1 : len;

	// 1. Ищем первое вхождение
	int pos = str.indexOf(subStr, from);

	// 2. Ищем оставшиеся n-1 вхождений
	for (int i = 1; i < n && pos != -1; ++i) {
		pos = str.indexOf(subStr, pos + addToPos);
	}

	return pos;
}

bool MyQString::StartsWith(const QString &str, const std::string_view &str_view)
{
	if((size_t)str.length() < str_view.length()) return false;

	uint index = 0;
	for(auto c:str_view)
	{
		if(str[index] != c) return false;
		index++;
	}

	return true;
}

std::pair<MyQString::mapQCHar, MyQString::mapQCHar> MyQString::TranslitWrongLanguageMaps()
{
	const wchar_t *latin = L"qwertyuiop[]asdfghjkl;'zxcvbnm,./QWERTYUIOP{}ASDFGHJKL:\"" "ZXCVBNM<>"; // разрыв из-за экранирования \"
	const wchar_t *kiril = L"йцукенгшщзхъфывапролджэячсмитьбю.ЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭ"  "ЯЧСМИТЬБЮ";
	auto len = wcslen(latin);
	assert(len == wcslen(kiril));

	std::map<QChar, QChar> toLatin;
	std::map<QChar, QChar> toKiril;

	for(size_t i=0; i<len; i++)
	{
		toLatin[*kiril] = *latin;
		toKiril[*latin] = *kiril;
		kiril++;
		latin++;
	}

	return {std::move(toLatin), std::move(toKiril)};
}

QString MyQString::TranslitWrongLanguage(QString str)
{
	static std::pair<mapQCHar, mapQCHar> translitMaps = TranslitWrongLanguageMaps();
	for(auto &c:str)
	{
		if(auto it = translitMaps.first.find(c); it != translitMaps.first.end())
		{
			c = it->second;
		}
		else if(auto it = translitMaps.second.find(c); it != translitMaps.second.end())
		{
			c = it->second;
		}
		// else ; // буква остается без изменений
	}
	return str;
}

QString MyQString::TranslitToLatin(const QString &str)
{
	static std::map<QChar, const char *> translit { {L'а',"a"},   {L'б',"b"},  {L'в',"v"},  {L'г',"g"},  {L'д',"d"},
													{L'е',"e"},   {L'ё',"yo"}, {L'ж',"zh"}, {L'з',"z"},  {L'и',"i"},
													{L'й',"j"},
													{L'к',"k"},   {L'л',"l"},  {L'м',"m"},  {L'н',"n"},  {L'о',"o"},
													{L'п',"p"},   {L'р',"r"},  {L'с',"s"},  {L'т',"t"},  {L'у',"u"},
													{L'ф',"f"},   {L'х',"h"},  {L'ц',"ts"}, {L'ч',"ch"}, {L'ш',"sh"},
													{L'щ',"sch"}, {L'ъ',""},   {L'ы',"y"},  {L'ь',""},   {L'э',"e"},
													{L'ю',"yu"},  {L'я',"ya"},

													{L'А',"A"},   {L'Б',"B"},  {L'В',"V"},  {L'Г',"G"},  {L'Д',"D"},
													{L'Е',"E"},   {L'Ё',"YO"}, {L'Ж',"ZH"}, {L'З',"Z"},  {L'И',"I"},
													{L'Й',"J"},
													{L'К',"K"},   {L'Л',"L"},  {L'М',"M"},  {L'Н',"N"},  {L'О',"O"},
													{L'П',"P"},   {L'Р',"R"},  {L'С',"S"},  {L'Т',"T"},  {L'У',"U"},
													{L'Ф',"F"},   {L'Х',"H"},  {L'Ц',"TS"}, {L'Ч',"CH"}, {L'Ш',"SH"},
													{L'Щ',"SCH"}, {L'Ъ',""},   {L'Ы',"Y"},  {L'Ь',""},   {L'Э',"E"},
													{L'Ю',"YU"},  {L'Я',"YA"},
											 /* */};

	QString result;
	for(auto &c:str)
	{
		if(auto it = translit.find(c); it != translit.end())
		{
			result += it->second;
		}
		else result += c;
	}
	return result;
}

QString MyQString::ToSentenceCase(QString str)
{
	int size = str.size();
	if(size >= 1) str[0] = str[0].toUpper();
	if(size >= 2) str[1] = str[1].toLower();
	for(int i=4; i<size; i++)
	{
		if(str[i-2] == '.' && str[i-1] == ' ')
			str[i] = str[i].toUpper();
		else str[i] = str[i].toLower();
	}
	return str;
}

QString MyQString::ToUpperWordStartLetter(QString str)
{
	int size = str.size();
	if(size >= 1) str[0] = str[0].toUpper();
	for(int i=1; i<size; i++)
	{
		if(str[i-1] == ' ')
			str[i] = str[i].toUpper();
		else str[i] = str[i].toLower();
	}
	return str;
}

template<class char_type>
QString & MyQString::RightJistifie(QString & str_to_justifie, int width, char_type fill, bool trunc)
{
	static_assert(std::is_same<char_type, char>::value || std::is_same<char_type, QChar>::value,
			"RightJistifie accepts only char or QChar on arg fill");

	int size = str_to_justifie.size();
	if(size > width)
	{
		if(trunc) return str_to_justifie.remove(0, size - width);
		else return str_to_justifie;
	}
	if(size == width) return str_to_justifie;

	int inserting = width - size;
	for(int i=0; i<inserting; i++)
		str_to_justifie.prepend(fill);

	return str_to_justifie;
}

template<class char_type>
QString & MyQString::LeftJistifie(QString & str_to_justifie, int width, char_type fill, bool trunc)
{
	static_assert(std::is_same<char_type, char>::value || std::is_same<char_type, QChar>::value,
			"LeftJistifie accepts only char or QChar on arg fill");

	int size = str_to_justifie.size();
	if(size > width)
	{
		if(trunc) { str_to_justifie.chop(size - width); return str_to_justifie; }
		else return str_to_justifie;
	}
	if(size == width) return str_to_justifie;

	str_to_justifie.resize(width, fill);

	return str_to_justifie;
}

template<class String, class Splitter>
std::pair<QString, QString> MyQString::SplitOnce(const String &string, const Splitter &splitter)
{
	int pos = string.indexOf(splitter);
	if (pos == -1)
		return { QString(string), {} };
	return { string.left(pos), string.mid(pos + get_length_universal(splitter)) };
}

#endif
