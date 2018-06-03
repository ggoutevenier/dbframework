#pragma once
#include "interface.h"
#include "function.h"
#include "xform.h"

namespace dk {
	class FieldBase : public IField {
		size_t offset;
		std::string name;
		int column;
		std::vector<char> buff;
	protected:
		std::list<std::shared_ptr<IField> > others;
		std::unique_ptr<IType> type;
		void *adjust(void *data) const {
			return ((char*)data) + offset;
		}
		const void *adjust(const void *data) const {
			return ((char*)data) + offset;
		}
/*		template<class S, typename ...ARGS>
		void xForm(ARGS&&... args) {
			type = std::unique_ptr<IType>(new S(std::forward<ARGS>(args)...));
		}*/
		template<class S, typename ...ARGS>
		void xForm(ARGS&&... args) {
			type = std::unique_ptr<IType>(new XForm<S>(std::forward<ARGS>(args)...));
		}

	public:
		FieldBase(size_t offset, const std::string &name, int column) :
			offset(offset), name(name), column(column) {}
		~FieldBase() {}
		std::string getName() const override { return name; }
		void set(IStatement &writer, const  void *data) override {
			assert(type.get());
			type->set(writer, adjust(data), *this);
		}
		void get(IResultSet &reader, void *data) override {
			assert(type.get());
			type->get(reader, adjust(data), *this);
			for (auto &v : others) //populate others
				v->get(reader, data);
		}
		bool resolve(Store &store, void *data) const override {
			assert(type.get());
			return type->resolve(store, adjust(data));
		}
		const char *dataType(const IMetaData &metadata) const override {
			assert(type.get());
			return type->dataType(metadata, *this);
		}
		int getColumn() const { return column; }
		void *getBuff(size_t size) override {
			buff.resize(size);
			return buff.data();
		}
		std::vector<char> &getBuff2(size_t size) override {
			buff.resize(size);
			return buff;
		}

		void other(std::shared_ptr<IField> &o) {
			static_cast<FieldBase *>(o.get())->column = this->column;
			others.push_back(o);
		}
		bool is_selectable() const override {
			return type->is_selectable();
		}
	};

	template<class T>
	class Field : public FieldBase {
	public:
		Field(size_t offset, const std::string &name, int column) :
			FieldBase(offset, name, column)
		{
			FieldBase::xForm<T >();
		}
		~Field() {}
	};

	template<>
	class Field<bool> : public FieldBase {
		using T = bool;
	public:
		Field(size_t offset, const std::string &name, int column) :
			FieldBase(offset, name, column)
		{
			FieldBase::xForm<T>();
		}
		~Field() {}
		Field<T> &boolVal(const char *TF) {
			std::array<char, 2> tf = { TF[0],TF[1] };
			FieldBase::xForm<T>(tf);
			return *this;
		}
	};

/*	template<class T>
	class Field<Ref<T> > : public FieldBase {
	public:
		Field(size_t offset, const std::string &name, int column) :
			FieldBase(offset, name, column)
		{
			this->type = std::unique_ptr<IType>(new Type<Ref<T>>());
			//FieldBase::xForm<Type<T> >();
		}
		~Field() {}
	};*/

/*	template<>
	class Field<bool> : public FieldBase {
		using T = bool;
	public:
		Field(size_t offset, const std::string &name, int column) :
			FieldBase(offset, name, column)
		{

			FieldBase::xForm<Type<T> >();
		}
		~Field() {}
		Field<T> &boolVal(const char *TF) {
			FieldBase::xForm<XForm<T> >(TF);
			return *this;
		}
	};*/

/*	template<size_t N>
	class Field<dec::decimal<N> > : public FieldBase {
		using T = dec::decimal<N>;
	public:
		Field(size_t offset, const std::string &name, int column) :
			FieldBase(offset, name, column)
		{
//			this->type = std::unique_ptr<IType>(new XForm<T>());
			FieldBase::xForm<XForm<T> >();
		}
		~Field() {}
	};
*/
/*	template<>
	class Field<timestamp > : public FieldBase {
		using T = timestamp;
	public:
		Field(size_t offset, const std::string &name, int column) :
			FieldBase(offset, name, column)
		{
//			this->type = std::unique_ptr<IType>(new XForm<timestamp>());
			FieldBase::xForm<XForm<T> >();
		}
		~Field() {}
	};*/

	template<>
	class Field<Sequence > : public FieldBase {
		using T = Sequence;
		T sequence;
	public:
		Field(size_t offset, const std::string &name, int column) :
			FieldBase(offset, name, column)
		{
			FieldBase::xForm<T>();
		}
		void get(IResultSet &reader, void *data) override {
			Sequence &seq = *static_cast<Sequence *>(adjust(data));
			seq = sequence;
			sequence++;
			for (auto &v : others) //populate others
				v->get(reader, data);
		}
		Field &startWith(const T::type &v) {
			sequence.startWith(v);
			return *this;
		}
		Field &increaseBy(const T::type &v) {
			sequence.increaseBy(v);
			return *this;
		}
		~Field() {}
	};

	template<class T>
	class Field<const logic::Function<T> *> : public FieldBase {
	public:
		Field(size_t offset, int column) :
			FieldBase(offset, "", column)
		{
		}
		~Field() {}
		bool resolve(Store &, void *data) const override {
			T &t = *(T*)data;
			const logic::Function<T> **ptr = ((const logic::Function<T>**)adjust(data));
			*ptr = logic::Functions<T>::getFunction(static_cast<typename T::key_type>(t));
			return true;
		}
	};

}