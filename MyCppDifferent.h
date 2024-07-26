//---------------------------------------------------------------------------
#ifndef MyCppDifferent_H
#define MyCppDifferent_H
//---------------------------------------------------------------------------
#include <direct.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <algorithm>
//---------------------------------------------------------------------------
class MyCppDifferent
{
public:
    inline static std::string ToDiapasons(std::vector<int> vect);
    inline static std::string GetPathToExe();	// если программа запускается из среды Qt creator, то выдаёт путь не до конца
						// пропускает последнюю папку debug или release
						// если программа запущена уже как выпущенный exe-файл, то всё норм

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
    static auto reverse(T &aContainer) { return reverted_container(aContainer); }
};
//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------------------
#endif
