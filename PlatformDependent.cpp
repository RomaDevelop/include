#include "PlatformDependent.h"

//#undef _WIN32

#ifndef _WIN32
#error PlatformDependent is implemented only for Windows.
#endif

#include <windows.h>

#include <QApplication>
#include <QDebug>

#include "MyQShortings.h"

QDateTime PlatformDependent::GetProcessStartTime(uint processID) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)processID);
	if (hProcess == NULL) {
		auto err = GetLastError();
		qdbg << "PlatformDependent::GetProcessStartTime: Не удалось открыть процесс с PID "
				+ QSn(processID) + ". Ошибка: " + QSn(err);
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
	}
	else
	{
		auto err = GetLastError();
		qdbg << "PlatformDependent::GetProcessStartTime: Не удалось получить время процесса. Ошибка: "
				+ QSn(err);
		CloseHandle(hProcess);
		return QDateTime(); // Возвращаем пустой QDateTime в случае ошибки
	}
}

bool PlatformDependent::IsProcessRunning(uint processID) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	if(hProcess != NULL)
	{
		DWORD exitCode;
		if(GetExitCodeProcess(hProcess, &exitCode)) // Получаем код завершения процесса
		{
			CloseHandle(hProcess); // Закрываем дескриптор
			return (exitCode == STILL_ACTIVE); // Если код завершения равен STILL_ACTIVE, процесс все еще работает
		}
		else
		{
			auto err = GetLastError();
			qdbg << "IsProcessRunning::GetExitCodeProcess error: " + QSn(err);
		}
		CloseHandle(hProcess); // Закрываем дескриптор в случае ошибки
	}
	else
	{
		auto err = GetLastError();
		if(err == ERROR_INVALID_PARAMETER) ; // ok, means not running
		else qdbg << "IsProcessRunning::OpenProcess unknown error " + QSn(err);
	}
	return false; // Процесс не запущен или не может быть открыт
}

PlatformDependent::CopyMoveFileRes PlatformDependent::CopyMoveFile(QString SourceFile, QString Destination, CopyMoveFileMode Mode)
{
	SourceFile.replace('/','\\');
	Destination.replace('/','\\');
	wchar_t cFrom[MAX_PATH] = {0};
	wcscpy(cFrom, SourceFile.toStdWString().c_str());
	wchar_t cTo[MAX_PATH] = {0};
	wcscpy(cTo, Destination.toStdWString().c_str());
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

	CopyMoveFileRes res;
	res.errorCode = SHFileOperation(&fos);

	if(res.errorCode == 0) res.success = true;
	else
	{
		res.success = false;
		res.errorText = "Не удалось переместить файл\n\n"+SourceFile+
				"\n\nв\n\n"+Destination+"!\n\nКод ошибки SHFileOperation: "+ QSn(res.errorCode);
		if(Mode == copy)
			res.errorText.replace("переместить", "скопировать");
	}

	return res;
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

void PlatformDependent::FlashClickOnTitle(QWidget *w)
{
	// Получаем дескриптор окна
	HWND hwndTool = reinterpret_cast<HWND>(w->winId());

	static int captionHeight = GetSystemMetrics(SM_CYCAPTION);

	// Получаем координаты заголовка окна
	RECT rect;
	GetWindowRect(hwndTool, &rect);
	int x = (rect.left + rect.right) / 2; // Центр окна по X
	int y = rect.top + captionHeight / 2; // Центр высоты заголовка окна

	// Запоминаем текущие координаты мыши
	POINT currentPos;
	GetCursorPos(&currentPos);

	// Перемещаем курсор мыши в нужную позицию
	SetCursorPos(x, y);

	// Эмулируем клик мышью
	INPUT input {};
	input.type = INPUT_MOUSE;
	input.mi.dx = 0;
	input.mi.dy = 0;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &input, sizeof(INPUT)); // Нажатие кнопки мыши

	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(INPUT)); // Отпускание кнопки мыши

	// Возвращаем курсор мыши на исходные координаты
	SetCursorPos(currentPos.x, currentPos.y);
}

void PlatformDependent::ShowPropertiesWindow(const QString &file)
{
	SHELLEXECUTEINFO sei = {};
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_INVOKEIDLIST;
	sei.lpVerb = L"properties";
	sei.lpFile = reinterpret_cast<LPCWSTR>(file.utf16());
	sei.nShow = SW_SHOW;
	ShellExecuteEx(&sei);
}




