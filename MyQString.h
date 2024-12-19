#ifndef MyQStrring_H
#define MyQStrring_H

#include <QDebug>
#include <QStringList>

struct MyQString
{
	inline static QStringList QStringListSized(int size, const QString &value = "");

	template<class char_type>
	inline static QString& RightJistifie(QString &str_to_justifie, int width, char_type fill = ' ', bool trunc = false);
	template<class char_type>
	inline static QString& LeftJistifie(QString &str_to_justifie, int width, char_type fill = ' ', bool trunc = false);

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
};

QStringList MyQString::QStringListSized(int size, const QString & value)
{
	QStringList ret;
	ret.reserve(size);
	for(int i=0; i<size; i++)
		ret.append(value);
	return ret;
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

#endif
