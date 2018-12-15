#pragma once
#include "field.h"

namespace dk {
	class Record : public IRecord {
	private:
		std::string name;
		Fields fields, refs;
		template<class B, class T>
		size_t getOffset(B &b, T &t) {
			return (size_t)&t - (size_t)&b;
		}
		ResolveFunc resolveFunc_;
		std::unique_ptr<IField> logic_;
	public:
		Record(const Record &)=delete;
		Record(Record &&record) noexcept :
			name(std::move(record.name)),
			fields(std::move(record.fields)),
			refs(std::move(record.refs)),
			resolveFunc_(std::move(record.resolveFunc_)),
			logic_(std::move(record.logic_)){

		}
		Record(
			const std::string name,
			ResolveFunc f = [](Store &, void *)->bool { return true; }
		) : name(name), resolveFunc_(f) {}
		~Record() {}
		template<class B, class T>
		Field<const logic::Function<T> *> &add(B &b, const logic::Function<T> *&t) {
			logic_ =
				std::make_unique<Field<const logic::Function<T> *> >(
					getOffset(b, t),
					(int)fields.size() + 1);
			return *(Field<const logic::Function<T> *>*)logic_.get();
		}
		template<class B, class T>
		Field<T> &add(B &b, T &t, const std::string &name) {
			IField *ptr = 0;
			for (auto &v : fields) {// replace with std::find_if
				if (name == v->getName()) {
					ptr = v.get();
					break;
				}
			}
			if (ptr) {
				std::shared_ptr<IField> fld =
					std::make_shared<Field<T> >(getOffset(b, t), name, 0); // check on casting/auto

				ptr->other(fld);
			}
			else {
				fields.push_back( // check on emplace
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
			refs.push_back( // check on emplace
				std::make_unique<Field<Ref<T> > >(
					getOffset(b, t),
					name,
					(int)refs.size() + 1)
			);
			return *(Field<Ref<T> >*)refs.back().get(); // might have race condition
		}

		void set(IStatement &writer, const  void *data) const override {
			for (const auto &field : fields) //check on for_each lambdas
				field->set(writer, data);
		}
		void get(IResultSet &reader, void *data) const override {
			for (const auto &field : fields)
				field->get(reader, data);
		}
		bool resolve(Store &store, void *data) const override {
			bool rtn = true;
			for (const auto &field : refs) {
				rtn &= field->resolve(store, data);
			}
			resolveFunc_(store, data);
			if (logic_.get()) { // can we default to a noop lamdba func in constructor
				logic_->resolve(store, data);
			}
			return rtn;
		}
		const Fields &getFields() const override { return fields; }
		std::string getName() const override { return name; }
	};
}
