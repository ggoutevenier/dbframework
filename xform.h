#pragma once
#include "type.h"
#include <string>
/**
 * xForm is used to transform data from a specific from into another defined by the
 * template parameter.
 *
 * This class is used to transform the data after it is read from the database and
 * before it is put into the C++ class
 */
namespace dk {
	template<size_t N>
	class Type<char[N] > : public Type<char *> {
		public:
		Type():Type<char*>(N){}
		~Type() {}
		Type<char[N]> &setSize(size_t n) {
			Type<char *>::setSize(std::min(n,N));
			return *this;
		}
	};

	template<>
	class Type<char > : public Type<char[1]> {
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, uint32_t pos) override {
			auto v = static_cast<const char*>(data);
			Type<char[1]>::set(writer, v, pos);
		}
		void get(IResultSet &reader, void *data, uint32_t pos) override {
			auto v = static_cast<char*>(data);
			Type<char[1]>::get(reader,v, pos);
		}
	};

	template<int N>
	class Type<dec::decimal<N> > : public Type<Number> {
	public:
		Type():Type<Number>(18,N) {}
		~Type() {}
		void set(IStatement &writer, const  void *data, uint32_t pos) override {
			Number src(static_cast<const dec::decimal<N>*>(data)->getUnbiased(),N);
			Type<Number>::set(writer, &src, pos);
		}
		void get(IResultSet &reader, void *data, uint32_t pos) override {
			Number src(0,N);
			Type<Number>::get(reader, &src, pos);
			if(src.p!=N)
				src.v*=pow(10,N-src.p);
			static_cast<dec::decimal<N>*>(data)->setUnbiased(src.v);
		}
	};
}
