#pragma once
//#include "interface.h"
#include "function.h"
#include "xform.h"

namespace dk {
	class ColumnBase : public IColumn {
		size_t offset;
		std::string name;
		int position;
		std::vector<char> buff;
	protected:
		std::list<std::shared_ptr<IColumn> > others;
		std::unique_ptr<IType> type_;
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

		std::vector<char> &getScratch(size_t size) override {
			buff.resize(size);
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
	};

	template<>
	class Column<bool> : public ColumnBase {
		using T = bool;
	public:
		Column(size_t offset, const std::string &name, int column) :
			ColumnBase(offset, name, column)
		{
			ColumnBase::xForm<T>();
		}
		~Column() {}
		Column<T> &boolVal(const char *TF) {
			std::array<char, 2> tf = { TF[0],TF[1] };
			ColumnBase::xForm<T>(tf);
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

}
