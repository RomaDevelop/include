//---------------------------------------------------------------------------
#ifndef MQBYTEARRAY_H
#define MQBYTEARRAY_H
//---------------------------------------------------------------------------
#include <QByteArray>
//---------------------------------------------------------------------------
class MyQBA
{
public:
    static QString QByteArrToStr(const QByteArray &byteArray)
    {
        QString ret;
        for(int i=0; i<byteArray.size(); i++)
        {
            QString byte = QString::number((int)byteArray[i]);
            while(byte.length()<4) byte = " " + byte;
            ret += byte + ";";
        }
        return ret;
    }
};
#endif
