#pragma once
#include <map>

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
