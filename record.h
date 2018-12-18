#pragma once
#include "column.h"

namespace dk {
	class Record : public IRecord {
	private:
		std::string name;
		Columns columns, refs;
		template<class B, class T>
		size_t getOffset(B &b, T &t) {
			return (size_t)&t - (size_t)&b;
		}
		ResolveFunc resolveFunc_;
		std::unique_ptr<IColumn> logic_;
	public:
		Record(const Record &)=delete;
		Record(Record &&record) noexcept :
			name(std::move(record.name)),
			columns(std::move(record.columns)),
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
		Column<const logic::Function<T> *> &add(B &b, const logic::Function<T> *&t) {
			logic_ =
				std::make_unique<Column<const logic::Function<T> *> >(
					getOffset(b, t),
					(int)columns.size() + 1);
			return *(Column<const logic::Function<T> *>*)logic_.get();
		}
		template<class B, class T>
		Column<T> &add(B &b, T &t, const std::string &name) {
			IColumn *ptr = 0;
			for (auto &column : columns) {// replace with std::find_if
				if (name == column->getName()) {
					ptr = column.get();
					break;
				}
			}
			if (ptr) {
				std::shared_ptr<IColumn> fld =
					std::make_shared<Column<T> >(getOffset(b, t), name, 0); // check on casting/auto

				ptr->other(fld);
			}
			else {
				columns.push_back( // check on emplace
					std::make_unique<Column<T> >(
						getOffset(b, t),
						name,
						(int)columns.size() + 1)
				);
			}
			return *(Column<T>*)columns.back().get(); // might have race condition
		}

		template<class B, class T>
		Column<Ref<T> > &add(B &b, Ref<T> &t, const std::string &name) {
			refs.push_back( // check on emplace
				std::make_unique<Column<Ref<T> > >(
					getOffset(b, t),
					name,
					(int)refs.size() + 1)
			);
			return *(Column<Ref<T> >*)refs.back().get(); // might have race condition
		}

		void set(IStatement &writer, const  void *data) const override {
			for (const auto &column : columns) //check on for_each lambdas
				column->set(writer, data);
		}
		void get(IResultSet &reader, void *data) const override {
			for (const auto &column : columns)
				column->get(reader, data);
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
		const Columns &getColumns() const override { return columns; }
		std::string getName() const override { return name; }
	};
}
