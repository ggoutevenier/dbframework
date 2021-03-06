#pragma once
#include "interface.h"
#include "ref.h"

namespace dk {
	template<class T>
	class Type : public IType {
	protected:
//		T &typed(void *v) const {
//			return *static_cast<T *>(v);
//		}
//		const T &typed(const void *v) const {
//			return *static_cast<const T *>(v);
//		}
	public:
		using XTYPE=T;
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
		const std::string type(
			const IMetaData &mdata,
			const IColumn &field
		) const override {
			return mdata.type<T>(field);
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
			IColumn &field
		) const override {
			Type<Sequence::type>::set(writer, data, field);
		}
		void get(
			IResultSet &reader,
			void *data,
			IColumn &field
		) const override { //show never call Field<Sequence>::get should handle
			assert(false);
		}
		const std::string type(
			const IMetaData &mdata,
			const IColumn &field
		) const override {
			return Type<Sequence::type>::type(mdata, field);
		}
		bool resolve(Store &, void *data) const override {
			return true;
		}
		bool is_selectable() const override {
			return false;
		}
	};

	template<class T>
	class Type <Ref<T>> : public IType { // can it inherit from type<T>
		typedef typename T::key_type K;
		typedef Ref<T> R;
	protected:
/*		K &typed(void *data) const {
			return *static_cast<R*>(data);
		}
		const K &typed(const void *data) const {
			return *static_cast<const R*>(data);
		}*/
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
		
		bool is_selectable() const override { return false; }
	};
}
