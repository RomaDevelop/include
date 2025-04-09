#include "PlatformDependent.h"

#include <windows.h>

#include <iostream>

#include <QApplication>
#include <QDateTime>
#include <QWidget>
#include <QDebug>

#include "MyQShortings.h"

QDateTime PlatformDependent::GetProcessStartTime(uint processID) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)processID);
	if (hProcess == NULL) {
		std::cerr << "Не удалось открыть процесс с PID " << processID << ". Ошибка: " << GetLastError() << std::endl;
		return QDateTime(); // Возвращаем пустой QDateTime в случае ошибки
	}

	FILETIME creationTime, exitTime, kernelTime, userTime;

	if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
		// Преобразуем FILETIME в QDateTime
		ULARGE_INTEGER ull;
		ull.LowPart = creationTime.dwLowDateTime;
		ull.HighPart = creationTime.dwHighDateTime;

		// FILETIME представляет время в 100-наносекундных интервалах с 1 января 1601 года
		// Преобразуем в QDateTime
		QDateTime startTime = QDateTime::fromMSecsSinceEpoch(ull.QuadPart / 10000 - 11644473600000LL);
		CloseHandle(hProcess);
		return startTime;
	} else {
		std::cerr << "Не удалось получить время процесса. Ошибка: " << GetLastError() << std::endl;
		CloseHandle(hProcess);
		return QDateTime(); // Возвращаем пустой QDateTime в случае ошибки
	}
}

bool PlatformDependent::IsProcessRunning(uint processID) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	if(hProcess != NULL)
	{
		DWORD exitCode;
		// Получаем код завершения процесса
		if(GetExitCodeProcess(hProcess, &exitCode))
		{
			CloseHandle(hProcess); // Закрываем дескриптор
			// Если код завершения равен STILL_ACTIVE, процесс все еще работает
			return (exitCode == STILL_ACTIVE);
		}
		else
		{
			std::cerr << "GetExitCodeProcess error: " << GetLastError() << std::endl;
		}
		CloseHandle(hProcess); // Закрываем дескриптор в случае ошибки
	}
	else
	{
		std::cerr << "OpenProcess error: " << GetLastError() << std::endl;
	}
	return false; // Процесс не запущен или не может быть открыт
}

int PlatformDependent::CopyMoveFile(QString S, QString D, CopyMoveFileMode Mode)
{
	S.replace('/','\\');
	D.replace('/','\\');
	wchar_t cFrom[MAX_PATH] = {0};
	wcscpy(cFrom, S.toStdWString().c_str());
	wchar_t cTo[MAX_PATH] = {0};
	wcscpy(cTo, D.toStdWString().c_str());
	SHFILEOPSTRUCT fos;
	memset(&fos, 0, sizeof(SHFILEOPSTRUCT));
	if(QApplication::activeWindow())
		fos.hwnd = (HWND)QApplication::activeWindow()->winId();
	else if(GetConsoleWindow())
		fos.hwnd = GetConsoleWindow();
	else fos.hwnd = nullptr;
	fos.wFunc = Mode == move ? FO_MOVE : FO_COPY;
	fos.pFrom = cFrom;
	fos.pTo = cTo;
	fos.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | FOF_SIMPLEPROGRESS;
	return SHFileOperation(&fos);
}

void PlatformDependent::SetTopMost(QWidget * w, bool topMost)
{
	HWND hwnd = reinterpret_cast<HWND>(w->winId());
	SetWindowPos(hwnd, topMost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
				 SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
}

void PlatformDependent::SetTopMostFlash(QWidget *w)
{
	SetTopMost(w, true);
	SetTopMost(w, false);
}
