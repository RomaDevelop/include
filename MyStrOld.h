#ifndef MyStrOld_H
#define MyStrOld_H

#include <QDebug>
#include <QStringList>

struct MyStrOld
{
    inline static int CountElemetsInStrings(QString strAsList, QString splitter)
    {
        if(strAsList.endsWith(splitter)) strAsList.chop(splitter.size());
        return strAsList.split(splitter).size();
    }
    inline static QString DelElemetFromStringsByValue(QString strAsList, QString splitter, QString value)
    {
        if(strAsList.endsWith(splitter)) strAsList.chop(splitter.size());
        auto tmpStrList = strAsList.split(splitter);
        if(!tmpStrList.removeOne(value)) qDebug() << "DelElemetFromStringsByValue - removeOne false result";
        return tmpStrList.join(splitter) + splitter;
    }

    inline static QString SortElemetsInStrings(QString strAsList, QString splitter)
    {
        if(strAsList.endsWith(splitter)) strAsList.chop(splitter.size());
        auto tmpStrList = strAsList.split(splitter);
        tmpStrList.sort();
        return tmpStrList.join(splitter) + splitter;
    }

};

#endif
