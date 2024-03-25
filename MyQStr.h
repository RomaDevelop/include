//---------------------------------------------------------------------------
#ifndef MYQSTRH
#define MYQSTRH
//---------------------------------------------------------------------------
#include <QString>
#include <QStringList>
#include <QTextEdit>
//---------------------------------------------------------------------------
namespace MyStr
{
    // велосипед QString::split
//    QString GetElemetFromStrings(QString StringAsList, QString endString, int Index)
//    {
//        if(endString=="") return "GetElemetFromList: endString не должно быть пусто";
//        QString ret_str;
//        if(StringAsList.mid(StringAsList.length()-endString.length(),endString.length()) != endString)
//            ret_str="GetElemetFromList: нет завершения списка "+StringAsList+" ("+endString+") в конце!";
//        else
//        {
//            int cur_ind=0;
//            bool find=false;
//            while(StringAsList.indexOf(endString) != -1)
//            {
//                ret_str=StringAsList.left(StringAsList.indexOf(endString));
//                if(cur_ind==Index) { find=true; break; }
//                cur_ind++;
//                StringAsList.remove(0,StringAsList.indexOf(endString)+endString.length());
//            }
//            if(!find)
//                ret_str="GetElemetFromList: нет элемента с индексом "+QString::number(Index)+"!";
//        }
//        return ret_str;
//    }

    // велосипед QString::split
//    QStringList EditToStrList(QTextEdit *edit)
//    {
//        QString str {edit->toPlainText()};
//        if(str[str.length()-1] != '\n') str += '\n';
//        QStringList strList;
//        while(str.length())
//        {
//            int pos = str.indexOf("\n");
//            if(pos != -1)
//            {
//                strList += str.left(pos);
//                str.remove(0,pos+1);
//            }
//            else break;
//        }
//        return strList;
//    }

    // велосипед QString::split
//    QStringList StringToStrList(QString text, QString splitter)
//    {
//        text.split()
//        if(text.left(splitter.length()) != splitter) text += splitter;
//        QStringList strList;
//        while(text.length())
//        {
//            int pos = text.indexOf("\n");
//            if(pos != -1)
//            {
//                strList += text.left(pos);
//                text.remove(0,pos+1);
//            }
//            else break;
//        }
//        return strList;
//    }
}
//---------------------------------------------------------------------------
#endif
