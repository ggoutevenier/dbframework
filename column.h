#pragma once
//#include "interface.h"
#include "function.h"
#include "xform.h"
#include <algorithm>

namespace dk {
	class ColumnBase : public IColumn {
	protected:
		size_t offset;
		std::string name;
		int position;
		std::list<std::shared_ptr<IColumn> > others;
		void *adjust(void *data) const {
			return static_cast<char*>(data) + offset;
		}
		const void *adjust(const void *data) const {
			return const_cast<Column<T>*>(this)->adjust(data);
		}
	public:
		ColumnBsae(size_t offset, const std::string &name, int position) :
			offset(offset), name(name), position(position) {}
		virtual ~ColumnBase() {}

		std::string getColumnName() const override { return name; }

		int getColumnPosition() const override{ return position; }

		void other(std::shared_ptr<IColumn> &o) {
			static_cast<ColumnBase *>(o.get())->position = this->position;
			others.push_back(o);
		}
		bool selectable() const override {
			return getType()->selectabe();
		}
		size_t getSize() const override {
			return gtType()->getSize();
		}
	};


	template<class T>
	class Column : public ColumnBase , public Type<T> {
	protected:
	public:
		Column(size_t offset, const std::string &name, int position) :
			ColumnBase(offset, name position) {}
		virtual ~Column() {}

		void set(IStatement &writer, const  void *data) override {
			Type<T>::set(writer, adjust(data), *this);
		}

		void get(IResultSet &reader, void *data) override {
			Type<T>::get(reader, adjust(data), *this);
			for (auto &v : others) //populate others
				v->get(reader, data);
		}

		bool resolve(Store &store, void *data) const override {
			return Type<T>::resolve(store, adjust(data));
		}

		const IType *getType() const override {
			return this;
		}
	};

	template<class T>
	class Column<const logic::Function<T> *> : public ColumnBase {
	public:
		Column(size_t offset, int column) :
			ColumnBase(offset, "", column)
		{
		}
		~Column() {}

		void set(IStatement &writer, const  void *data) override {}
		void get(IResultSet &reader, void *data) override {}

		const IType *getType() const override {return 0;}

		bool resolve(Store &, void *data) const override {
			T &t = *(T*)data;
			const logic::Function<T> **ptr = ((const logic::Function<T>**)adjust(data));
			*ptr = logic::Functions<T>::getFunction(static_cast<typename T::key_type>(t));
			return true;
		}
	};

/*
	template<class T>
	class Column : public ColumnBase {
	public:
		Column(size_t offset, const std::string &name, int column) :
			ColumnBase(offset, name, column)
		{
			ColumnBase::xForm<T >();
		}
		~Column() {}
		Column<T> &setSize(size_t size) {
			getBuff().resize(size+1);
			return *this;
		}
	};

	template<>
	class Column<Sequence > : public ColumnBase {
		using T = Sequence;
		T sequence,increment;
	public:
		Column(size_t offset, const std::string &name, int column) :
			ColumnBase(offset, name, column)
		{
			ColumnBase::xForm<T>();
		}
		void get(IResultSet &reader, void *data) override {
			Sequence &seq = *static_cast<Sequence *>(adjust(data));
			seq = sequence;
			sequence+=increment;
			for (auto &v : others) //populate others
				v->get(reader, data);
		}
		Column &startWith(const T::type &v) {
			sequence = Sequence(v);
			return *this;
		}
		Column &increamentBy(const T::type &v) {
			increment = Sequence(v);
			return *this;
		}
		~Column() {}
	};

	template<class T>
	class Column<const logic::Function<T> *> : public ColumnBase {
	public:
		Column(size_t offset, int column) :
			ColumnBase(offset, "", column)
		{
		}
		~Column() {}
		bool resolve(Store &, void *data) const override {
			T &t = *(T*)data;
			const logic::Function<T> **ptr = ((const logic::Function<T>**)adjust(data));
			*ptr = logic::Functions<T>::getFunction(static_cast<typename T::key_type>(t));
			return true;
		}
	};

	template<>
	class Column<tm> : public ColumnBase {
		std::string format;
	public:
		Column(size_t offset, int column) :
			ColumnBase(offset, "", column)
		{
			ColumnBase::xForm<std::string >();
			setDateFormat("%4d-%02d-%02d %02d:%02d:%02d");
			getBuff().resize(20);
		}
		~Column() {}
		void setDateFormat(std::string format) {
			this->format = format;
		}
		std::string getDateFormat() const override {
			return format;
		}
	};

	template<>
	class Column<Number> : public ColumnBase {
		unsigned int m; /// maximum number of digits (the precision)
		unsigned int d; /// number of digits to the right of the decimal point (the scale)
	public:
		Column(size_t offset, const std::string &name, int column) :
			ColumnBase(offset, name, column),m(10),d(0)
		{
			xForm<Number >();
		}
		~Column() {}
		int getPrecision() const override {
			return m;
		}
		int getScale() const override {
			return d;
		}
		Column<Number> & setPrecision(int m) {
			this->m=m;
			return *this;
		}
		Column<Number> & setScale(int d)  {
			this->d=d;
			return *this;
		}
	};

	template<int N>
	class Column<dec::decimal<N> > : public Column<typename Type<dec::decimal<N>>::XTYPE > {
		using BASE = Column<typename Type<dec::decimal<N>>::XTYPE > ;
	public:
		Column(size_t offset, const std::string &name, int column) :
			BASE(offset, name, column)
		{
			ColumnBase::xForm<dec::decimal<N> >();
			BASE::setPrecision(18);
			BASE::setScale(N);
		}
		~Column() {}
	};

	template<size_t N>
	class Column<char[N]> : public Column<typename Type<char[N]>::XTYPE > {
		size_t size;
		using BASE = Column<typename Type<char[N]>::XTYPE > ;
	public:
		Column(size_t offset, const std::string &name, int column) :
			BASE(offset, name, column),size(N)
		{
			ColumnBase::xForm<char[N] >();
		}
		~Column() {}
		Column<char[N]> &setSize(size_t size) {
			assert(size<=N);
			this->size=size;
			return *this;
		}
		size_t getSize() const override {
			return size;
		}

	};

	template<>
	class Column<std::string> : public Column<Type<std::string>::XTYPE> {
		using BASE = Column<Type<std::string>::XTYPE > ;
	public:
		Column(size_t offset, const std::string &name, int column) :
			BASE(offset, name, column)
		{
			ColumnBase::xForm<std::string >();
			setSize(255);
		}
		~Column() {}
	};
	template<>
	class Column<char> : public Column<Type<char>::XTYPE> {
		using BASE = Column<Type<char>::XTYPE > ;
	public:
		Column(size_t offset, const std::string &name, int column) :
			BASE(offset, name, column)
		{
			ColumnBase::xForm<char >();
		}
		~Column() {}
	};

	template<>
	class Column<bool> : public Column< Type<bool>::XTYPE > {
		using BASE = Column<Type<bool>::XTYPE > ;
	public:
		Column(size_t offset, const std::string &name, int column) :
			BASE(offset, name, column)
		{
			ColumnBase::xForm<bool>();

		}
		~Column() {}
		Column<bool> &boolVal(const char *TF) {
			std::array<char, 2> tf = { TF[0],TF[1] };
			ColumnBase::xForm<bool>(tf);
			return *this;
		}
	};*/

}
