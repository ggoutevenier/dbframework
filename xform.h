#pragma once
#include "timestamp.h"
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
	template<>
	class Type<std::string > : public Type<char*> {
		std::vector<char> buff;
		void toBuff_(std::string str) override {
			// if string large then buff only copy size bytes of string
			// make sure to set null term
			*std::copy_n(
				str.begin(),
				std::min(str.length(),getSize()),
				buff.begin()
			)=0;
		}
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
			toBuff_(*static_cast<const std::string*>(data));
			Type<char*>::writer.set(buff.data(),column);
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			Type<char*>::reader.get(buff.data(),column);
			*static_cast<std::string*>(data) = buff.data();
		}
		Type<std::string> setSize(size_t size) {
			buff.resize(size+1);
			buff.at(size)=0; // set null term
			return *this;
		}
		size_t getSize() const override {
			if(buff.size()==0) setSize(0);// if buff not size make room for null term
			return buff.size()-1; // size one less due to null term
		}
	};

	template<size_t N>
	class Type<char[N] > : public Type<char *> {
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
			auto v = static_cast<const char *>(data);
			Type<char *>::set(writer, &v, column);
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			auto v = static_cast<char *>(data);
			Type<char *>::get(reader, &v,column);
		}
		size_t getSize() const override {
			return N;
		}
	};

	template<>
	class Type<char > : public Type<char[1]> {
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
			auto v = static_cast<const char*>(data);
			Type<char[1]>::set(writer, v, column);
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			auto v = static_cast<char*>(data);
			Type<char[1]>::get(reader,v,column);
		}
	};

	template<int N>
	class Type<dec::decimal<N> > : public Type<Number> {
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &field) const override {
			auto &t = *static_cast<const dec::decimal<N>*>(data);
			Number src(t.getUnbiased(),N);
			Type<Number>::set(writer, &src, field);
		}
		void get(IResultSet &reader, void *data, IColumn &field) const override {
			Number src(0,N);
			auto &t = *static_cast<dec::decimal<N>*>(data);
			Type<Number>::get(reader, &src, field);
			if(src.p!=N)
				src.v*=pow(10,N-src.p);
			t.setUnbiased(src.v);
		}
	};

	template<>
	class Type<bool> : public TypeBase<bool> {
		const std::array<char, 2> TF;
		
	public:
		char buff;
		Type(std::array<char, 2> TF = { '1','0' }) : TF(TF) {
		}
		~Type() {}
		void set(bool value) {
			buff = TF.at(value == false);
		}
		void get(bool &data, IColumn &field) const{
			data = (TF[0]==buff);
		}
		size_t getSize() const override {
			return 1;
		}
	};

	template<>
	class Type<timestamp> : public Type<tm> {
		typedef timestamp T;
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &field) const override {
			auto &t = *static_cast<const timestamp*>(data);
			tm src;
			t.as_tm(src);
			Type<tm>::set(writer, &src, field);
		}
		void get(IResultSet &reader, void *data, IColumn &field) const override {
			tm src;
			auto &t = *static_cast<timestamp*>(data);
			Type<tm>::get(reader, &src, field);
			t = src;
		}
	};
}
