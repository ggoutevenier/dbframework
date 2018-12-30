#pragma once
#include "record.h"
namespace dk {

	class RecordBuilder : public Record {
	private:
		template<class B, class T>
		size_t getOffset(B &b, T &t) {
			return (size_t)&t - (size_t)&b;
		}
	public:
		RecordBuilder (
			const std::string name,
			ResolveFunc f = [](Store &, void *)->bool { return true; }
		) : Record(name, f) {}
		template<class B, class T>
		Column<T> &add(B &b, T &t, const std::string &name) {
			IColumn *ptr = 0;
			for (auto &column : columns) {// replace with std::find_if
				if (name == column->getColumnName()) {
					ptr = column.get();
					break;
				}
			}
			if (ptr) {
				auto fld = static_cast<std::shared_ptr<IColumn> >(
					std::make_shared<Column<T> >(getOffset(b, t), name, 0));

//				std::shared_ptr<IColumn> fld =
//					std::make_shared<Column<T> >(getOffset(b, t), name, 0); // check on casting/auto

				ptr->other(fld);
			}
			else {
				auto v=std::make_unique<Column<T> >(
						getOffset(b, t),
						name,
						(int)columns.size() + 1);
				ptr=v.get();
				columns.push_back(std::move(v));
			}
			return *static_cast<Column<T>*>(ptr);
		}

		template<class B, class T>
		Column<const logic::Function<T> *>  &add(B &b, const logic::Function<T> * &t) {
			using F = Column<const logic::Function<T>*>;
			auto v = std::make_unique<F>(
					getOffset(b, t),
					(int)refs.size() + 1);
			refs.push_back(std::move(v));

			return *static_cast<F *>(v.get());
		}

		template<class B, class T>
		Column<Ref<T> > &add(B &b, Ref<T> &t, const std::string &name) {
			using R = Column<Ref<T> >;
			auto v = std::make_unique<R >(
					getOffset(b, t),
					name,
					(int)refs.size() + 1);

			refs.push_back(std::move(v));
			return *static_cast<R *>(v.get());
		}
	};

	template<class T>
	class metadata : public RecordBuilder , public T {
		template<class S>
		Column<S> &add(S &s, const std::string &name) {
			return RecordBuilder::add(static_cast<const T&>(*this), s, name);
		}
		Column<const logic::Function<T> *> &add(const logic::Function<T> *&f) {
			return RecordBuilder::add(static_cast<const T&>(*this), f);
		}

	public:
		metadata();
	};
}
