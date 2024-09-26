#ifndef CodeMarkers_H
#define CodeMarkers_H

#include <QString>

struct CodeMarkers
{
    inline static const QString mock {"MOCK"};
    inline static const QString legacy {"LEGACY"};
    inline static const QString needOptimisation {"NEED OPTIMISATION"};
    inline static const QString note {"note"};
    inline static const QString toDo {"toDo"};

    static void CanBeOptimized([[maybe_unused]] const char* comment) {}
};

#endif
