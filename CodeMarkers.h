#ifndef CodeMarkers_H
#define CodeMarkers_H

#include <QString>

struct CodeMarkers
{
    inline static const QString mock {"mock"};
    inline static const QString deprecated {"deprecated"};
    inline static const QString retired {"retired"};
    inline static const QString note {"note"};

    static void to_do(QString) {}
	static void to_do_with_cpp20(QString) {}
    static void can_be_optimized(QString) {}
    static void need_optimisation(QString) {}

};

#endif
