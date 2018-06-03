#pragma once

namespace dk {
	class Sequence {
	public:
		using type = size_t;
		Sequence() :seq(1), inc(1){}
		Sequence(const Sequence &a) :seq(a.seq),inc(a.inc) {}
		type getSequence() const { return seq; }
		Sequence &operator=(const Sequence &a) {
			seq = a.seq;
			inc = a.inc;
			return *this;
		}
		const Sequence operator++(int ) {
			seq+=inc;
			return *this;
		}
		void startWith(const type &v) {
			seq = v;
		}
		void increaseBy(const type &v) {
			inc = v;
		}
	private:
		type seq,inc;
	};
}