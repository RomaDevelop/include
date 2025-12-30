#ifndef PlatformDependent_H
#define PlatformDependent_H

#include <QString>
#include <QDateTime>
#include <QWidget>

#include "declare_struct.h"

struct Thumbnail
{
	struct CreateThumbnailRes { QString error; QImage image; };
	static CreateThumbnailRes CreateThumbnail(const QString& file_to_get_prvw);
	/// требуется подключение dll:
	/// LIBS += -luuid
	/// ИИ писал, что нужны -lshell32 и -lole32 не понадобились
};

#endif
