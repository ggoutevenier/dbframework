#pragma once
namespace dk {
	template<class T>
	class interval {
	public:
		T from, to;
		bool between(const T &a) const {
			return !(a < from || to < a);
		}
		interval() {}
		interval(const interval<T> &v) :
			from(v.from), to(v.to) {}
		interval(const T &from, const T &to) :
			from(from), to(to) {}
	};
}
