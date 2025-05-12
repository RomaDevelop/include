//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MyQSplitter_h
#define MyQSplitter_h
//------------------------------------------------------------------------------------------------------------------------------------------
#include <QSplitter>
#include <QTimer>
#include <QDebug>
//------------------------------------------------------------------------------------------------------------------------------------------
struct MyQSplitter
{
    inline static bool SetSize(QSplitter* splitter, QWidget *widget, int size);
};
//------------------------------------------------------------------------------------------------------------------------------------------

bool MyQSplitter::SetSize(QSplitter * splitter, QWidget *widget, int size)
{
    int index = splitter->indexOf(widget);
    if(index != -1)
    {
	auto sizes = splitter->sizes();
	int difference = size - sizes[index];
	sizes[index] = size;
	for(int i=0; i<sizes.size(); i++)
	{
	    if(i == index) continue;

	    sizes[i] -= difference / (sizes.size()-1);
	}
	splitter->setSizes(sizes);
	return true;
    }

    return false;
}

#endif
//------------------------------------------------------------------------------------------------------------------------------------------



