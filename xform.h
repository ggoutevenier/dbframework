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
	template<class T> class XForm : public Type<T>{};

	template<>
	class XForm<std::string > : public Type<char*> {
	public:
		XForm() {}
		~XForm() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
//			auto &v = *static_cast<const std::string*>(data);
			column.toBuff(*static_cast<const std::string*>(data));
			writer.set(column);
/*			auto &buff = column.getBuff();
			std::copy(v.begin(),v.end(),buff.begin());
			buff.at(std::min(v.length(),buff.size()-1))=0;
			auto p = column.getBuff().data();
			Type<char*>::set(writer, &p, column);*/
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			auto &v = *static_cast<std::string*>(data);
//			auto buff = column.getBuff().data();
//			Type<char*>::get(reader,&buff,column);
//			v = std::string(buff,strlen(buff));
			reader.get(column);
			v = column.getBuff().data();
		}
	};

	template<>
	class XForm<char > : public Type<char *> {
	public:
		XForm() {}
		~XForm() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
			auto v = static_cast<const char*>(data);
			Type<char *>::set(writer, v, column);
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			auto v = static_cast<char*>(data);
			Type<char *>::get(reader,v,column);
		}
	};

	template<size_t N>
	class XForm<char[N] > : public Type<char *> {
	public:
		XForm() {}
		~XForm() {}
		void set(IStatement &writer, const  void *data, IColumn &column) const override {
			auto v = static_cast<const char *>(data);
			Type<char *>::set(writer, &v, column);
		}
		void get(IResultSet &reader, void *data, IColumn &column) const override {
			auto v = static_cast<char *>(data);
			Type<char *>::get(reader, &v,column);
		}
	};

	template<int N>
	class XForm<dec::decimal<N> > : public Type<Number> {
	public:
		XForm() {}
		~XForm() {}
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
	class XForm<bool> : public XForm<char > {
		const std::array<char, 2> TF; 
	protected:
	public:
		XForm(std::array<char, 2> TF = { '1','0' }) : TF(TF) {}
		~XForm() {}
		void set(IStatement &writer, const  void *data, IColumn &field) const override {
			char src;
			auto &t = *static_cast<const bool*>(data);
			src = TF.at(t == false);
			XForm<char>::set(writer, &src, field);
		}
		void get(IResultSet &reader, void *data, IColumn &field) const override {
			char src;
			auto &t = *static_cast<bool*>(data);
			XForm<char>::get(reader, &src, field);
			if (!(src == TF.at(0) || src == TF.at(1) || src == 0)) {
				//Conversion error
			}
			t = (src == TF.at(0));
		}
	};

	template<>
	class XForm<timestamp> : public Type<tm> {
		typedef timestamp T;
	public:
		XForm() {}
		~XForm() {}
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
