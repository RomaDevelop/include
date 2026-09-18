//--------------------------------------------------------------------------------------------------------------------------
#ifndef MyQMPCPlaylist_H
#define MyQMPCPlaylist_H
//--------------------------------------------------------------------------------------------------------------------------
#include "MyQShortings.h"
#include "MyQString.h"
#include "MyQFileDir.h"
#include "declare_struct.h"
//--------------------------------------------------------------------------------------------------------------------------
struct MyQMPCPlaylist
{
	inline static bool CreateMPCPlaylist(QStringList files, QString save_as_name);
	inline static bool CreateMPCPlaylist(QStringList files, QStringList labels, QString save_as_name);
	declare_struct_4_fields_move(NamePath, QString, name, QString, path, QString, pathName, QString, label);
	declare_struct_2_fields_move(ParseRes, QString, errors, std::vector<NamePath>, files);
	inline static ParseRes ReadMPCPlaylist(QString fileName);
};

namespace keyWords_ns {
	const char *MPCPLAYLIST = "MPCPLAYLIST";
	const char *filename = "filename";
	const char *label = "label";
	const char *type = "type";
}

//--------------------------------------------------------------------------------------------------------------------------

bool MyQMPCPlaylist::CreateMPCPlaylist(QStringList files, QString save_as_name)
{
	return CreateMPCPlaylist(files, {}, save_as_name);
}

bool MyQMPCPlaylist::CreateMPCPlaylist(QStringList files, QStringList labels, QString save_as_name)
{
	QString playlist = keyWords_ns::MPCPLAYLIST;
	playlist.append("\n");
	for(int i=0; i<files.size(); i++)
	{
		QString number = QSn(i+1);
		playlist.append(number + ",type,0\n");
		playlist.append(number + "," + keyWords_ns::filename + "," + files[i] + "\n");

		if(i < labels.size())
			playlist.append(number + "," + keyWords_ns::label + "," + labels[i] + "\n");
	}

	return MyQFileDir::WriteFile(save_as_name, playlist);
}

MyQMPCPlaylist::ParseRes MyQMPCPlaylist::ReadMPCPlaylist(QString fileName)
{
	ParseRes res;
	auto readRes = MyQFileDir::ReadFile2(fileName);
	if(!readRes.success) { res.errors = "error reading file " + fileName+"\n"; return res; }

	readRes.content.remove('\r');

	QStringList rows = readRes.content.split('\n');
	if(rows.isEmpty() || rows[0] != keyWords_ns::MPCPLAYLIST)
	{
		if(readRes.content.size() > 300)
		{
			readRes.content.chop(297);
			readRes.content += "...";
		}
		res.errors = "wrong format file " + fileName + "\n\nReaded content:\n" + readRes.content;
		return res;
	}

	std::map<int, NamePath> items;
	for(int i=0; i<rows.size(); i++)
	{
		if(i == 0 and rows[i] == keyWords_ns::MPCPLAYLIST) continue;
		if(rows[i].isEmpty()) continue;

		auto words = rows[i].split(',');
		if(words.size() <= 2) { res.errors += "error in row " + QSn(i+1) + ", too short row\n"; continue; }

		bool ok;
		QString &numberStr = words[0];
		int number = numberStr.toInt(&ok);
		if(not ok)  { res.errors += "error in row " + QSn(i+1) + ", not starts with number\n"; continue; }

		QString& rowType = words[1];
		auto &item = items[number];

		if(0) ;
		else if(rowType == keyWords_ns::type)
		{
			rows[i].remove(0, numberStr.size()+1+rowType.size()+1);
			if(rows[i].isEmpty()) { res.errors += "error in row " + QSn(i+1) + ", empty after remove number and type\n"; continue; }

			if(rows[i] != "0") { res.errors += "error in row " + QSn(i+1) + ", unexpected type value "+rows[i]+"\n"; continue; }
		}
		else if(rowType == keyWords_ns::filename)
		{
			rows[i].remove(0, numberStr.size()+1+rowType.size()+1);
			if(rows[i].isEmpty()) { res.errors += "error in row " + QSn(i+1) + ", empty after remove number and type\n"; continue; }

			QFileInfo fileInfo(rows[i]);
			item.name = fileInfo.fileName();
			item.path = fileInfo.path();
			item.pathName = fileInfo.filePath();
		}
		else if(rowType == keyWords_ns::label)
		{
			rows[i].remove(0, numberStr.size()+1+rowType.size()+1);
			if(rows[i].isEmpty()) { res.errors += "error in row " + QSn(i+1) + ", empty after remove number and type\n"; continue; }

			item.label = rows[i];
		}
		else res.errors += "error in row " + QSn(i+1) + ", unexpected type "+rowType+"\n";
	}

	for(auto &node:items)
	{
		auto &item = node.second;
		if(item.name.isEmpty() or item.path.isEmpty() or item.pathName.isEmpty())
		{
			res.errors += "error in record number " + QSn(node.first) + ", file data not set\n";
			continue;
		}
		res.files.emplace_back(std::move(item));
	}

	return res;
}

//--------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------
