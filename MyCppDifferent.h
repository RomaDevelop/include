//------------------------------------------------------------------------------------------------------------------------------------------
#ifndef MyCppDifferent_H
#define MyCppDifferent_H
//------------------------------------------------------------------------------------------------------------------------------------------
#include <direct.h>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
#include <iomanip>

#include "CodeMarkers.h"
//------------------------------------------------------------------------------------------------------------------------------------------
class MyCppDifferent
{
public:
	inline static std::string ToDiapasons(std::vector<int> vect);

	inline static std::string GetPathToExe();	// если программа запускается из среды Qt creator, то выдаёт путь не до конца
	// пропускает последнюю папку debug или release
	// если программа запущена уже как выпущенный exe-файл, то всё норм

	inline static std::string current_date_time(const char *format = "%Y.%m.%d %H-%M-%S");

	inline static void sleep_ms(uint milliseconds) { std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds)); }

	template<class T>
	struct reverted_container
	{
		T *container_ptr;

		reverted_container(T &aContainer): container_ptr{&aContainer} {}

		auto begin() { return container_ptr->rbegin(); }
		auto begin() const { return container_ptr->rbegin(); }
		auto end() { return container_ptr->rend(); }
		auto end() const { return container_ptr->rend(); }
	};

	template<class T>
	inline static auto reverse(T &aContainer) { return reverted_container(aContainer); }

	inline static std::array<unsigned char,4> to_bytes(float value); // return[0] - нулевой байт value
	inline static float to_float(std::array<unsigned char,4> bytes); // нулевой байт в return = bytes[0]
	inline static bool test_float_to_bytes_conversion();

	template <class T>
	class any_guard
	{
	public:
		any_guard(T &varible, T startValue, T endValue):
			m_variable {varible},
			m_end_value {std::move(endValue)}
		{
			m_variable = std::move(startValue);
		}
		~any_guard() { m_variable = std::move(m_end_value); }
	private:
		T &m_variable;
		T m_end_value;
	};
};


//------------------------------------------------------------------------------------------------------------------------------------------
std::string MyCppDifferent::ToDiapasons(std::vector<int> vect)
{
	std::string resOff;
	if(vect.size())
	{
		resOff = std::to_string(vect[0]);
		int interval = 1;
		for(int i=1; i<(int)vect.size(); i++)
		{
			if(vect[i] - vect[i-1] == 1)
			{
				if(resOff[resOff.size()-1] != '-') resOff += "-";
				interval++;

			}
			else
			{
				if(resOff[resOff.size()-1] == '-')
				{
					if(interval == 2)
					{
						resOff[resOff.size()-1] = ',';
						resOff += " ";
					}
					resOff += std::to_string(vect[i-1]) + ", " + std::to_string(vect[i]);
				}
				else resOff += ", " + std::to_string(vect[i]);
				interval = 1;
			}
		}

		if(resOff[resOff.size()-1] == '-') resOff += std::to_string(vect.back());
	}
	return resOff;
}

std::string MyCppDifferent::GetPathToExe()
{
	char buffer[_MAX_PATH];
	if(getcwd(buffer, _MAX_PATH) != NULL)
	{
		std::string path(buffer);
		std::replace(path.begin(), path.end(), '\\', '/');
		return path;
	}
	else
	{
		std::cerr << "GetPathToExe ERROR getcwd returned NULL" << std::endl;
		return "ERROR";
	}
}

std::string MyCppDifferent::current_date_time(const char * format)
{
	if(0) CodeMarkers::to_do("нужно пределать эту функцию, убрать stringstream и put_time, сделать быстрее");

	time_t now = time(nullptr); // Получаем текущее время как количество секунд с начала 1970 года
	struct tm timeinfo;
	localtime_s(&timeinfo, &now); // Преобразуем время в структуру tm с локальным временем
	std::stringstream ss;
	ss << std::put_time(&timeinfo, format);
	std::string time_str = ss.str();
	return time_str;
}

std::array<unsigned char, 4> MyCppDifferent::to_bytes(float value)
{
	const int size = 4;
	const int sizeOf = sizeof(value);
	if(sizeOf != size) { std::cerr << "ToBytes error, sizeof(float) = " + std::to_string(sizeOf); return {}; }

	std::array<unsigned char,size> ret;
	unsigned char* ptr = reinterpret_cast<unsigned char*>(&value);
	for(int i=0; i<size; i++)
	{
		ret[i] = ptr[i];
	}

	return ret;
}

float MyCppDifferent::to_float(std::array<unsigned char, 4> bytes)
{
	const int size = bytes.size();
	const int sizeOf = sizeof(float);
	if(sizeOf != size) { std::cerr << "ToBytes error, sizeof(float) = " + std::to_string(sizeOf); return {}; }

	float ret;
	unsigned char* ptr = reinterpret_cast<unsigned char*>(&ret);
	for(int i=0; i<size; i++)
	{
		ptr[i] = bytes[i];
	}

	return ret;
}

bool MyCppDifferent::test_float_to_bytes_conversion()
{
	bool res = true;
	srand(time(nullptr));

	float from;
	std::array<unsigned char,4> bytes;
	float to;
	std::cerr.precision(15);
	for(int i=0; i<10; i++)
	{
		from = (float) rand() / rand();
		bytes = to_bytes(from);
		to = to_float(bytes);
		if(fabs(from - to) > fabs(from) * 0.00001)
		{ std::cerr << "bad conversion " << from << " -> " << to << " fabs=" << fabs(from - to) << std::endl; res = false; }

		from = (float) rand() / - rand();
		bytes = to_bytes(from);
		to = to_float(bytes);
		if(fabs(from - to) > fabs(from) * 0.00001)
		{ std::cerr << "bad conversion " << from << " -> " << to << " fabs=" << fabs(from - to) << std::endl; res = false; }

		from = (float) rand() / rand() * 999999.0f;
		bytes = to_bytes(from);
		to = to_float(bytes);
		if(fabs(from - to) > fabs(from) * 0.00001)
		{ std::cerr << "bad conversion " << from << " -> " << to << " fabs=" << fabs(from - to) << std::endl; res = false; }

		from = (float) rand() / - rand() * 999999.0f;
		bytes = to_bytes(from);
		to = to_float(bytes);
		if(fabs(from - to) > fabs(from) * 0.000001)
		{ std::cerr << "bad conversion " << from << " -> " << to << " fabs=" << fabs(from - to) << std::endl; res = false; }
	}

	return res;
}

//------------------------------------------------------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------------------------------------------------------
