#pragma once
#include "record.h"
namespace dk {
	template<class TT>
	class metadata : protected Record {
		using T = TT;
		T t;
		metadata();
		template<class S>
		Field<S> &add(S &s, const std::string &name) {
			return Record::add(t, s, name);
		}
		Field<const logic::Function<T> *> &add(const logic::Function<T> *&f) {
			return Record::add(t, f);
		}

	public:
		static const Record &record() { //not thread safe
			static metadata<T> rtn;
			return rtn;
		}
	};
}