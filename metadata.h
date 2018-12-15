#pragma once
#include "record.h"
namespace dk {
	template<class T>
	class metadata : public Record , public T {
		template<class S>
		Field<S> &add(S &s, const std::string &name) {
			return Record::add(static_cast<const T&>(*this), s, name);
		}
		Field<const logic::Function<T> *> &add(const logic::Function<T> *&f) {
			return Record::add(static_cast<const T&>(*this), f);
		}

	public:
		metadata();
	};
}
