#pragma once
#include <map>
namespace dk {
	template<class K, class T>
	const T *resolve_(const K &key, const std::map<K, const T*> &m) {
		const std::map<K, const T*>::const_iterator it = m.find(key);
		if (it == m.end()) {
			return 0;
		}
		return it->second;
	}

	template<class T>
	class Ref : public T::key_type {
	public:
		Ref() {}
		Ref(const typename T::key_type &key) : T::key_type(key) {}
		const T* ptr;
		const T *operator->() const { return ptr; }
		bool resolve(const std::map<typename T::key_type, const T*> &m) {
			return (ptr = resolve_(static_cast<typename T::key_type&>(*this), m)) != 0;
		}
	};
}