#pragma once
#include "interface.h"
#include "ref.h"

namespace dk {
	template<class T>
	class TypeBase : public IType {
	public:
		virtual ~Type() {}
		void set(
			IStatement &writer,
			const  void *data,
			IColumn &field
		) const override {
			writer.set(*static_cast<const T *>(data), field);
		}
		void get(
			IResultSet &reader,
			void *data,
			IColumn &field
		) const override {
			reader.get(*static_cast<T *>(data), field);
		}
		const std::string name(
			const IMetaData &mdata,
			const IColumn &field
		) const override {
			return mdata.type<T>(field);
		}
		bool resolve(Store &, void *data) const override {
			return true;
		}
		bool selectable() const override {
			return true;
		}
		size_t getSize() const override {return 0;}
	};

	template<class T>
	class Type : public TypeBase<T> {};

	template<>
	class Type<Number> : TypeBase<T> {
		unsigned int m; /// maximum number of digits (the precision)
		unsigned int d; /// number of digits to the right of the decimal point (the scale)
	public:
		Type() {}
		~Type() {}

		int getPrecision() const {
			return m;
		}
		int getScale() const {
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
	}

	template<>
	class Type<tm> : public TypeBase<tm> {
		std::array<char,30> format;
	public:
		std::array<char,20> buff;
		Type():format("%4d-%02d-%02d %02d:%02d:%02d") {}
		~Type() {}
		void setDateFormat(std::string format) {
			this->format = format;
		}
		std::string getDateFormat() const override {
			return format;
		}
		size_t getSize() const override {return buff.size();}
		
		void set(tm &date) {
			tm &date = *static_cast<tm*>(data);
			date.tm_hour = date.tm_min = date.tm_sec = 0;
			sscanf(	buff,
				format,
				&date.tm_year,
				&date.tm_mon,
				&date.tm_mday,
				&date.tm_hour,
				&date.tm_min,
				&date.tm_sec);

			date.tm_year -= 1900;
			date.tm_mon -= 1;
			mktime(&date);
		}
		
		char * asAString(const tm &date) {
			sprintf(
				buff,
				format,
				date.tm_year + 1900,
				date.tm_mon + 1,
				date.tm_mday,
				date.tm_hour,
				date.tm_min,
				date.tm_sec
			);
			return buff;
		}
	}

	};


	template<>
	class Type < Sequence > : public Type<Sequence::type> {
	public:
		virtual ~Type() {}
		void get(
			IResultSet &reader,
			void *data,
			IColumn &field
		) const override { //show never call Field<Sequence>::get should handle
			assert(false);
		}
		bool selectable() const override {
			return false;
		}
	};

	template<class T>
	class Type <Ref<T>> : public IType { // can it inherit from type<T>
		typedef typename T::key_type K;
		typedef Ref<T> R;
	protected:
	public:
		void set(IStatement &writer, const  void *data, IColumn &field) const override {
			//noop
		}
		void get(IResultSet &reader, void *data, IColumn &field) const override {
			//noop
		}
		const std::string type(const IMetaData &mdata, const IColumn &field) const {
			return std::string();
		}

		bool resolve(Store &store, void *data) const override {
			auto &r = *static_cast<R*>(data);
			r.resolve(getMap<T>(store));
			return r.ptr != 0;
		}
		
		bool selectable() const override { return false; }
	};
}
