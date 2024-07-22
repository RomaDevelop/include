//---------------------------------------------------------------------------
#ifndef MYQDOM
#define MYQDOM
//---------------------------------------------------------------------------
#include <vector>
#include <QDebug>
#include <QFile>
#include <QDomDocument>
//---------------------------------------------------------------------------
struct MyQDom
{
    inline static std::vector<QDomElement> GetTopLevelElements(const QDomNode &node);
    inline static std::vector<QDomElement> GetAllLevelElements(const QDomNode &node);
    inline static std::vector<std::pair<QString,QString>> GetAttributes(const QDomElement &element);
    inline static QDomElement FirstChildIncludeSubChilds(const QDomNode &node, const QString &tagName);
    inline static QDomElement FirstChildIncludeSubChilds(const QDomNode &node, const std::pair<QString,QString> &attribute);
    inline static QString ToString(const QDomElement &element);
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
    std::vector<QDomElement> elements;
    QDomNode childNode = node.firstChild();
    while(!childNode.isNull())
    {
	QDomElement childElement = childNode.toElement(); // try to convert the node to an element.
	if(!childElement.isNull())
	{
	    elements.push_back(childElement);
	    if(childElement.hasChildNodes())
	    {
		auto nestedElementsOfChild = GetAllLevelElements(childNode);
		std::move(nestedElementsOfChild.begin(), nestedElementsOfChild.end(), std::back_inserter(elements));
	    }
	}
	childNode = childNode.nextSibling();
    }
    return elements;
}

std::vector<std::pair<QString, QString> > MyQDom::GetAttributes(const QDomElement & element)
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

QString MyQDom::ToString(const QDomElement & element)
{
    QString str;
    QTextStream stream(&str);
    element.save(stream,2);
    return str;
}
//---------------------------------------------------------------------------
#endif
