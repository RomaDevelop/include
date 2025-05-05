//---------------------------------------------------------------------------------------------------------------------------
#ifndef MyQMPCPlaylist_H
#define MyQMPCPlaylist_H
//---------------------------------------------------------------------------------------------------------------------------

#include "MyQShortings.h"
#include "MyQFileDir.h"
#include "declare_struct.h"
//---------------------------------------------------------------------------------------------------------------------------
struct MyQMPCPlaylist
{
	inline static bool CreateMPCPlaylist(QStringList files, QString save_as_name);
	declare_struct_3_fields_move(NamePath, QString, name, QString, path, QString, pathName);
	declare_struct_2_fields_move(ParseRes, QString, errors, std::vector<NamePath>, files);
	inline static ParseRes ReadMPCPlaylist(QString fileName);
};

namespace keyWords_ns {
	const char *MPCPLAYLIST = "MPCPLAYLIST";
	const char *filename = "filename";
}

//---------------------------------------------------------------------------------------------------------------------------

bool MyQMPCPlaylist::CreateMPCPlaylist(QStringList files, QString save_as_name)
{
	QString playlist = keyWords_ns::MPCPLAYLIST;
	playlist.append("\n");
	for(int i=0; i<files.size(); i++)
	{
		QString number=QSn(i+1);
		playlist.append(number+",type,0\n");
		playlist.append(number+","+keyWords_ns::filename+","+files[i]+"\n");
	}

	return MyQFileDir::WriteFile(save_as_name, playlist);
}

MyQMPCPlaylist::ParseRes MyQMPCPlaylist::ReadMPCPlaylist(QString fileName)
{
	ParseRes res;
	auto readRes = MyQFileDir::ReadFile2(fileName);
	if(!readRes.success) { res.errors = "error reading file " + fileName; return res; }

	readRes.content.remove('\r');

	QStringList rows = readRes.content.split('\n');
	if(rows.isEmpty() || rows[0] != "MPCPLAYLIST")
	{
		if(readRes.content.size() > 300)
		{
			readRes.content.chop(297);
			readRes.content += "...";
		}
		res.errors = "wrong format file " + fileName + "\n\nReaded content:\n" + readRes.content;
		return res;
	}

	int comma;
	for(int i=2; i<rows.size(); i+=2)
	{
		comma = rows[i].indexOf(',');

		if(comma == -1) { res.errors += "error in row " + QSn(i) + ", not found comma\n"; continue; }
		rows[i].remove(0, comma+1);

		comma = rows[i].indexOf(',');
		if(comma == -1) { res.errors += "error in row " + QSn(i) + ", not found second comma\n"; continue; }

		QStringRef type(&rows[i],0, comma);
		if(type != keyWords_ns::filename) res.errors += "error in row " + QSn(i) + ", type not "+keyWords_ns::filename+"\n";

		rows[i].remove(0, comma+1);

		if(rows[i].isEmpty()) { res.errors += "error in row " + QSn(i) + ", empty after second comma\n"; continue; }

		QString &file = rows[i];
		QFileInfo fileInfo = file;

		res.files.emplace_back(NamePath(fileInfo.fileName(), fileInfo.path(), file));
	}

	return res;
}

//---------------------------------------------------------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------------------------------------------------------
