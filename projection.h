#pragma once
namespace dk {
	template<class A, class T, class S>
	inline void projection(T &dst, const S &src) {
		static_cast<A&>(dst) = src;
	}
	template<class A, class B, class T, class S>
	inline void projection(T &dst, const S &src) {
		projection<A>(dst, src);
		projection<B>(dst, src);
	}

	template<class A, class B, class C, class T, class S>
	inline void projection(T &dst, const S &src) {
		projection<A, B>(dst, src);
		projection<C>(dst, src);
	}

	template<class A, class B, class C, class D, class T, class S>
	inline void projection(T &dst, const S &src) {
		projection<A, B, C>(dst, src);
		projection<D>(dst, src);
	}

	template<class A, class B, class C, class D, class E, class T, class S>
	inline void projection(T &dst, const S &src) {
		projection<A, B, C, D>(dst, src);
		projection<E>(dst, src);
	}
}