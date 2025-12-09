//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MyCDifferent_H
#define MyCDifferent_H
//------------------------------------------------------------------------------------------------------------------------------------------

#include <cstring>
#include <stddef.h>

struct MyCDifferent
{
	///\brief Преобразует число num в бинарную строку.
	/// Строка будет длиной в соответсвии с битовой размерностью типа числа.
	/// Старшие неиспользуемые биты заполняются нулями.
	template<typename T>
	static void NumberToBinaryString(T num, char *bufStr) {
		const int bitSize = sizeof(T) * 8;

		bufStr[bitSize] = '\0';

		for (int i = bitSize-1; i >= 0; --i) {
			bufStr[i] = (num & 1) ? '1' : '0'; // Получение последнего бита
			num >>= 1; // Сдвиг числа вправо
		}
	}

	static bool str_ends_with(const char* fullString, const char* ending)
	{
		if (fullString == nullptr || ending == nullptr) {
			return true;
		}
		size_t fullLength = std::strlen(fullString);
		size_t endingLength = std::strlen(ending);

		if (endingLength > fullLength) {
			return false;
		}

		// Сравниваем подстроку в конце fullString с ending
		return std::strcmp(fullString + fullLength - endingLength, ending) == 0;
	}

	template<typename T>
	static T BinaryStringToNumber(const char* binaryString, bool &ok) {
		ok = true;
		size_t len = 0;
		T num = 0;
		while (true) {
			len++;
			num <<= 1; // сдвиг влево
			if (*binaryString == '1') {
				num |= 1; // установка последнего бита в 1
			}
			else if(*binaryString == '0') {
				// после сдвига последний бит уже 1
			}
			else {
				ok = false;
				return 0;
			}
			binaryString++; // Переход к следующему символу
			if(*binaryString == '\0') break;
		}
		if(len > sizeof (T)*8) ok = false;
		return num;
	}
};

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
