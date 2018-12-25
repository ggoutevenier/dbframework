#pragma once
#include "sequence.h"
//#include "ref.h"
#include "decimal.h"
#include "number.h"
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
	class IColumn;
	class Store;
	class IRecord;
	template<class T>
	const std::map<typename T::key_type, T> &getMap(Store &s);

	class IMetaData {
	protected:
		virtual const std::string typeBool(const IColumn &f) const = 0;
		virtual const std::string typeInt16(const IColumn &f) const = 0;
		virtual const std::string typeInt32(const IColumn &f) const = 0;
		virtual const std::string typeInt64(const IColumn &f) const = 0;
		virtual const std::string typeFloat(const IColumn &f) const = 0;
		virtual const std::string typeDouble(const IColumn &f) const = 0;
		virtual const std::string typeNumber(const IColumn &f) const = 0;
		virtual const std::string typeString(const IColumn &f) const = 0;
		virtual const std::string typeDate(const IColumn &f) const = 0;
		virtual std::string bindVar(const std::string column) const = 0;
	public:
		virtual ~IMetaData() {}
		virtual std::string selectSQL(const IRecord &record) const = 0;
		virtual std::string insertSQL(const IRecord &record) const = 0;
		virtual std::string createSQL(const IRecord &record) const = 0;
		template<class T> const std::string type(const IColumn &f) const;
	};


	class IResultSet {
	protected:
	public:
		virtual ~IResultSet() {}
		virtual bool next() = 0;
		virtual void get(float &v, IColumn &f) = 0;
		virtual void get(double &v, IColumn &f) = 0;
		virtual void get(bool &v, IColumn &f) = 0;
		virtual void get(int16_t &v, IColumn &f) = 0;
		virtual void get(int32_t &v, IColumn &f) = 0;
		virtual void get(int64_t &v, IColumn &f) = 0;
		virtual void get(uint16_t &v, IColumn &f) = 0;
		virtual void get(uint32_t &v, IColumn &f) = 0;
		virtual void get(uint64_t &v, IColumn &f) = 0;
		virtual void get(struct tm &v, IColumn &f) = 0;
		virtual void get(Number &v, IColumn &f) = 0;
//		virtual void get(char &v, IColumn &f) = 0;
		virtual void get(char *v, IColumn &f) = 0;
		virtual void get(IColumn &f) = 0;
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
		virtual void set(const float &v, IColumn &f) = 0;
		virtual void set(const bool &v, IColumn &f) = 0;
		virtual void set(const std::int16_t &v, IColumn &f) = 0;
		virtual void set(const std::int32_t &v, IColumn &f) = 0;
		virtual void set(const std::int64_t &v, IColumn &f) = 0;
		virtual void set(const std::uint16_t &v, IColumn &f) = 0;
		virtual void set(const std::uint32_t &v, IColumn &f) = 0;
		virtual void set(const std::uint64_t &v, IColumn &f) = 0;
		virtual void set(const double &v, IColumn &f) = 0;
//		virtual void set(const char &v, IColumn &f) = 0;
		virtual void set(const char *v, IColumn &f) = 0;
//		virtual void set(const std::string &v, IColumn &f) = 0;
		virtual void set(const struct tm &v, IColumn &f) = 0;
		virtual void set(const Number &v, IColumn &f) = 0;
		virtual void set( IColumn &f) = 0;
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
		virtual void set(IStatement &writer, const  void *data, IColumn &field) const = 0;
		virtual void get(IResultSet &reader, void *data, IColumn &field) const = 0;
		virtual const std::string type(const IMetaData &mdata, const IColumn &field) const = 0;
		virtual bool resolve(Store &, void *data) const = 0;
		virtual bool is_selectable() const = 0;
	};

	class IColumn {
	public:
		virtual std::string getName() const = 0;
		virtual void set(IStatement &writer, const  void *data) = 0;
		virtual void get(IResultSet &reader, void *data) = 0;
		virtual const std::string type(const IMetaData &metadata) const = 0;
		virtual bool resolve(Store &, void *data) const = 0;
		virtual int getColumn() const = 0;
//		virtual std::vector<char> &getScratch(size_t size) = 0;
		virtual std::vector<char> &getBuff() = 0;
		const std::vector<char> &getBuff() const {return const_cast<IColumn*>(this)->getBuff();}
		virtual void other(std::shared_ptr<IColumn> &other) = 0;
		virtual bool is_selectable() const = 0;
		virtual ~IColumn() {}
		virtual const IType &getType() const = 0;
		virtual size_t getSize() const = 0;
//		virtual IColumn &setSize(size_t size) = 0;
		virtual int getPrecision() const = 0;
		virtual int getScale() const = 0;
//		virtual IColumn &setPrecision(int size) = 0;
		virtual std::string getDateFormat() const = 0;
		virtual void toBuff(std::string ) = 0;
	};

	class IRecord {
	public:
		using Columns = std::list<std::unique_ptr<IColumn> >;
		using ResolveFunc = bool(*)(Store&, void*);
		virtual void set(IStatement &writer, const  void *data) const = 0;
		virtual void get(IResultSet &reader, void *data) const = 0;
		virtual bool resolve(Store &store, void *data) const = 0;
		virtual const Columns &getColumns() const = 0;
		virtual std::string getName() const = 0;
		virtual ~IRecord() {}
	};

	template<> inline const std::string IMetaData::type<bool>(const IColumn &f) const {
		return typeBool(f);
	}

	template<> inline const std::string IMetaData::type<int16_t>(const IColumn &f) const {
		return typeInt16(f);
	}

	template<> inline const std::string IMetaData::type<int32_t>(const IColumn &f) const {
		return typeInt32(f);
	}

	template<> inline const std::string IMetaData::type<int64_t>(const IColumn &f) const {
		return typeInt64(f);
	}

	template<> inline const std::string IMetaData::type<uint16_t>(const IColumn &f) const {
		return typeInt16(f);
	}

	template<> inline const std::string IMetaData::type<uint32_t>(const IColumn &f) const {
		return typeInt32(f);
	}

	template<> inline const std::string IMetaData::type<uint64_t>(const IColumn &f) const {
		return typeInt64(f);
	}

	template<> inline const std::string IMetaData::type<float>(const IColumn &f) const {
		return typeFloat(f);
	}

	template<> inline const std::string IMetaData::type<double>(const IColumn &f) const {
		return typeDouble(f);
	}

	template<> inline const std::string IMetaData::type<std::string>(const IColumn &f) const {
		return typeString(f);
	}
	template<> inline const std::string IMetaData::type<char *>(const IColumn &f) const {
		return typeString(f);
	}

	template<> inline const std::string IMetaData::type<struct tm>(const IColumn &f) const {
		return typeInt16(f);
	}

	template<> inline const std::string IMetaData::type<Number>(const IColumn &f) const {
		return typeNumber(f);
	}
}
