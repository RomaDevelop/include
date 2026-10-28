#ifndef stdx_h
#define stdx_h

#include <cstring>
#include <array>
#include <vector>
#include <algorithm>

#include "CodeMarkers.h"

namespace stdx
{
	// Объявления вспомогательных типов и функций
	namespace {
		template <typename C, typename = std::void_t<>>
		struct has_find : std::false_type {};

		template <typename T>
		struct has_find<T, std::void_t<decltype(std::declval<T>().find(std::declval<typename T::key_type>()))>>
			: std::true_type {};

		template<class L, class R>
		bool smart_compare(const L& lhs, const R& rhs);
	}

	template<class Range, class Value>
	bool contains(const Range& range, const Value& value)
	{
		// Если у контейнера есть метод find, используем его
		if constexpr (has_find<Range>::value)
		{
			return range.find(value) != range.end();
		}
		else // Иначе - перебор
		{
			for(auto &item:range) if(smart_compare(item, value)) return true;
			return false;
		}

		CodeMarkers::to_do_with_cpp20("проверку has_find<T, M>::value можно сделать еще проще, через requires");
	}

	/// idiom remove - erase
	template<class Container, class Value>
	void remove_erase(Container& container, const Value& value)
	{
		container.erase(std::remove(container.begin(), container.end(), value), container.end());
	}

	/// idiom remove_if - erase, in remove_if Predicate checks each item from remove_from with stdx::contains
	template<class Container, class Range>
	void remove_erase_range(Container& container, const Range& range)
	{
		auto remove_res = std::remove_if(container.begin(), container.end(), [&](const auto& item) {
			return stdx::contains(range, item);
		});

		container.erase(remove_res, container.end());
	}

	enum class insert_result { new_node_inserted, existing_node_was_rewrited, not_enough_space };

	template<class key_t, std::size_t size>
	class flat_stack_set
	{
	public:

		template<typename... Args>
		static flat_stack_set from_args(Args&&... args)
		{
			// Проверка времени компиляции, чтобы не передали аргументов больше, чем вмещает контейнер
			static_assert(sizeof...(Args) <= size, "Too many arguments for flat_stack_set capacity!");

			flat_stack_set set;

			// C++17 Fold Expression (сворачивание параметров шаблона)
			// Последовательно вызывает insert для каждого переданного аргумента
			(set.insert(std::forward<Args>(args)), ...);

			return set;
		}

		bool contains(const key_t& key) const
		{
			return find_key(key) != nullptr;
		}
		key_t* find_key(const key_t& key)
		{
			for(std::size_t i=0; i<m_current_size; i++) if(smart_compare(m_data[i], key)) return &m_data[i];
			return nullptr;
		}
		const key_t* find_key(const key_t& key) const
		{
			for(std::size_t i=0; i<m_current_size; i++) if(smart_compare(m_data[i], key)) return &m_data[i];
			return nullptr;
		}

		insert_result insert(key_t key)
		{
			auto k = find_key(key);
			if(k)
			{
				*k = std::move(key);
				return insert_result::existing_node_was_rewrited;
			}

			if(m_current_size >= size)
			{
				return insert_result::not_enough_space;
			}

			auto &newNode = m_data[m_current_size++];
			newNode = std::move(key);
			return insert_result::new_node_inserted;
		}

	private:
		std::array<key_t, size> m_data;
		std::size_t m_current_size = 0;
	};

	template<class key_t, class value_t, std::size_t size>
	class flat_stack_map
	{
	public:

		template<typename... Args>
		static flat_stack_map from_args(Args&&... args)
		{
			// Проверка времени компиляции, чтобы не передали аргументов больше, чем вмещает контейнер
			static_assert(sizeof...(Args) <= size, "Too many arguments for flat_stack_set capacity!");

			flat_stack_map map;

			// C++17 Fold Expression (сворачивание параметров шаблона)
			// Последовательно вызывает insert для каждого переданного аргумента
			(map.insert(std::forward<Args>(args)), ...);

			return map;
		}

		//value_t& operator[] (const key_t& key) - корректно невозможно, ибо что возвращать, если заполнена

		value_t* find_value(const key_t& key)
		{
			for(std::size_t i=0; i<m_current_size; i++) if(smart_compare(m_data[i].first, key)) return &m_data[i].second;
			return nullptr;
		}
		const value_t* find_value(const key_t& key) const
		{
			for(std::size_t i=0; i<m_current_size; i++) if(smart_compare(m_data[i].first, key)) return &m_data[i].second;
			return nullptr;
		}

		insert_result insert(key_t key, value_t value)
		{
			auto v = find_value(key);
			if(v)
			{
				*v = std::move(value);
				return insert_result::existing_node_was_rewrited;
			}

			if(m_current_size >= size)
			{
				return insert_result::not_enough_space;
			}

			auto &newNode = m_data[m_current_size++];
			newNode.first = std::move(key);
			newNode.second = std::move(value);
			return insert_result::new_node_inserted;
		}
		insert_result insert(std::pair<key_t, value_t> node)
		{
			return insert(node.first, node.second);
		}

	private:
		std::array<std::pair<key_t, value_t>, size> m_data;
		std::size_t m_current_size = 0;
	};


	// класс абсолютно абсурдный, создан для эксперимента, нужно будет удалить
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
		insert_result insert(key_t key, value_t value)
		{
			if(key >= values.size())
			{
				values.resize(key+1);
				exists.resize(key+1, false);

				exists[key] = true;
				values[key] = std::move(value);
				return insert_result::new_node_inserted;
			}

			values[key] = std::move(value);
			if(exists[key]) return insert_result::existing_node_was_rewrited;
			else { exists[key] = true; return insert_result::new_node_inserted; }
		}
		/*
	итераторы
	перехода с одного на другой нужно пролистывать все пустые
	или в добавок создать ещё и линейный список?
	нужно почитать как работает hash map
	*/
	};

	namespace {
		template<class L, class R>
		bool smart_compare(const L& lhs, const R& rhs)
		{
			// std::decay_t превращает массивы char[N] в указатели const char*
			using DecayedL = std::decay_t<L>;
			using DecayedR = std::decay_t<R>;

			if constexpr (std::is_convertible_v<DecayedL, const char*> && std::is_convertible_v<DecayedR, const char*>)
			{
				if (lhs == nullptr || rhs == nullptr)
				{
					return lhs == rhs;
				}

				return std::strcmp(lhs, rhs) == 0;
			}
			else
			{
				return lhs == rhs;
			}
		}
	}
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
