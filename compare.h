#pragma once
#include "timestamp.h"

namespace dk {
	template<class T>
	inline bool less_(const T&a, const T&b) {
		return a < b;
	}
	template<class A, class B, class T>
	inline bool less_(const T&a, const T&b) {
		return
			less_<A>(a, b) ? true :
			less_<A>(b, a) ? false :
			less_<B>(a, b);
	}
	template<class A, class B, class C, class T>
	inline bool less_(const T&a, const T&b) {
		return
			less_<A, B>(a, b) ? true :
			less_<A, B>(b, a) ? false :
			less_<C>(a, b);
	}
	template<class A, class B, class C, class D, class T>
	inline bool less_(const T&a, const T&b) {
		return
			less_<A, B, C>(a, b) ? true :
			less_<A, B, C>(b, a) ? false :
			less_<D>(a, b);
	}
	template<class A, class B, class C, class D, class E, class T>
	inline bool less_(const T&a, const T&b) {
		return
			less_<A, B, C, D>(a, b) ? true :
			less_<A, B, C, D>(b, a) ? false :
			less_<E>(a, b);
	}
	inline bool less_(const timestamp a, const timestamp b) {
		return a < b;
	}
	template<class T, typename A, typename B>
	inline bool is_equal(const A &a, const B &b) {
		return !(((T&)a) < ((T&)b) || ((T&)b) < ((T&)a));
	}
}