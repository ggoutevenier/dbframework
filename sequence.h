
#pragma once

namespace dk {
	class Sequence {
	public:
		using type = unsigned int;
		Sequence() :seq(1){}
		Sequence(const type &a) :seq(a) {}
		Sequence(const Sequence &a) :seq(a.seq) {}
		type getSequence() const { return seq; }
		Sequence &operator=(const Sequence &a) {
			seq = a.seq;
			return *this;
		}
		Sequence &operator+=(const Sequence &a) {
			seq += a.seq;
			return *this;
		}
	private:
		type seq;
	};
}
