//---------------------------------------------------------------------------
#ifndef MYQDOM
#define MYQDOM
//---------------------------------------------------------------------------
#include <vector>
//#include <functional>
#include <QDebug>
#include <QFile>
#include <QTextCodec>
#include <QDomDocument>

#include "MyQFileDir.h"
//---------------------------------------------------------------------------
struct MyQDom
{
    inline static std::vector<QDomElement> GetTopLevelElements(const QDomNode &node);
    inline static std::vector<QDomElement> GetAllLevelElements(const QDomNode &node);
    inline static std::vector<QDomElement> GetAllLevelElements_if(const QDomNode &node, std::function<bool(const QDomNode &node)> condition);
    inline static std::vector<std::pair<QString,QString>> Attributes(const QDomElement &element);
    inline static QStringList AttributesNames(const QDomElement &element);
    inline static QStringList AttributesValues(const QDomElement &element);
    inline static QDomElement FirstChildIncludeSubChilds(const QDomNode &node, const QString &tagName);
    inline static QDomElement FirstChildIncludeSubChilds(const QDomNode &node, const std::pair<QString,QString> &attribute);
    inline static void ReplaceInAttributes(QDomElement &element, const QString &replaceWhat, const QString &replaceTo);
    inline static QString ToString(const QDomElement &element);
    inline static QString ToStringIgnoreNested(const QDomElement &element, int truncateAttrs = -1);
    inline static QDomDocument FromFile(const QString &fileName, const char * encoding = "UTF-8")
    {
        QDomDocument doc;
        auto readRes = MyQFileDir::ReadFile2(fileName, encoding);
        if(!readRes.success)  qCritical() << "MyQDom::FromFile ReadFile2 unsuccess";
        else if(!doc.setContent(readRes.content))  qCritical() << "MyQDom::FromFile setContent returned false";
        return doc;
    }
    inline static bool ToFile(const QDomNode &node, const QString &fileName, const char * encoding = "UTF-8")
    {
        QFile file(fileName);
        if(file.open(QFile::WriteOnly))
        {
            QTextStream stream(&file);
            if(encoding != nullptr && strcmp(encoding, "") != 0)
            {
                if(auto codec = QTextCodec::codecForName(encoding)) stream.setCodec(codec);
                else
                {
                    qCritical() << QString("MyQDom::ToFile unknown codec [") +encoding+"]";
                    return false;
                }
            }

            node.save(stream, 2);
            return true;
        }
        qCritical() << "MyQDom::ToFile can't open file ["+fileName+"]";
        return false;
    }
};

std::vector<QDomElement> MyQDom::GetTopLevelElements(const QDomNode & node)
{
    std::vector<QDomElement> elemtnts;
    QDomNode childNode = node.firstChild();
    while(!childNode.isNull())
    {
	QDomElement e = childNode.toElement(); // try to convert the node to an element.
	if(!e.isNull())
	{
	    elemtnts.push_back(e);
	}
	childNode = childNode.nextSibling();
    }
    return elemtnts;
}

std::vector<QDomElement> MyQDom::GetAllLevelElements(const QDomNode & node)
{
    return GetAllLevelElements_if(node, nullptr);
}

std::vector<QDomElement> MyQDom::GetAllLevelElements_if(const QDomNode & node, std::function<bool (const QDomNode &)> condition)
{
    std::vector<QDomElement> elements;

    if(node.isElement())
    {
        if(!condition || condition(node)) elements.push_back(node.toElement());
    }

    QDomNode childNode = node.firstChildElement();
    if(node.isDocument()) childNode = node.toDocument().documentElement().firstChildElement();
    while(!childNode.isNull())
    {
	auto nestedElementsOfChild = GetAllLevelElements_if(childNode, condition);
	std::move(nestedElementsOfChild.begin(), nestedElementsOfChild.end(), std::back_inserter(elements));

	childNode = childNode.nextSibling();
    }

    return elements;
}

std::vector<std::pair<QString, QString> > MyQDom::Attributes(const QDomElement & element)
{
    std::vector<std::pair<QString,QString>> attrsVector;
    auto attrs = element.attributes();
    int size = attrs.size();
    for(int i=0; i<size; i++)
    {
	auto attr = attrs.item(i).toAttr();
	attrsVector.push_back({attr.name(),attr.value()});
    }
    return attrsVector;
}

QStringList MyQDom::AttributesNames(const QDomElement & element)
{
    QStringList attrsNames;
    auto attrs = element.attributes();
    int size = attrs.size();
    for(int i=0; i<size; i++)
    {
	auto attr = attrs.item(i).toAttr();
	attrsNames.push_back(attr.name());
    }
    return attrsNames;
}

QStringList MyQDom::AttributesValues(const QDomElement & element)
{
    QStringList attrsVals;
    auto attrs = element.attributes();
    int size = attrs.size();
    for(int i=0; i<size; i++)
    {
	auto attr = attrs.item(i).toAttr();
	attrsVals.push_back(attr.value());
    }
    return attrsVals;
}

QDomElement MyQDom::FirstChildIncludeSubChilds(const QDomNode & node, const QString & tagName)
{
    QDomElement res;
    auto childs = GetTopLevelElements(node);
    for(auto &child:childs)
    {
	if(child.tagName() == tagName)
	{
	    res = child;
	}
	if(child.hasChildNodes())
	{
	    res = FirstChildIncludeSubChilds(child,tagName);
	}
	if(!res.isNull()) break;
    }
    return res;
}

QDomElement MyQDom::FirstChildIncludeSubChilds(const QDomNode & node, const std::pair<QString, QString> & attribute)
{
    QDomElement res;
    auto childs = GetTopLevelElements(node);
    for(auto &child:childs)
    {
	if(child.attribute(attribute.first) == attribute.second)
	{
	    res = child;
	}
	if(child.hasChildNodes())
	{
	    res = FirstChildIncludeSubChilds(child,attribute);
	}
	if(!res.isNull()) break;
    }
    return res;
}

void MyQDom::ReplaceInAttributes(QDomElement & element, const QString & replaceWhat, const QString & replaceTo)
{
    auto attributes = element.attributes();
    int size = attributes.size();
    for(int attr_i=0; attr_i<size; attr_i++)
    {
	auto attr = attributes.item(attr_i).toAttr();
	QString value = attr.value();
	if(value.contains(replaceWhat))
	    attr.setValue(value.replace(replaceWhat, replaceTo));
    }
}

QString MyQDom::ToString(const QDomElement & element)
{
    QString str;
    QTextStream stream(&str);
    element.save(stream,2);
    return str;
}

QString MyQDom::ToStringIgnoreNested(const QDomElement & element, int truncateAttrs)
{
    QString ret;
    auto attrs = Attributes(element);
    for(auto &attr:attrs)
    {
	if(truncateAttrs < 0)
	    ret += attr.first + "=\"" + attr.second + "\" ";
	else
	{
	    bool needTruncate = truncateAttrs < attr.second.size();
	    attr.second.truncate(truncateAttrs);
	    if(needTruncate) attr.second += "...";
	    ret += attr.first + "=\"" + attr.second + "\" ";
	}
    }
    ret.chop(1);

    ret = "<"+element.tagName()+" "+ret;
    if(element.childNodes().isEmpty()) ret += "/>";
    else ret += ">";
    return ret;
}

//---------------------------------------------------------------------------
#endif
