#ifndef MyQStrring_H
#define MyQStrring_H

#include <QDebug>
#include <QStringList>

struct MyQString
{
    inline static QStringList QStringListSized(int size, const QString &value = "");

    inline static QString& RightJistifie(QString &str_to_justifie, int width, QChar fill = QLatin1Char(' '), bool trunc = false);
    inline static QString& LeftJistifie(QString &str_to_justifie, int width, QChar fill = QLatin1Char(' '), bool trunc = false);

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

QString & MyQString::RightJistifie(QString & str_to_justifie, int width, QChar fill, bool trunc)
{
    int size = str_to_justifie.size();
    if(size > width)
    {
	if(trunc) return str_to_justifie.remove(0, size - width);
	else return str_to_justifie;
    }
    if(size == width) return str_to_justifie;
    str_to_justifie.resize(width);
    for(int i=size-1, j=width-1; i>=0; i--, j--)
	str_to_justifie[j]=str_to_justifie[i];
    int inserting = width - size;
    for(int i=0; i<inserting; i++)
	str_to_justifie[i] = fill;
    return str_to_justifie;
}

QString & MyQString::LeftJistifie(QString & str_to_justifie, int width, QChar fill, bool trunc)
{
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
