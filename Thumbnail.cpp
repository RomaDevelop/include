#include "Thumbnail.h"

//#undef _WIN32

#ifndef _WIN32
#error Thumbnail is implemented only for Windows.
#endif

#include <shlobj.h>

#include <QtWin>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

#include "MyQShortings.h"

Thumbnail::CreateThumbnailRes CreateErrRes(QString text, HRESULT &hr)
{
	wchar_t* msgBuf = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  (LPWSTR)&msgBuf, 0, NULL);

	QString msg = QString::fromWCharArray(msgBuf);
	LocalFree(msgBuf);

	return {text+"; error code: "+QString("0x%1").arg(static_cast<quint32>(hr), 8, 16, QChar('0'))
					+ " msg: "+msg,{}};
}

Thumbnail::CreateThumbnailRes Thumbnail::CreateThumbnail(const QString & file_to_get_prvw)
{
	QFileInfo fiToGet(file_to_get_prvw);
	if(fiToGet.isFile() == false) { return {file_to_get_prvw+" is not file",{}}; }
	std::wstring Folder = QDir::toNativeSeparators(fiToGet.path()).toStdWString();
	std::wstring FileName = fiToGet.fileName().toStdWString();

	IShellFolder* pDesktop = NULL;
	IShellFolder* pSub = NULL;
	IExtractImage* pIExtract = NULL;
	LPITEMIDLIST pidl = NULL;

	HRESULT hr;
	hr = SHGetDesktopFolder(&pDesktop);
	if(FAILED(hr)) { return CreateErrRes("SHGetDesktopFolder fail", hr); }
	hr = pDesktop->ParseDisplayName(NULL, NULL, (LPWSTR)Folder.data(), NULL, &pidl, NULL);
	if(FAILED(hr)) { return CreateErrRes("pDesktop->ParseDisplayName fail", hr); }
	hr = pDesktop->BindToObject(pidl, NULL, IID_IShellFolder, (void**)&pSub);
	if(FAILED(hr)) { return CreateErrRes("BindToObject fail", hr); }
	hr = pSub->ParseDisplayName(NULL, NULL, (LPWSTR)FileName.c_str(), NULL, &pidl, NULL);
	if(FAILED(hr)) { return CreateErrRes("pSub->ParseDisplayName 2 fail", hr); }
	hr = pSub->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST *)&pidl, IID_IExtractImage, NULL, (void**)& pIExtract);
	if(FAILED(hr)) { return CreateErrRes("GetUIObjectOf fail", hr); }

	SIZE size;
	size.cx =512; // ранее было 320*320, по совету gpt поставил 512, качество стало по лучше
	size.cy =512;

	DWORD dwFlags = IEIFLAG_ORIGSIZE | IEIFLAG_QUALITY; // для качества gpt советовал другие флаги пробовать, но ничего не менялось

	HBITMAP hThumbnail = NULL;

	// Set up the options for the image
	OLECHAR pathBuffer[MAX_PATH];
	hr = pIExtract->GetLocation(pathBuffer, MAX_PATH, NULL, &size,32, &dwFlags);
	if(FAILED(hr)) { return CreateErrRes("GetLocation fail", hr); }

	// Get the image
	hr = pIExtract->Extract(&hThumbnail);
	if(FAILED(hr)) { return CreateErrRes("Extract fail", hr); }

	pIExtract->Release();
	pDesktop->Release();
	pSub->Release();

	if (!hThumbnail) { return {"hThumbnail is null",{}}; }

	auto image = QtWin::imageFromHBITMAP(hThumbnail);
	if (image.isNull()) { return {"image is null",{}}; }

	return {"", std::move(image)};
}




