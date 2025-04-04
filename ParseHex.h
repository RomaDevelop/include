#ifndef ParseHex_H
#define ParseHex_H

#include <map>

#include <QString>

#include "MyQFileDir.h"

struct ParseHex
{
    using QStringsMap = std::map<QString,QString>;
    inline static QString ParseFile(const QString &file, QStringsMap &adressesAndCommands);

    inline static bool CheckSumForHexRow(const QString& line);
    inline static uint16_t CalcByteCrc(uint16_t CrcOld, uint8_t Byte)
    {
        /*
        ---------------------------------------------------------------
        Ссылка на онлайн расчёт CRC -16 / MODBUS:
        ---------------------------------------------------------------
        https://crccalc.com/?crc=0100%200100%200000%200000%200000%200000%200000%200000%200000%200000&method=CRC-16/MODBUS&datatype=hex&outtype=hex
        */

      int16_t result;

      uint16_t crc;
      int16_t i;

      crc = CrcOld;
      crc ^= Byte;

      for (i = 8; i != 0; i--)
      {
        if ((crc & 0x0001) != 0)
        {
          crc >>= 1;
          crc ^= 0xA001;
        }
        else
        {
          crc >>= 1;
        }
      }

      result = crc;

      return result;
    }
};

#endif

QString ParseHex::ParseFile(const QString & file, ParseHex::QStringsMap & adressesAndCommands)
{
    // https://microsin.net/programming/pc/intel-hex-file-format.html?ysclid=m6ixn3tdg4403881195

    auto readRes = MyQFileDir::ReadFile2(file);
    if(!readRes.success) { return "Error reading file"; }

    QStringList lines = readRes.content.split("\n");
    for(auto &line:lines) if(line.endsWith('\r')) line.chop(1);

    int line_n = 1;
    const QString dataMarker = "00";
    const QString adressHmarker = "04";
    QString adressH;
    QString data;

    for(auto &line:lines)
    {
        if(!CheckSumForHexRow(line))
            return "checksum error on line #" + QSn(line_n) + " ("+line+")";

        QString byteSize = line.mid(1,2);
        //qdbg << byteSize;
        QString adressL = line.mid(3,4);
        //qdbg << adressL;
        QString rowType = line.mid(7,2);
        //qdbg << rowType;
        QString adressH_or_data = line.mid(9,byteSize.toUShort(nullptr,16)*2);
        //qdbg << adressHorData;
        QString ks = line.right(2);
        //qdbg << ks;

        if(rowType == dataMarker) data = adressH_or_data;
        else if(rowType == adressHmarker) adressH = adressH_or_data;
        else if(rowType == "01") break;
        else
            return "wrong row type ["+rowType+"] on line #" + QSn(line_n) + " ("+line+")";

        if(rowType == dataMarker)
        {
            bool needNextAdress = false;
            QString prevAdress;
            while(!data.isEmpty())
            {
                if(data.size() < 8)
                    return "wrong data field, size = "+QSn(data.size())+" on line #" + QSn(line_n) + " ("+line+")";

                QString dataFragment = data.left(6);
                data.remove(0,8);

                QChar temp1 = dataFragment[0];
                QChar temp2 = dataFragment[1];
                dataFragment[0] = dataFragment[4];
                dataFragment[1] = dataFragment[5];
                dataFragment[4] = temp1;
                dataFragment[5] = temp2;

                QString adress;
                if(!needNextAdress) adress = QSn((adressH + adressL).toUInt(nullptr, 16) >> 1, 16); // сдвиг вправо = деление на 2
                else adress = QSn(prevAdress.toUInt(nullptr, 16) + 2, 16);
                while(adress.size() < 6) adress.prepend('0');

                adressesAndCommands[adress] = std::move(dataFragment);

                prevAdress = adress;
                needNextAdress = true;
            }
        }

        line_n++;
    }
    return "";
}

bool ParseHex::CheckSumForHexRow(const QString & line)
{
    if(line.size() % 2 != 1) return false;
    uchar crcacc = 0;
    for(int i=1; i<line.size(); i+=2)
        crcacc += QStringRef(&line,i,2).toUShort(nullptr,16);
    return crcacc == 0;
}
