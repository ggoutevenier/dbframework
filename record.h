#pragma once
#include "interface.h"
#include "column.h"

namespace dk {
	class Record : public IRecord {
	protected:
		using ResolveFunc = bool(*)(Store&, void*);

		std::string name;
		Columns columns, refs;
		ResolveFunc resolveFunc_;
	public:
		Record(const Record &)=delete;
		Record(Record &&record) noexcept :
			name(std::move(record.name)),
			columns(std::move(record.columns)),
			refs(std::move(record.refs)),
			resolveFunc_(std::move(record.resolveFunc_))
			{}
		Record(
			const std::string name,
			ResolveFunc f
		) : name(name), resolveFunc_(f) {}
		~Record() {}

		void set(IStatement &writer, const  void *data) const override {
			for (const auto &column : columns)
				column->set(writer, data);
		}
		void get(IResultSet &reader, void *data) const override {
			for (const auto &column : columns)
				column->get(reader, data);
		}
		bool resolve(Store &store, void *data) const override {
			bool rtn = true;
			for (const auto &ref : refs)
				rtn &= ref->resolve(store, data);
			resolveFunc_(store, data);
			return rtn;
		}
		const Columns &getColumns() const override { return columns; }
		std::string getName() const override { return name; }
	};
}
