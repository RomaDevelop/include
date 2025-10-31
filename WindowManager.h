//===============================================================================================================================
/*
 * Класс WindowManager предназначен для огранизации своевременного удаления окон пользовательских типов
 *
 * Использование:
 * В конструкторе окна размещаем вызов RegisterWindow(this)
 * В деструкторе окна развмещаем вызов UnregisterWindow(this)
 * В произвольном месте вызываем WindowManager<T>::DeleteAllWindows() для корректного уничтожения всех окон T
 *
 * Окну можно устанавливать атрибут Qt::WA_DeleteOnClose или родителя
 *
 * Логика работы:
 * При вызове WindowManager<T>::RegisterWindow WindowManager<T> принимает владение памятью окна T через unique_ptr
 * Никто больше не должен владеть памятью окна!!!
 * При вызове WindowManager<T>::UnregisterWindow WindowManager<T> прекращает владение памятью окна T
 */
//===============================================================================================================================
#ifndef WindowManager_H
#define WindowManager_H
//-------------------------------------------------------------------------------------------------------------------------------
#include <map>

#include <QDebug>
#include <QWidget>

#include "MyCppDifferent.h"
//--------------------------------------------------------------------------------------------------------------------------------
///\brief WindowManager предназначен для огранизации своевременного удаления окон пользовательских типов
template <typename T>
class WindowManager {
public:
	///\brief хранилище окон
	static inline std::map<T*, std::unique_ptr<T>> existingWindows;

	///\brief Нужно вызвать в конструкторе окна T
	/// WindowManager<T> принимает владение памятью окна T через unique_ptr
	static void RegisterWindow(T* window)
	{
		existingWindows[window] = std::unique_ptr<T>(window);
	}

	///\brief Нужно вызвать в деструкторе окна T
	/// WindowManager<T> прекращает владение памятью окна T
	static void UnregisterWindow(T* window)
	{
		if(doClearNow == true) return;

		auto it = existingWindows.find(window);
		if (it != existingWindows.end())
		{
			it->second.release();
			existingWindows.erase(it);
			/// когда окно уничтожется, потому что его закрыли или уничтожается его родитель,
			/// вызывается его деструктор, а в нём UnregisterWindow,
			/// WindowManager прекращает владение и удаляет из хранилища
		}
		else
		{
			qCritical() << "Удаляемое окно не найдено в existingWindows";
		}
	}

	///\brief Удаление всех окон T
	static void DeleteAllWindows()
	{
		MyCppDifferent::any_guard guard(doClearNow, true, false);
		existingWindows.clear();
	}

private:
	///\brief флаг блокирующий двойное удаление из хранилища
	/// вызов DeleteAllWindows инициирует работу деструкторов,
	/// а в деструкторах вызывается UnregisterWindow, что приведет к повторному удалению узла хранилища
	inline static bool doClearNow = false;
};

//--------------------------------------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------------------------------------
