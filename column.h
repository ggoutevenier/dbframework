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
			return static_cast<const char*>(data) + offset;
		}
	public:
		ColumnBase(size_t offset, const std::string &name, int position) :
			offset(offset), name(name), position(position) {}
		virtual ~ColumnBase() {}

		std::string getColumnName() const override { return name; }

		int getColumnPosition() const override{ return position; }

		void other(std::shared_ptr<IColumn> &o) {
			static_cast<ColumnBase *>(o.get())->position = this->position;
			others.push_back(o);
		}
		bool selectable() const override {
			return getType().selectable();
		}
	};

	template<class T>
	class Column : public ColumnBase , public Type<T> {
	protected:
	public:
		Column(size_t offset, const std::string &name, int position) :
			ColumnBase(offset, name, position) {}
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

		IType &getType() override {
			return Type<T>::getType();
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

		IType &getType() override {
			assert(0);
			return *(IType*)nullptr;
		} // TODO fix this

		bool resolve(Store &, void *data) const override {
			T &t = *(T*)data;
			const logic::Function<T> **ptr = ((const logic::Function<T>**)adjust(data));
			*ptr = logic::Functions<T>::getFunction(static_cast<typename T::key_type>(t));
			return true;
		}
	};
}
