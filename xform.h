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
//	template<class T> class XForm : public Type<T>{};

	template<>
	class Type<std::string > : public Type<char*> {
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
			column.toBuff(*static_cast<const std::string*>(data));
			writer.set(column);
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			auto &v = *static_cast<std::string*>(data);
			reader.get(column);
			v = column.getBuff().data();
		}
	};

	template<size_t N>
	class Type<char[N] > : public Type<char *> {
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
			auto v = static_cast<const char *>(data);
			Type<XTYPE>::set(writer, &v, column);
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			auto v = static_cast<char *>(data);
			Type<XTYPE>::get(reader, &v,column);
		}
	};

	template<>
	class Type<char > : public Type<char[1]> {
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
			auto v = static_cast<const char*>(data);
			Type<XTYPE>::set(writer, v, column);
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			auto v = static_cast<char*>(data);
			Type<XTYPE>::get(reader,v,column);
		}
	};

	template<int N>
	class Type<dec::decimal<N> > : public Type<Number> {
	public:
		Type() {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &field) const override {
			auto &t = *static_cast<const dec::decimal<N>*>(data);
			XTYPE src(t.getUnbiased(),N);
			Type<XTYPE>::set(writer, &src, field);
		}
		void get(IResultSet &reader, void *data, IColumn &field) const override {
			XTYPE src(0,N);
			auto &t = *static_cast<dec::decimal<N>*>(data);
			Type<XTYPE>::get(reader, &src, field);
			if(src.p!=N)
				src.v*=pow(10,N-src.p);
			t.setUnbiased(src.v);
		}
	};

	template<>
	class Type<bool> : public Type<char > {
		const std::array<char, 2> TF; 
	protected:
	public:
		Type(std::array<char, 2> TF = { '1','0' }) : TF(TF) {}
		~Type() {}
		void set(IStatement &writer, const  void *data, IColumn &field) const override {
			XTYPE src;
			auto &t = *static_cast<const bool*>(data);
			*src = TF.at(t == false);
			Type<XTYPE>::set(writer, src, field);
		}
		void get(IResultSet &reader, void *data, IColumn &field) const override {
			XTYPE src;
			auto &t = *static_cast<bool*>(data);
			Type<XTYPE>::get(reader, src, field);
			if (!(*src == TF.at(0) || *src == TF.at(1) || *src == 0)) {
				//Conversion error
			}
			t = (*src == TF.at(0));
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
			XTYPE src;
			t.as_tm(src);
			Type<XTYPE>::set(writer, &src, field);
		}
		void get(IResultSet &reader, void *data, IColumn &field) const override {
			XTYPE src;
			auto &t = *static_cast<timestamp*>(data);
			Type<XTYPE>::get(reader, &src, field);
			t = src;
		}
	};
}
