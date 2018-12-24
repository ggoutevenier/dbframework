#pragma once
#include <cstdint>
namespace dk {
	template<class T>
	inline constexpr T pow(const T base, unsigned const exponent)
	{
		return (exponent == 0) ? 1 : (base * pow(base, exponent-1));
	}

	class Number {
	public:
		int64_t v;
		int p;
		Number():v(0),p(0) {}
		Number(int64_t v,int p) :
			v(v),p(p){}
		double asDouble() const {
			return v/pow(10,p);
		}
		void fromDouble(double d) {
			v=d*pow(10,p);
		}
	};
}
