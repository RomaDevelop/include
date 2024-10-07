#ifndef MyQStrring_H
#define MyQStrring_H

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
};

#endif
