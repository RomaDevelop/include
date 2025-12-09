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

	inline static QStringList ArgsToStrList(int argc, char *argv[]);

	inline static QString GetRowOfLetter(const QString& str, int letterIndex);

	template<class String, class Splitter>
	inline static std::pair<QString, QString> SplitOnce(const String &string, const Splitter &splitter);

	inline static bool StartsWith(const QString &str, const std::string_view &str_view);

	template<typename... Args>
	inline static void Append(QString& s, const Args&... args) { (s.append(args), ...); }
	template<class char_type>
	inline static QString& RightJistifie(QString &str_to_justifie, int width, char_type fill = ' ', bool trunc = false);
	template<class char_type>
	inline static QString& LeftJistifie(QString &str_to_justifie, int width, char_type fill = ' ', bool trunc = false);

	template<class QWidgetWithText>
	inline static int TextWidthInWidget(QWidgetWithText *widget)
	{
		return QFontMetrics(widget->font()).boundingRect(widget->text()).width();
	}

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
	static constexpr int sepLen(QChar) { return 1; }
	static constexpr int sepLen(char) { return 1; }
	static int sepLen(const char* s) { return int(strlen(s)); }
	template<class S>
	static constexpr int sepLen(const S &s) { return int(s.size()); }
};

//------------------------------------------------------------------------------------------------------------------------------------------

QStringList MyQString::QStringListSized(int size, const QString & value)
{
	QStringList ret;
	ret.reserve(size);
	for(int i=0; i<size; i++)
		ret.append(value);
	/// тут цикл!!! нельзя делать ret[i] = std::move(value)
	return ret;
}

QStringList MyQString::ArgsToStrList(int argc, char *argv[])
{
	return MyQDifferent::ArgsToStrList(argc, argv);
}

QString MyQString::GetRowOfLetter(const QString &str, int letterIndex)
{
	if (letterIndex >= str.length() || letterIndex < 0) { return ""; }

	int start = letterIndex;

	while (start > 0 && str[start - 1] != '\n') { start--; }

	int end = str.indexOf('\n', letterIndex);

	if (end == -1) { end = str.length(); }

	return str.mid(start, end - start).trimmed();
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
	return { string.left(pos), string.mid(pos + sepLen(splitter)) };
}

#endif
