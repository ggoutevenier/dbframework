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
/*		template<class B, class T>
		Column<const logic::Function<T> *> &add(B &b, const logic::Function<T> *&t) {
			logic_ =
				std::make_unique<Column<const logic::Function<T> *> >(
					getOffset(b, t),
					(int)columns.size() + 1);
			return *(Column<const logic::Function<T> *>*)logic_.get();
		}*/
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
		Column<const logic::Function<T> *>  &add(B &b, const logic::Function<T> * &t) {
			refs.push_back( // check on emplace
				std::make_unique<Column<const logic::Function<T> * > >(
					getOffset(b, t),
					(int)refs.size() + 1)
			);
			return *(Column<const logic::Function<T> * >*)refs.back().get(); // might have race condition
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
