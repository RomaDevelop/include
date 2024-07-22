//---------------------------------------------------------------------------
#ifndef MyCppDifferent_H
#define MyCppDifferent_H
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
class MyCppDifferent
{
public:
    static std::string ToDiapasons(std::vector<int> vect)
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

    template<class T>
    struct reverted_container
    {
	T *container_ptr;

	reverted_container(T &aContainer): container_ptr{&aContainer} {}

	auto begin()
	{
	    return container_ptr->rbegin();
	}

	auto end()
	{
	    return container_ptr->rend();
	}

	auto begin() const
	{
	    return container_ptr->rbegin();
	}

	auto end() const
	{
	    return container_ptr->rend();
	}
    };

    template<class T>
    static auto reverse(T &aContainer)
    {
	return reverted_container(aContainer);
    }

};
#endif
