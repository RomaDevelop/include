#ifndef MyQStrring_H
#define MyQStrring_H

#include <QDebug>
#include <QStringList>

struct MyQString
{
    inline static QStringList QStringListSized(int size, const QString &value = "")
    {
	QStringList ret;
	ret.reserve(size);
	for(int i=0; i<size; i++)
	    ret.append(value);
	return ret;
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

#endif
