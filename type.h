#pragma once
#include "interface.h"
#include "store.h"

namespace dk {
	template<class T>
	class Type : public IType {
	protected:
		virtual T &typed(void *v) const { return *(T *)v; }
		virtual const T &typed(const void *v) const {
			return *(const T *)v;
		}
	public:
		virtual ~Type() {}
		void set(
			IStatement &writer,
			const  void *data,
			IField &field
		) const override {
			writer.bind(typed(data), field);
		}
		void get(
			IResultSet &reader,
			void *data,
			IField &field
		) const override {
			reader.getColumn(typed(data), field);
		}
		const char *dataType(
			const IMetaData &mdata,
			const IField &field
		) const {
			return mdata.dataType<T>(field);
		}
		bool resolve(Store &, void *data) const override {
			return true;
		}
		bool is_selectable() const override {
			return true;
		}
	};

	template<>
	class Type < Sequence > : public Type<Sequence::type> {
	public:
		virtual ~Type() {}
		void set(
			IStatement &writer,
			const  void *data,
			IField &field
		) const override {
			Type<Sequence::type>::set(writer, data, field);
		}
		void get(
			IResultSet &reader,
			void *data,
			IField &field
		) const override { //show never call Field<Sequence>::get should handle
			assert(false);
		}
		const char *dataType(
			const IMetaData &mdata,
			const IField &field
		) const {
			return Type<Sequence::type>::dataType(mdata, field);
		}
		bool resolve(Store &, void *data) const override {
			return true;
		}
		bool is_selectable() const override {
			return false;
		}
	};

	template<class T>
	class Type < Ref<T> > : public IType {
		typedef typename T::key_type K;
		typedef Ref<T> R;
	protected:
		K & typed(void *v) const {
			R &r = *(R*)data;
			return r;
		}
		const K &typed(const void *v) const {
			const R &r = *(const R*)data;
			return r;
		}
	public:
		void set(IStatement &writer, const  void *data, IField &field) const override {
			//noop
		}
		void get(IResultSet &reader, void *data, IField &field) const override {
			//noop
		}
		const char *dataType(const IMetaData &mdata, const IField &field) const {
			return 0;
		}

		bool resolve(Store &store, void *data) const override {
			R &r = *(R*)data;
//			r.ptr = reference<T>(store, (K&)r); // defined in store.h
			store.reference(r);
			return r.ptr != 0;
		}
		
		bool is_selectable() const override { return false; }
	};
}