#pragma once
//#include "interface.h"
#include "function.h"
#include "xform.h"
#include <algorithm>

namespace dk {
	class ColumnBase : public IColumn {
		size_t offset;
		std::string name;
		int position;
	protected:
		std::list<std::shared_ptr<IColumn> > others;
		std::unique_ptr<IType> type_;
		std::vector<char> buff;
		/// function returns the a pointer offset to the correct position
		/// assuming the pointer passed in is at the first position of the record
		void *adjust(void *data) const {
			return ((char*)data) + offset;
		}
		const void *adjust(const void *data) const {
			return ((char*)data) + offset;
		}
		/// function to set a transform of the column as it is transitioned
		/// between the database and c classes
		template<class S, typename ...ARGS>
		void xForm(ARGS&&... args) {
			type_ = std::unique_ptr<IType>(new XForm<S>(std::forward<ARGS>(args)...));
		}
	public:
		ColumnBase(size_t offset, const std::string &name, int position) :
			offset(offset), name(name), position(position) {}
		virtual ~ColumnBase() {}
		/// get the name of the column
		std::string getName() const override { return name; }

		/// function to write column of data out to the database
		void set(IStatement &writer, const  void *data) override {
			assert(type_.get());
			type_->set(writer, adjust(data), *this);
		}
		/// function to read column of data from the database
		void get(IResultSet &reader, void *data) override {
			assert(type_.get());
			type_->get(reader, adjust(data), *this);
			for (auto &v : others) //populate others
				v->get(reader, data);
		}
		/// function to resolve any column pointer if needed
		bool resolve(Store &store, void *data) const override {
			assert(type_.get());
			return type_->resolve(store, adjust(data));
		}
		/// function to return the database type of the column
		const std::string type(const IMetaData &metadata) const override {
			assert(type_.get());
			return type_->type(metadata, *this);
		}
		/// get the position of the column in the query
		int getColumn() const { return position; }

		std::vector<char> &getBuff() override {
			return buff;
		}

		void other(std::shared_ptr<IColumn> &o) {
			static_cast<ColumnBase *>(o.get())->position = this->position;
			others.push_back(o);
		}
		// is the column selectable/insertable from the database
		bool is_selectable() const override {
			return type_->is_selectable();
		}
		const IType &getType() const override {
			return *type_;
		}
		size_t getSize() const override {
			return buff.size();
		}
		int getPrecision() const override {
			return 0;
		}
		int getScale() const override {
			return 0;
		}
		std::string getDateFormat() const override {
			return "";
		}
		void toBuff(std::string str) override {
			size_t n = std::min(str.length(),getBuff().size());

			if(n==0) {
				getBuff().at(0)=0;
			}
			else {
				auto it=std::copy_n(str.begin(),n,getBuff().begin());
				if(n<getBuff().size()) {
					*it=0;
				}
			}
		}
	};

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
	class Column<Number> : public ColumnBase {
		unsigned int m; /// maximum number of digits (the precision)
		unsigned int d; /// number of digits to the right of the decimal point (the scale)
	public:
		Column(size_t offset, const std::string &name, int column) :
			ColumnBase(offset, name, column),m(10),d(0)
		{
			ColumnBase::xForm<Number >();
		}
		~Column() {}
		int getPrecision() const override {
			return m;
		}
		int getScale() const override {
			return d;
		}
		Column<Number> & setPrecision(int m) /*override*/ {
			this->m=m;
			return *this;
		}
		Column<Number> & setScale(int d) /*override*/ {
			this->d=d;
			return *this;
		}
	};

	template<int N>
	class Column<dec::decimal<N> > : public Column<Number > {
	public:
		Column(size_t offset, const std::string &name, int column) :
			Column<Number>(offset, name, column)
		{
			ColumnBase::xForm<dec::decimal<N> >();
			setPrecision(18);
			setScale(N);
		}
		~Column() {}
	};

	template<size_t N>
	class Column<char[N]> : public Column<char *> {
		size_t size;
	public:
		Column(size_t offset, const std::string &name, int column) :
			Column<char *>(offset, name, column),size(N)
		{
			ColumnBase::xForm<char[N] >();
		}
		~Column() {}
		Column<char[N]> &setSize(size_t size) /*override*/ {
			assert(size<=N);
			this->size=size;
			return *this;
		}
		size_t getSize() const override {
			return size;
		}

	};

	template<>
	class Column<char> : public Column<char[1]> {
	public:
		Column(size_t offset, const std::string &name, int column) :
			Column<char[1]>(offset, name, column)
		{
			ColumnBase::xForm<char >();
		}
		~Column() {}
	};

	template<>
	class Column<std::string> : public Column<char *> {
	public:
		Column(size_t offset, const std::string &name, int column) :
			Column<char*>(offset, name, column)
		{
			ColumnBase::xForm<std::string >();
			setSize(255);
		}
		~Column() {}
	};

	template<>
	class Column<bool> : public Column<char> {
	public:
		Column(size_t offset, const std::string &name, int column) :
			Column<char>(offset, name, column)
		{
			ColumnBase::xForm<bool>();

		}
		~Column() {}
		Column<bool> &boolVal(const char *TF) {
			std::array<char, 2> tf = { TF[0],TF[1] };
			ColumnBase::xForm<bool>(tf);
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

}
