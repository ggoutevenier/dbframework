#pragma once
#include "interface.h"
#include <string>
#include <memory>
#include <boost/lexical_cast.hpp>
#include "type.h"
#include "xform.h"
#include <iostream>

namespace dk {
	class MetaData : public IMetaData{
	protected:
		IConnection &conn;
		const std::string typeBool(const IType &f) const override;
		const std::string typeInt16(const IType &f) const override;
		const std::string typeInt32(const IType &f) const override;
		const std::string typeInt64(const IType &f) const override;
		const std::string typeFloat(const IType &f) const override;
		const std::string typeDouble(const IType &f) const override;
		const std::string typeString(const IType &f) const override;
		const std::string typeDate(const IType &f) const override;
		const std::string typeNumber(const IType &f) const override;
	public:
		MetaData(IConnection &c) : conn(c) {}
		virtual ~MetaData() {}
		std::string selectSQL(const IRecord &record) const override;
		std::string insertSQL(const IRecord &record) const override;
		std::string createSQL(const IRecord &record) const override;
	};

	class ResultSet : public IResultSet {
	protected:
		IStatement &stmt;
		template<class T> void getString(T &t, IType &f,uint32_t pos);
		template<class T> void getInt64(T &t, IType &f,uint32_t pos);
		void getDouble(float &t, IType &f,uint32_t pos);
	public:
		virtual ~ResultSet() override {}
		ResultSet(IStatement &stmt) :stmt(stmt) {}
		void get(float &v, IType &f,uint32_t pos) override;
		void get(double &v, IType &f,uint32_t pos) override=0;
		void get(char *v, IType &f,uint32_t pos) override=0;
		void get(bool &v, IType &f,uint32_t pos) override;
		void get(int16_t &v, IType &f,uint32_t pos) override;
		void get(int32_t &v, IType &f,uint32_t pos) override;
		void get(int64_t &v, IType &f,uint32_t pos) override;
		void get(uint16_t &v, IType &f,uint32_t pos) override;
		void get(uint32_t &v, IType &f,uint32_t pos) override;
		void get(uint64_t &v, IType &f,uint32_t pos) override;
		void get(struct tm &v, IType &f,uint32_t pos) override;
		void get(Number &v, IType &f,uint32_t pos) override;
		void get(std::string &v, IType &f,uint32_t pos) override;
	};

	class Statement : public IStatement {
	protected:
		IConnection &conn;
		std::string sql;
		template<class S, class T> void set_(const T &t, IType &f,uint32_t pos);
	public:
		Statement(IConnection &conn) :conn(conn) {}
		virtual ~Statement() {}
		void commit() override;
		void set(const float &v, IType &f,uint32_t pos) override;
		void set(const bool &v, IType &f,uint32_t pos) override;
		void set(const std::int16_t &v, IType &f,uint32_t pos) override;
		void set(const std::int32_t &v, IType &f,uint32_t pos) override;
		void set(const std::int64_t &v, IType &f,uint32_t pos) override;
		void set(const std::uint16_t &v, IType &f,uint32_t pos) override;
		void set(const std::uint32_t &v, IType &f,uint32_t pos) override;
		void set(const std::uint64_t &v, IType &f,uint32_t pos) override;
		void set(const double &v, IType &f,uint32_t pos) override=0;
		void set(const char *v, IType &f,uint32_t pos) override=0;
		void set(const struct tm &v, IType &f,uint32_t pos) override;
		void set(const Number &v, IType &f,uint32_t pos) override;
		void set(const std::string &v, IType &f,uint32_t pos) override;
	};

	class Connection : public IConnection {
	protected:
		std::unique_ptr<IMetaData> mdata;
	public:
		virtual ~Connection() {}
		std::unique_ptr<IStatement> createStatement(const std::string &sql) override;
		virtual std::unique_ptr<IStatement> createStatement() = 0;
		void execute(const std::string &sql) override;
		IMetaData &getMetaData() override;
	};

	/****************************************** Metadata ***************************/
	inline const std::string MetaData::typeBool(const IType &column)
		const {
		return IMetaData::type<int64_t>(column);
	}
	inline const std::string MetaData::typeInt16(const IType &column)
		const {
		return IMetaData::type<int64_t>(column);
	}

	inline const std::string MetaData::typeInt32(const IType &column)
		const {
		return IMetaData::type<int64_t>(column);
	}

	inline const std::string MetaData::typeInt64(const IType &column)
		const {
		return IMetaData::type<std::string>(column);
	}

	inline const std::string MetaData::typeFloat(const IType &column)
		const {
		return IMetaData::type<double>(column);
	}

	inline const std::string MetaData::typeDouble(const IType &column)
		const {
		return IMetaData::type<std::string>(column);
	}
	
	inline const std::string MetaData::typeDate(const IType &column)
		const {
		return IMetaData::type<std::string>(column);
	}

	inline const std::string MetaData::typeString(const IType &column)
		const {
		return IMetaData::type<std::string>(column);
	}

	inline const std::string MetaData::typeNumber(const IType &column)
		const {
		return IMetaData::type<double>(column);
	}

	inline std::string MetaData::selectSQL(const IRecord &record) const {
		std::stringstream ss;
		ss << "select ";
		bool first = true;
		for (auto &column : record.getColumns()) {
			if (!first) ss << ",";
			else first = false;
			ss << (column->selectable() ? "" : "null as ");
			ss << column->getColumnName();
		}
		ss << " from " << record.getName();
		return ss.str();
	}

	inline std::string MetaData::insertSQL(const IRecord &record) const {
		std::stringstream ss;
		ss << "insert into " << record.getName() << "(";
		bool first = true;
		for (auto &column : record.getColumns()) {
			if (!first) ss << ",";
			else first = false;
			ss << column->getColumnName();
		}
		ss << " ) values ( ";
		first = true;
		for (auto &column : record.getColumns()) {
			if (!first) ss << ",";
			else first = false;
			ss << bindVar(column->getColumnName());
		}
		ss << ")";

		return ss.str();
	}

	inline std::string MetaData::createSQL(const IRecord &record) const { // todo primary key
		std::stringstream ss;
		ss << "create table " << record.getName() << "(";
		bool first = true;
		for (auto &column : record.getColumns()) {
			if (!first) ss << ",";
			else first = false;
			ss << column->getColumnName() << " " << column->getTypeName(*this);
		}
		ss << ")";
		return ss.str();
	}

	/****************************************** Resultset ***************************/

	inline void ResultSet::get(bool &value, IType &itype,uint32_t pos) {
		auto type = static_cast<Type<bool>*>(&itype);
		get(type->buff,itype,pos);
		type->getV(value);
	}

	template<class T>
	inline void ResultSet::getInt64(T &value, IType &type,uint32_t pos) {
		int64_t i;
		get(i, type,pos);
		value = static_cast<T>(i);
	}
	
	inline void ResultSet::getDouble(float &value, IType &type, uint32_t pos) {
		double d;
		get(d, type, pos);
		value = static_cast<float>(d);
	}
	inline void ResultSet::get(float &value, IType &type, uint32_t pos) {
		getDouble(value, type,pos);
	}

	inline void ResultSet::get(int16_t &value, IType &type,uint32_t pos) {
		getInt64(value, type, pos);
	}
	inline void ResultSet::get(int32_t &value, IType &type,uint32_t pos) {
		getInt64(value, type, pos);
	}
	inline void ResultSet::get(int64_t &value, IType &type,uint32_t pos) {
		double d;
		get(d, type, pos);
		value = static_cast<int64_t>(d);
	}
	inline void ResultSet::get(uint16_t &value, IType &type,uint32_t pos) {
		getInt64(value, type, pos);
	}
	inline void ResultSet::get(uint32_t &value, IType &type,uint32_t pos) {
		getInt64(value, type, pos);
	}
	inline void ResultSet::get(uint64_t &value, IType &type,uint32_t pos) {
		double d;
		get(d, type, pos);
		value = static_cast<uint64_t>(d);
	}
	inline void ResultSet::get(Number &value, IType &type,uint32_t pos) {
		double d;
		get(d, type, pos);
		value.fromDouble(d);
	}

	inline void ResultSet::get(struct tm &v, IType &itype,uint32_t pos) {
		auto &type = *static_cast<Type<tm>*>(&itype);
		get(type.buff.data(), itype, pos);
		type.setV(v);
	}

	inline void ResultSet::get(std::string &v, IType &itype,uint32_t pos) {
		auto type = static_cast<Type<std::string>*>(&itype);
		get(type->getBuff(), itype, pos);
		*static_cast<std::string*>(&v) = type->asString();
	}
	/****************************************** Statement ***************************/
	template <class S,class T>
	inline void Statement::set_(const T &value, IType &type,uint32_t pos) {
		set( static_cast<S>(value), type, pos);
	}

	inline void Statement::set(const float &value, IType &type,uint32_t pos) {
		set_<double>(value, type, pos);
	}
	
	inline void Statement::set(const bool &value, IType &itype,uint32_t pos) {
		auto type = static_cast<Type<bool>*>(&itype);
		type->setV(value);
		set(type->buff,itype,pos);
	}
	
	inline void Statement::set(const std::int16_t &value, IType &type,uint32_t pos) {
		set_<int32_t>(value, type, pos);
	}
	
	inline void Statement::set(const std::int32_t &value, IType &type,uint32_t pos) {
		set_<int64_t>(value, type, pos);
	}
	
	inline void Statement::set(const std::int64_t &value, IType &type,uint32_t pos) {
		double d=value;
		set(d, type, pos);
	}
	
	inline void Statement::set(const std::uint16_t &value, IType &type,uint32_t pos) {
		set_<int32_t>(value, type, pos);
	}
	
	inline void Statement::set(const std::uint32_t &value, IType &type,uint32_t pos) {
		set_<int64_t>(value, type, pos);
	}
	
	inline void Statement::set(const std::uint64_t &value, IType &type,uint32_t pos) {
		double d=value;
		set(d, type, pos);
	}

	inline void Statement::set(const Number &value, IType &type,uint32_t pos) {
		double d = value.asDouble();
		set(d, type, pos);
	}

	inline void Statement::set(const struct tm &value, IType &itype,uint32_t pos) {
		auto type = static_cast<Type<tm>*>(&itype);
		set(type->asString(value), itype, pos);
	};

	inline void Statement::set(const std::string &value, IType &itype,uint32_t pos) {
		auto type = static_cast<Type<std::string>*>(&itype);
		type->fromString(value);
		set(type->getBuff(), itype, pos);
	};
	inline void Statement::commit() { conn.commit(); }

	/****************************************** Connection ***************************/
	inline IMetaData &Connection::getMetaData() { return *mdata.get(); }
	inline void Connection::execute(const std::string &sql) {
		createStatement(sql)->execute();
	}

	inline std::unique_ptr<IStatement> Connection::createStatement(const std::string &sql) {
		std::unique_ptr<IStatement> stmt = createStatement();
		stmt->query(sql);
		return stmt;
	}
}
