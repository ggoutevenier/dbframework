#pragma once
#include "interface.h"
#include <string>
#include <memory>
#include <boost/lexical_cast.hpp>

namespace dk {
	class MetaData : public IMetaData{
	protected:
		IConnection &conn;
		const std::string typeBool(const IColumn &f) const override;
		const std::string typeInt16(const IColumn &f) const override;
		const std::string typeInt32(const IColumn &f) const override;
		const std::string typeInt64(const IColumn &f) const override;
		const std::string typeFloat(const IColumn &f) const override;
		const std::string typeDouble(const IColumn &f) const override;
		const std::string typeString(const IColumn &f) const override;
		const std::string typeDate(const IColumn &f) const override;
		const std::string typeNumber(const IColumn &f) const override;
//		virtual const char *dataType(const std::string &, const IField  &f) const =0;
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
		template<class T> void getString(T &t, IColumn &f);
		template<class T> void getInt64(T &t, IColumn &f);
		void getDouble(float &t, IColumn &f);
	public:
		virtual ~ResultSet() override {}
		ResultSet(IStatement &stmt) :stmt(stmt) {}
		void get(float &v, IColumn &f) override;
		void get(double &v, IColumn &f) override=0;
//		void get(char &v, IColumn &f) override;
		void get(char *v, IColumn &f) override=0;
		void get(bool &v, IColumn &f) override;
		void get(int16_t &v, IColumn &f) override;
		void get(int32_t &v, IColumn &f) override;
		void get(int64_t &v, IColumn &f) override;
		void get(uint16_t &v, IColumn &f) override;
		void get(uint32_t &v, IColumn &f) override;
		void get(uint64_t &v, IColumn &f) override;
		void get(struct tm &v, IColumn &f) override;
		void get(Number &v, IColumn &f) override;
//		void get(std::string &v, IColumn &f) override;
		void get(IColumn &f) override;
	};

	class Statement : public IStatement {
	protected:
		IConnection &conn;
		std::string sql;
		template<class S, class T> void set_(const T &t, IColumn &f);
//		template<class T> void setString(const T &t, IColumn &f);
//		template<class T> void setInt64(const T &t, IField &f);
//		void setDouble(const float &t, IField &f);
	public:
		Statement(IConnection &conn) :conn(conn) {}
		virtual ~Statement() {}
		void commit() override;
//		void set_array_size(unsigned int) override {};
		void set(const float &v, IColumn &f) override;
		void set(const bool &v, IColumn &f) override;
		void set(const std::int16_t &v, IColumn &f) override;
		void set(const std::int32_t &v, IColumn &f) override;
		void set(const std::int64_t &v, IColumn &f) override;
		void set(const std::uint16_t &v, IColumn &f) override;
		void set(const std::uint32_t &v, IColumn &f) override;
		void set(const std::uint64_t &v, IColumn &f) override;
		void set(const double &v, IColumn &f) override=0;
//		void set(const char &v, IColumn &f) override;
		void set(const char *v, IColumn &f) override=0;
		void set(const struct tm &v, IColumn &f) override;
		void set(const Number &v, IColumn &f) override;
//		void set(const std::string &v, IColumn &f) override;
		void set(IColumn &f) override;
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
	inline const std::string MetaData::typeBool(const IColumn &column)
		const {
		return IMetaData::type<int64_t>(column);
	}
	inline const std::string MetaData::typeInt16(const IColumn &column)
		const {
		return IMetaData::type<int64_t>(column);
	}

	inline const std::string MetaData::typeInt32(const IColumn &column)
		const {
		return IMetaData::type<int64_t>(column);
	}

	inline const std::string MetaData::typeInt64(const IColumn &column)
		const {
		return IMetaData::type<std::string>(column);
	}

	inline const std::string MetaData::typeFloat(const IColumn &column)
		const {
		return IMetaData::type<double>(column);
	}

	inline const std::string MetaData::typeDouble(const IColumn &column)
		const {
		return IMetaData::type<std::string>(column);
	}
	
	inline const std::string MetaData::typeDate(const IColumn &column)
		const {
		return IMetaData::type<std::string>(column);
	}

	inline const std::string MetaData::typeString(const IColumn &column)
		const {
		return IMetaData::type<std::string>(column);
	}

	inline const std::string MetaData::typeNumber(const IColumn &column)
		const {
		return IMetaData::type<double>(column);
	}

	inline std::string MetaData::selectSQL(const IRecord &record) const {
		std::stringstream ss;
		ss << "select ";
		bool first = true;
		for (const std::unique_ptr<IColumn> &column : record.getColumns()) {
			if (!first) ss << ",";
			else first = false;
			ss << (column->is_selectable() ? "" : "null as ");
			ss << column->getName();
		}
		ss << " from " << record.getName();
		return ss.str();
	}

	inline std::string MetaData::insertSQL(const IRecord &record) const {
		std::stringstream ss;
		ss << "insert into " << record.getName() << "(";
		bool first = true;
		for (const std::unique_ptr<IColumn> &column : record.getColumns()) {
			if (!first) ss << ",";
			else first = false;
			ss << column->getName();
		}
		ss << " ) values ( ";
		first = true;
		for (const std::unique_ptr<IColumn> &column : record.getColumns()) {
			(void)column; // suppress warning about column not used
			if (!first) ss << ",";
			else first = false;
			ss << bindVar(column->getName());
		}
		ss << ")";

		return ss.str();
	}

	inline std::string MetaData::createSQL(const IRecord &record) const { // todo primary key
		std::stringstream ss;
		ss << "create table " << record.getName() << "(";
		bool first = true;
		for (const std::unique_ptr<IColumn> &column : record.getColumns()) {
			if (!first) ss << ",";
			else first = false;
			std::string name = column->getName();
			std::string type = column->type(*this);
			ss << name << " " << type ;
		}
		ss << ")";
		return ss.str();
	}

	/****************************************** Resultset ***************************/
	inline void ResultSet::get(IColumn &column) {
		get(column.getBuff().data(),column);
	}

	template<class T>
	inline void ResultSet::getInt64(T &t, IColumn &column) {
		int64_t i;
		get(i, column);
		t = boost::lexical_cast<T>(i);
	}
	
	inline void ResultSet::getDouble(float &t, IColumn &column) {
		double d;
		get(d, column);
		t = boost::lexical_cast<float>(d);
	}
	inline void ResultSet::get(float &value, IColumn &column) {
		getDouble(value, column);
	}

	inline void ResultSet::get(bool &value, IColumn &column) {
		getInt64(value, column);
	}
	inline void ResultSet::get(int16_t &value, IColumn &column) {
		getInt64(value, column);
	}
	inline void ResultSet::get(int32_t &value, IColumn &column) {
		getInt64(value, column);
	}
	inline void ResultSet::get(int64_t &value, IColumn &column) {
		double d;
		get(d, column);
		value = d;
	}
	inline void ResultSet::get(uint16_t &value, IColumn &column) {
		getInt64(value, column);
	}
	inline void ResultSet::get(uint32_t &value, IColumn &column) {
		getInt64(value, column);
	}
	inline void ResultSet::get(uint64_t &value, IColumn &column) {
		double d;
		get(d, column);
		value = d;
	}
	inline void ResultSet::get(Number &value, IColumn &column) {
		double d;
		get(d, column);
		value.fromDouble(d);
	}

	static void fromString(char *str, tm &data, std::string format) {
		data.tm_hour = data.tm_min = data.tm_sec = 0;
		sscanf(str, format.c_str(),
			&data.tm_year,
			&data.tm_mon,
			&data.tm_mday,
			&data.tm_hour,
			&data.tm_min,
			&data.tm_sec);
		data.tm_year -= 1900;
		data.tm_mon -= 1;
		mktime(&data);
	}

	inline void ResultSet::get(struct tm &v, IColumn &f) {
		get(f);
		fromString(f.getBuff().data(), v, f.getDateFormat());
	}

	/****************************************** Statement ***************************/
	inline void Statement::set(IColumn &column) {
		set(column.getBuff().data(),column);
	}

	template<class S,class T>
	inline void Statement::set_(const T &t, IColumn &column) {
		set( boost::lexical_cast<S>(t),column);
	}

	inline void Statement::set(const float &value, IColumn &column) {
		set_<double>(value, column);
	}
	inline void Statement::set(const bool &value, IColumn &column) {
		set_<int16_t>(value, column);
	}
	inline void Statement::set(const std::int16_t &value, IColumn &column) {
		set_<int32_t>(value, column);
	}
	inline void Statement::set(const std::int32_t &value, IColumn &column) {
		set_<int64_t>(value, column);
	}
	inline void Statement::set(const std::int64_t &value, IColumn &column) {
		double d=value;
		set(d, column);
	}
	inline void Statement::set(const std::uint16_t &value, IColumn &column) {
		set_<int32_t>(value, column);
	};
	inline void Statement::set(const std::uint32_t &value, IColumn &column) {
		set_<int64_t>(value, column);
	}
	inline void Statement::set(const std::uint64_t &value, IColumn &column) {
		double d=value;
		set(d, column);
	}

	inline void Statement::set(const Number &value, IColumn &column) {
		double d = value.asDouble();
		set(d, column);
	}

	static std::string toString(const tm &data,std::string format) {
		char dt[60];
		sprintf(
			dt,
			format.c_str(),
			data.tm_year + 1900,
			data.tm_mon + 1,
			data.tm_mday,
			data.tm_hour,
			data.tm_min,
			data.tm_sec
		);
		return dt;
	}

	inline void Statement::set(const struct tm &v, IColumn &f) {
		f.toBuff(toString(v,f.getDateFormat()));
		set(f);
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
