#pragma once
#include "decimal.h"
#include "number.h"
#include <array>
#include <vector>
#include <list>
#include <cassert>
#include <string>
#include <cstdint>
#include <memory>
#include <map>

namespace dk {
	class IColumn;
	class Store;
	class IRecord;
	class IType;
	template<class T>
	const std::map<typename T::key_type, T> &getMap(Store &s);
	template<class T> class Type;

	class IMetaData {
	protected:
		virtual const std::string typeBool(const IType &f) const = 0;
		virtual const std::string typeInt16(const IType &f) const = 0;
		virtual const std::string typeInt32(const IType &f) const = 0;
		virtual const std::string typeInt64(const IType &f) const = 0;
		virtual const std::string typeFloat(const IType &f) const = 0;
		virtual const std::string typeDouble(const IType &f) const = 0;
		virtual const std::string typeNumber(const IType &f) const = 0;
		virtual const std::string typeString(const IType &f) const = 0;
		virtual const std::string typeDate(const IType &f) const = 0;
		virtual std::string bindVar(const std::string column) const = 0;
	public:
		virtual ~IMetaData() {}
		virtual std::string selectSQL(const IRecord &record) const = 0;
		virtual std::string insertSQL(const IRecord &record) const = 0;
		virtual std::string createSQL(const IRecord &record) const = 0;
		template<class T> const std::string type(const IType &f) const;
	};


	class IResultSet {
	protected:
	public:
		virtual ~IResultSet() {}
		virtual bool next() = 0;
		virtual void get(float &v, IType &f,uint32_t pos) = 0;
		virtual void get(double &v, IType &f,uint32_t pos) = 0;
		virtual void get(bool &v, IType &f,uint32_t pos) = 0;
		virtual void get(int16_t &v, IType &f,uint32_t pos) = 0;
		virtual void get(int32_t &v, IType &f,uint32_t pos) = 0;
		virtual void get(int64_t &v, IType &f,uint32_t pos) = 0;
		virtual void get(uint16_t &v, IType &f,uint32_t pos) = 0;
		virtual void get(uint32_t &v, IType &f,uint32_t pos) = 0;
		virtual void get(uint64_t &v, IType &f,uint32_t pos) = 0;
		virtual void get(struct tm &v, IType &f,uint32_t pos) = 0;
		virtual void get(Number &v, IType &f,uint32_t pos) = 0;
		virtual void get(char *v, IType &f,uint32_t pos) = 0;
		virtual void get(std::string &v, IType &f,uint32_t pos) = 0;
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
		virtual void set(const float &v, IType &f,uint32_t pos) = 0;
		virtual void set(const bool &v, IType &f,uint32_t pos) = 0;
		virtual void set(const std::int16_t &v, IType &f,uint32_t pos) = 0;
		virtual void set(const std::int32_t &v, IType &f,uint32_t pos) = 0;
		virtual void set(const std::int64_t &v, IType &f,uint32_t pos) = 0;
		virtual void set(const std::uint16_t &v, IType &f,uint32_t pos) = 0;
		virtual void set(const std::uint32_t &v, IType &f,uint32_t pos) = 0;
		virtual void set(const std::uint64_t &v, IType &f,uint32_t pos) = 0;
		virtual void set(const double &v, IType &f,uint32_t pos) = 0;
		virtual void set(const char *v, IType &f,uint32_t pos) = 0;
		virtual void set(const struct tm &v, IType &f,uint32_t pos) = 0;
		virtual void set(const Number &v, IType &f,uint32_t pos) = 0;
		virtual void set(const std::string &v, IType &f,uint32_t pos) = 0;
	};

	class IConnection {
	public:
		virtual ~IConnection() {}
		virtual std::unique_ptr<IStatement> createStatement(const std::string &sql) = 0;
		virtual void execute(const std::string &sql) = 0;
		virtual IMetaData &getMetaData() = 0;
		virtual void rollback() = 0;
		virtual void commit() = 0;
	};

	class IType {
	public:
		virtual ~IType() {}
		virtual void set(IStatement &writer, const  void *data, uint32_t pos) = 0;
		virtual void get(IResultSet &reader, void *data,uint32_t pos) = 0;
		virtual const std::string name(const IMetaData &mdata) const = 0;
		virtual bool resolve(Store &, void *data) const = 0;
		virtual bool selectable() const = 0;
		virtual size_t getSize() const = 0;
	};

	class IColumn {
	public:
		virtual std::string getColumnName() const = 0;
		virtual void set(IStatement &writer, const  void *data) = 0;
		virtual void get(IResultSet &reader, void *data) = 0;
		virtual bool resolve(Store &, void *data) const = 0;
		virtual void other(std::shared_ptr<IColumn> &other) = 0;
		virtual ~IColumn() {}
		virtual std::string getTypeName(const IMetaData &mdata) const = 0;
		virtual bool selectable() const = 0;
	};

	class IRecord {
	public:
		using Columns = std::list<std::unique_ptr<IColumn> >;
		virtual void set(IStatement &writer, const  void *data) const = 0;
		virtual void get(IResultSet &reader, void *data) const = 0;
		virtual bool resolve(Store &store, void *data) const = 0;
		virtual const Columns &getColumns() const = 0;
		virtual std::string getName() const = 0;
		virtual ~IRecord() {}
	};

	template<> inline const std::string IMetaData::type<bool>(const IType &f) const {
		return typeBool(f);
	}

	template<> inline const std::string IMetaData::type<int16_t>(const IType &f) const {
		return typeInt16(f);
	}

	template<> inline const std::string IMetaData::type<int32_t>(const IType &f) const {
		return typeInt32(f);
	}

	template<> inline const std::string IMetaData::type<int64_t>(const IType &f) const {
		return typeInt64(f);
	}

	template<> inline const std::string IMetaData::type<uint16_t>(const IType &f) const {
		return typeInt16(f);
	}

	template<> inline const std::string IMetaData::type<uint32_t>(const IType &f) const {
		return typeInt32(f);
	}

	template<> inline const std::string IMetaData::type<uint64_t>(const IType &f) const {
		return typeInt64(f);
	}

	template<> inline const std::string IMetaData::type<float>(const IType &f) const {
		return typeFloat(f);
	}

	template<> inline const std::string IMetaData::type<double>(const IType &f) const {
		return typeDouble(f);
	}

	template<> inline const std::string IMetaData::type<std::string>(const IType &f) const {
		return typeString(f);
	}
	template<> inline const std::string IMetaData::type<char *>(const IType &f) const {
		return typeString(f);
	}

	template<> inline const std::string IMetaData::type<struct tm>(const IType &f) const {
		return typeInt16(f);
	}

	template<> inline const std::string IMetaData::type<Number>(const IType &f) const {
		return typeNumber(f);
	}
}
