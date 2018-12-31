#pragma once
#include "interface.h"
#include "ref.h"
#include <algorithm>
#include <cstring>
//#include "sequence.h"

namespace dk {
	template<class T>
	class TypeBase : public IType {
	public:
		virtual ~TypeBase() {}
		void set(
			IStatement &writer,
			const void *data,
			uint32_t pos
		) override {
			writer.set(*static_cast<const T *>(data), *this, pos);
		}
		void get(
			IResultSet &reader,
			void *data,
			uint32_t pos
		) override {
			reader.get(*static_cast<T *>(data), *this, pos);
		}
		const std::string name(
			const IMetaData &mdata
		) const override {
			return mdata.type<T>(*this);
		}
		bool resolve(Store &, void *data) const override {
			return true;
		}
		bool selectable() const override {
			return true;
		}
		size_t getSize() const override {return 0;}
		IType &getType() {return *this;}
		std::string asString(const char *v) {
			size_t n = std::min(strlen(v),getSize());
			std::string str(v,&v[n]);
			return str;
		}
		void fromString(const std::string str,char *v) {
			size_t n=std::min(str.length(),getSize());
			std::copy_n(str.begin(),n,v);
			v[n]=0;
		}
	};

	template<class T>
	class Type : public TypeBase<T> {};

	template<>
	class Type<Number> : public TypeBase<Number> {
		uint32_t m; /// maximum number of digits (the precision)
		uint32_t d; /// number of digits to the right of the decimal point (the scale)
	public:
		Type(int m=10,int d=0):m(m),d(d) {}
		~Type() {}

		uint32_t getPrecision() const {
			return m;
		}
		uint32_t getScale() const {
			return d;
		}
		Type<Number> & setPrecision(int m) {
			this->m=m;
			return *this;
		}
		Type<Number> & setScale(int d) {
			this->d=d;
			return *this;
		}
	};

	template<>
	class Type<char *> : public TypeBase<char *> {
		size_t size;
	public:
		Type(size_t size=0):size(size) {}
		Type<char *> setSize(size_t size) {
			this->size=size;
			return *this;
		}
		size_t getSize() const override {
			return size;
		}
		void set(
			IStatement &writer,
			const void *data,
			uint32_t pos
		) override {
			writer.set(static_cast<const char *>(data), *this, pos);
		}
		void get(
			IResultSet &reader,
			void *data,
			uint32_t pos
		) override {
			reader.get(static_cast<char *>(data), *this, pos);
		}
	};

	template<>
	class Type<std::string > : public TypeBase<std::string> {
		std::vector<char> buff;
	public:
		Type() {setSize(64);}
		~Type() {}
		char *getBuff() {return buff.data();}
		const char *getBuff() const {return buff.data();}
		Type<std::string> setSize(size_t size) {
			buff.resize(size+1);
			return *this;
		}
		size_t getSize() const override {
			return buff.size()-1;
		}
		void fromString(std::string value) {
			TypeBase<std::string>::fromString(value,getBuff());
		}
		std::string asString() {
			return TypeBase<std::string>::asString(getBuff());
		}
	};


	template<>
	class Type<tm> : public TypeBase<tm> {
		std::string format;
	public:
		std::array<char,20> buff;
		Type():format("%4d-%02d-%02d %02d:%02d:%02d") {}
		~Type() {}
		void setDateFormat(std::string format) {
			this->format = format;
		}
		size_t getSize() const override {return buff.size();}
		
		void setV(tm &date) {
			date.tm_hour = date.tm_min = date.tm_sec = 0;
			sscanf(	buff.data(),
				format.c_str(),
				&date.tm_year,
				&date.tm_mon,
				&date.tm_mday,
				&date.tm_hour,
				&date.tm_min,
				&date.tm_sec);
			date.tm_isdst = 0;
			date.tm_gmtoff = 0;
			date.tm_year -= 1900;
			date.tm_mon -= 1;
			mktime(&date);
		}
		
		const char *asString(const tm &date) {
			sprintf(
				buff.data(),
				format.c_str(),
				date.tm_year + 1900,
				date.tm_mon + 1,
				date.tm_mday,
				date.tm_hour,
				date.tm_min,
				date.tm_sec
			);
			return buff.data();
		}
	};

	template<>
	class Type<bool> : public TypeBase<bool> {
		const std::array<char, 2> TF;

	public:
		char buff[2];
		Type(std::array<char, 2> TF = { '1','0' }) : TF(TF) {buff[1]=0;}
		~Type() {}
		void getV(bool &value) const {
			value = (TF[0]==buff[0]);
		}
		void setV(const bool &value) {
			buff[0] = TF.at(value == false);
		}
		size_t getSize() const override {
			return 1;
		}
	};


/*	template<>
	class Type < Sequence > : public TypeBase<Sequence::type> {
	public:
		virtual ~Type() {}
		void get(
			IResultSet &reader,
			void *data,
			IColumn &field
		) override { //show never call Field<Sequence>::get should handle
			assert(false);
		}
		bool selectable() const override {
			return false;
		}
	};*/

	template<class T>
	class Type <Ref<T>> : public IType { // can it inherit from type<T>
		typedef typename T::key_type K;
		typedef Ref<T> R;
	protected:
	public:
		void set(IStatement &writer, const  void *data, uint32_t pos) override {
			//noop
		}
		void get(IResultSet &reader, void *data,  uint32_t pos) override {
			//noop
		}
		const std::string name(const IMetaData &mdata) const override {
			return std::string();
		}

		bool resolve(Store &store, void *data) const override {
			auto &r = *static_cast<R*>(data);
			r.resolve(getMap<T>(store));
			return r.ptr != 0;
		}
		
		size_t getSize() const override {return 0;}

		bool selectable() const override { return false; }

		IType &getType() {
			return *this;
		}
	};
}
