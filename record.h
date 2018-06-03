#pragma once
#include "field.h"
#include <boost/core/noncopyable.hpp>
namespace dk {
	class Record : boost::noncopyable , public IRecord {
//	public:
//		using Fields = std::list<std::unique_ptr<IField> >;
//		using ResolveFunc = bool(*)(Store&, void*);
	private:
		std::string name;
		Fields fields, refs;
		template<class B, class T>
		size_t getOffset(B &b, T &t) {
			return (size_t)&t - (size_t)&b;
		}
		ResolveFunc resolveFunc;
		std::unique_ptr<IField> logic;
	public:
		Record(
			const std::string name,
			ResolveFunc f = [](Store &, void *)->bool { return true; }
		) : name(name), resolveFunc(f) {}
		~Record() {}
		template<class B, class T>
		Field<const logic::Function<T> *> &add(B &b, const logic::Function<T> *&t) {
			logic =
				std::make_unique<Field<const logic::Function<T> *> >(
					getOffset(b, t),
					(int)fields.size() + 1);
			return *(Field<const logic::Function<T> *>*)logic.get();
		}
		template<class B, class T>
		Field<T> &add(B &b, T &t, const std::string &name) {
			IField *ptr = 0;
			for (auto &v : fields) {
				if (name == v->getName()) {
					ptr = v.get();
					break;
				}
			}
			if (ptr) {
				std::shared_ptr<IField> fld =
					std::make_shared<Field<T> >(getOffset(b, t), name, 0);

				ptr->other(fld);
			}
			else {
				fields.push_back(
					std::make_unique<Field<T> >(
						getOffset(b, t),
						name,
						(int)fields.size() + 1)
				);
			}
			return *(Field<T>*)fields.back().get(); // might have race condition
		}

		template<class B, class T>
		Field<Ref<T> > &add(B &b, Ref<T> &t, const std::string &name) {
			refs.push_back(
				std::make_unique<Field<Ref<T> > >(
					getOffset(b, t),
					name,
					(int)refs.size() + 1)
			);
			return *(Field<Ref<T> >*)refs.back().get(); // might have race condition
		}

		void set(IStatement &writer, const  void *data) const override {
			for (const std::unique_ptr<IField> &field : fields)
				field->set(writer, data);
		}
		void get(IResultSet &reader, void *data) const override {
			for (const std::unique_ptr<IField> &field : fields)
				field->get(reader, data);
		}
		bool resolve(Store &store, void *data) const override {
			bool rtn = true;
			for (const std::unique_ptr<IField> &field : refs) {
				rtn &= field->resolve(store, data);
			}
			resolveFunc(store, data);
			if (logic.get()) {
				logic->resolve(store, data);
			}
			return rtn;
		}
		const Fields &getFields() const override { return fields; }
		std::string getName() const override { return name; }
	};
}