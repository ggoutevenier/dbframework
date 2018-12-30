#pragma once
#include <map>
/**
 * Used to resolve a pointer the underlying data given the key to the data
 * The type T is the class of the data.  The class should be derive from the key_type
 * The key_type must be type defined inside the class
 */

namespace dk {
	template<class T>
	class Ref : public T::key_type {
	public:
		Ref() : ptr(0) {}
		Ref(const typename T::key_type &key) : T::key_type(key),ptr(0) {}
		const T* ptr;
		const T *operator->() const { return ptr; }
		bool resolve(const std::map<typename T::key_type, T> &m) {
			auto it = m.find(*this);
			ptr =  (it == m.end() ? nullptr:&it->second);
			return ptr != 0;
		}
	};
}
