#ifndef stack_set_H
#define stack_set_H

#include <array>
#include <algorithm>
#include <iostream>
#include <initializer_list>

namespace stdx
{
	template<class T, std::size_t N>
	class stack_set {
	public:
		using iterator = typename std::array<T, N>::iterator;
		using const_iterator = typename std::array<T, N>::const_iterator;

		stack_set() : m_current_size(0) {}
		/// O(n*log(n)), sorting
		stack_set(std::initializer_list<T> init);

		/// O(log(n)) + O(k), k <= n, count moving backward elemens
		template <typename U>
		inline std::pair<iterator, bool> insert(U&& value);

		/// O(log(n))
		std::size_t count(const T& value) const { return contains(value) ? 1 : 0; }
		/// O(log(n))
		bool contains(const T& value) const { return std::binary_search(begin(), end(), value); }

		iterator begin() { return m_data.begin(); }
		iterator end() { return m_data.begin() + m_current_size; }

		const_iterator begin() const { return m_data.begin(); }
		const_iterator end() const { return m_data.begin() + m_current_size; }

		std::size_t size() const { return m_current_size; }
		std::size_t capacity() const { return m_data.size(); }
		bool empty() const { return m_current_size == 0; }

	private:
		std::array<T, N> m_data;
		std::size_t m_current_size;
	};

//--------------------------------------------------------------------------------------------------------------------------

	template<class T, std::size_t N>
	class unordered_stack_set {
	public:
		using iterator = typename std::array<T, N>::iterator;
		using const_iterator = typename std::array<T, N>::const_iterator;

		unordered_stack_set() : m_current_size(0) {}
		/// O(n); doesn't checks initializer_list for duplicates, save data in m_data without changes
		unordered_stack_set(std::initializer_list<T> init);

		/// O(n); checks existance before insert
		template <typename U>
		inline std::pair<iterator, bool> insert(U&& value);

		/// O(n)
		std::size_t count(const T& value) const { return contains(value) ? 1 : 0; }
		/// O(n)
		bool contains(const T& value) const { auto it=std::find(begin(), end(), value); return it!=end(); }

		iterator begin() { return m_data.begin(); }
		iterator end() { return m_data.begin() + m_current_size; }

		const_iterator begin() const { return m_data.begin(); }
		const_iterator end() const { return m_data.begin() + m_current_size; }

		std::size_t size() const { return m_current_size; }
		std::size_t capacity() const { return m_data.size(); }
		bool empty() const { return m_current_size == 0; }

	private:
		std::array<T, N> m_data;
		std::size_t m_current_size;
	};
}


//--------------------------------------------------------------------------------------------------------------------------

template<class T, std::size_t N>
stdx::stack_set<T, N>::stack_set(std::initializer_list<T> init) {
	if(init.size() <= N) m_current_size = init.size();
	else {
		std::cerr << "stack_set too big initializer_list";
		m_current_size = N;
	}

	std::copy_n(init.begin(), m_current_size, m_data.begin());

	std::sort(begin(), end());
	auto last = std::unique(begin(), end());
	m_current_size = std::distance(begin(), last);
}

template<class T, std::size_t N>
template <typename U>
std::pair<typename stdx::stack_set<T, N>::iterator, bool> stdx::stack_set<T, N>::insert(U&& value) {
	auto it = std::lower_bound(begin(), end(), value);

	if (it != end() && *it == value) {
		return { it, false };
	}

	if (m_current_size >= N) {
		return { end(), false };
	}

	std::move_backward(it, end(), end() + 1);
	*it = std::forward<U>(value);
	m_current_size++;

	return { it, true };
}

//--------------------------------------------------------------------------------------------------------------------------

template<class T, std::size_t N>
stdx::unordered_stack_set<T, N>::unordered_stack_set(std::initializer_list<T> init) {
	if(init.size() <= N) m_current_size = init.size();
	else {
		std::cerr << "stack_set_arr too big initializer_list";
		m_current_size = N;
	}

	std::copy_n(init.begin(), m_current_size, m_data.begin());
}

template<class T, std::size_t N>
template <typename U>
std::pair<typename stdx::unordered_stack_set<T, N>::iterator, bool> stdx::unordered_stack_set<T, N>::insert(U&& value) {
	auto it=std::find(begin(), end(), value);
	if (it != end()) {
		return { it, false };
	}

	if (m_current_size >= N) {
		return { end(), false };
	}

	it = begin() + m_current_size;
	*it = std::forward<U>(value);
	m_current_size++;

	return { it, true };
}

//--------------------------------------------------------------------------------------------------------------------------
#endif
