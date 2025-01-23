#include "MyQWindows.h"

#include <windows.h>

#include <iostream>

#include <QDateTime>
#include <QDebug>

#include "MyQShortings.h"

QDateTime MyQWindows::GetProcessStartTime(uint processID) {
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

bool MyQWindows::IsProcessRunning(uint processID) {
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
