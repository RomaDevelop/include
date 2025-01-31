#ifndef unhashed_map_H
#define unhashed_map_H

#include <vector>

namespace stdx
{
    template<class key_t, class value_t>
    class unhashed_map
    {
    public:
	explicit unhashed_map()
	{
	    static_assert(std::is_integral_v<key_t>, "ket_t must be an integral type");
	}

	std::vector<value_t> values;
	std::vector<bool> exists;

	value_t& operator[] (const key_t& key)
	{
	    if(key >= values.size())
	    {
		values.resize(key+1);
		exists.resize(key+1, false);
	    }

	    exists[key] = true;
	    return values[key];
	}
	bool insert(key_t key, value_t value)
	{
	    if(key >= values.size())
	    {
		values.resize(key+1);
		exists.resize(key+1, false);

		exists[key] = true;
		values[key] = std::move(value);
		return false;
	    }

	    values[key] = std::move(value);
	    if(exists[key]) return true;
	    else { exists[key] = true; return false; }
	}
	/*
	итераторы
	перехода с одного на другой нужно пролистывать все пустые
	или в добавок создать ещё и линейный список?
	нужно почитать как работает hash map
	*/
    };
}

/*

#include <iostream>
#include <map>
using namespace std;

#include "unhashed_map.h"
using namespace stdx;

int main()
{
	unhashed_map<int, string> m;

	//m.insert({5,"dsdv"});
	m[5] = "555";

	cout << "5: " << m[5] << "\n6: " << m[6];

	cout << "\ninsert 2 res: " << m.insert(2,"222") << "\n2: " << m[2];
	cout << "\ninsert 2 res: " << m.insert(2,"2222") << "\n2: " << m[2];

	return 0;
}

 */


#endif
