#pragma once
#include "sequence.h"
//#include "ref.h"
#include "decimal.h"
#include "timestamp.h"
#include <array>
#include <vector>
#include <list>
#include <cassert>
#include <string>
#include <cstdint>
#include <memory>
#include <map>

namespace dk {
	class IField;
	class Store;
	class IRecord;
	template<class T>
	const std::map<typename T::key_type, T> &getMap(Store &s);

//	template<class T>
//	const std::map<typename T::key_type, T> &getMap(const Store &s);

	class IMetaData {
	protected:
		virtual const char *dataType(const bool &, const IField &f) const = 0;
		virtual const char *dataType(const int16_t &, const IField &f) const = 0;
		virtual const char *dataType(const int32_t &, const IField &f) const = 0;
		virtual const char *dataType(const int64_t &, const IField &f) const = 0;
		virtual const char *dataType(const uint16_t &, const IField &f) const = 0;
		virtual const char *dataType(const uint32_t &, const IField &f) const = 0;
		virtual const char *dataType(const uint64_t &, const IField &f) const = 0;
		virtual const char *dataType(const float &, const IField &f) const = 0;
		virtual const char *dataType(const double &, const IField &f) const = 0;
		virtual const char *dataType(const std::string &, const IField &f) const = 0;
		virtual const char *dataType(const struct tm &, const IField &f) const = 0;
	public:
		virtual ~IMetaData() {}
		virtual std::string selectSQL(const IRecord &record) const = 0;
		virtual std::string insertSQL(const IRecord &record) const = 0;
		virtual std::string createSQL(const IRecord &record) const = 0;
		template<class T>
		const char *dataType(const IField &f) const { return dataType(T(), f); }
	};


	class IResultSet {
	protected:
	public:
		virtual ~IResultSet() {}
		virtual bool next() = 0;
		virtual void getColumn(float &v, const IField &f) = 0;
		virtual void getColumn(double &v, const IField &f) = 0;
		virtual void getColumn(char &v, const IField &f) = 0;
		virtual void getColumn(bool &v, const IField &f) = 0;
		virtual void getColumn(int16_t &v, const IField &f) = 0;
		virtual void getColumn(int32_t &v, const IField &f) = 0;
		virtual void getColumn(int64_t &v, const IField &f) = 0;
		virtual void getColumn(uint16_t &v, const IField &f) = 0;
		virtual void getColumn(uint32_t &v, const IField &f) = 0;
		virtual void getColumn(uint64_t &v, const IField &f) = 0;
		virtual void getColumn(std::string &v, const IField &f) = 0;
		virtual void getColumn(struct tm &v, const IField &f) = 0;
	};

	class IStatement {
	protected:
	public:
		virtual ~IStatement() {}
		virtual std::unique_ptr<IResultSet> executeQuery() = 0;
		virtual void commit() = 0;
		virtual bool execute() = 0;
		virtual size_t executeUpdate() = 0;
		virtual void flush() = 0;
		virtual void query(const std::string &sql) = 0;
		virtual void bind(const float &v, IField &f) = 0;
		virtual void bind(const bool &v, IField &f) = 0;
		virtual void bind(const std::int16_t &v, IField &f) = 0;
		virtual void bind(const std::int32_t &v, IField &f) = 0;
		virtual void bind(const std::int64_t &v, IField &f) = 0;
		virtual void bind(const std::uint16_t &v, IField &f) = 0;
		virtual void bind(const std::uint32_t &v, IField &f) = 0;
		virtual void bind(const std::uint64_t &v, IField &f) = 0;
		virtual void bind(const double &v, IField &f) = 0;
		virtual void bind(const char &v, IField &f) = 0;
		virtual void bind(const std::string &v, IField &f) = 0;
		virtual void bind(const struct tm &v, IField &f) = 0;
	};

	class IConnection {
	public:
		virtual ~IConnection() {}
		virtual std::unique_ptr<IStatement> createStatement(const std::string &sql) = 0;
		virtual void execute(const std::string &sql) = 0;
		virtual IMetaData &getMetaData() = 0;
		virtual std::unique_ptr<IStatement> createStatement() = 0;
		virtual void rollback() = 0;
		virtual void commit() = 0;
	};

	class IType {
	public:
		virtual ~IType() {}
		virtual void set(IStatement &writer, const  void *data, IField &field) const = 0;
		virtual void get(IResultSet &reader, void *data, IField &field) const = 0;
		virtual const char *dataType(const IMetaData &mdata, const IField &field) const = 0;
		virtual bool resolve(Store &, void *data) const = 0;
		virtual bool is_selectable() const = 0;
	};

	class IField {
	public:
		virtual std::string getName() const = 0;
		virtual void set(IStatement &writer, const  void *data) = 0;
		virtual void get(IResultSet &reader, void *data) = 0;
		virtual const char *dataType(const IMetaData &metadata) const = 0;
		virtual bool resolve(Store &, void *data) const = 0;
		virtual int getColumn() const = 0;
		virtual std::vector<char> &getScratch(size_t size) = 0;
		virtual void other(std::shared_ptr<IField> &other) = 0;
		virtual bool is_selectable() const = 0;
		virtual ~IField() {}
	};

	class IRecord {
	public:
		using Fields = std::list<std::unique_ptr<IField> >;
		using ResolveFunc = bool(*)(Store&, void*);
		virtual void set(IStatement &writer, const  void *data) const = 0;
		virtual void get(IResultSet &reader, void *data) const = 0;
		virtual bool resolve(Store &store, void *data) const = 0;
		virtual const Fields &getFields() const = 0;
		virtual std::string getName() const = 0;
		virtual ~IRecord() {}
	};
}
