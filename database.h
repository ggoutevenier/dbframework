#pragma once
#include "interface.h"
#include <string>
#include <memory>
#include <boost/lexical_cast.hpp>

namespace dk {
	class MetaData : public IMetaData{
	protected:
		IConnection &conn;
		const std::string typeBool(const IField &f) const override;
		const std::string typeInt16(const IField &f) const override;
		const std::string typeInt32(const IField &f) const override;
		const std::string typeInt64(const IField &f) const override;
		const std::string typeFloat(const IField &f) const override;
		const std::string typeDouble(const IField &f) const override;
		const std::string typeString(const IField &f) const override;
		const std::string typeDate(const IField &f) const override;
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
		template<class T> void getString(T &t, const IField &f);
		template<class T> void getInt64(T &t, const IField &f);
		void getDouble(float &t, const IField &f);
	public:
		virtual ~ResultSet() {}
		ResultSet(IStatement &stmt) :stmt(stmt) {}
		virtual void get(std::string &v, const IField &f) = 0;
		virtual void get(float &v, const IField &f) override;
		virtual void get(double &v, const IField &f) override;
		virtual void get(char &v, const IField &f) override;
		virtual void get(bool &v, const IField &f) override;
		virtual void get(int16_t &v, const IField &f) override;
		virtual void get(int32_t &v, const IField &f) override;
		virtual void get(int64_t &v, const IField &f) override;
		virtual void get(uint16_t &v, const IField &f) override;
		virtual void get(uint32_t &v, const IField &f) override;
		virtual void get(uint64_t &v, const IField &f) override;
		virtual void get(struct tm &v, const IField &f) override;
	};

	class Statement : public IStatement {
	protected:
		IConnection &conn;
		std::string sql;
		template<class S,class T> void set_(const T &t, IField &f);
		template<class T> void setString(const T &t, IField &f);
//		template<class T> void setInt64(const T &t, IField &f);
//		void setDouble(const float &t, IField &f);
	public:
		Statement(IConnection &conn) :conn(conn) {}
		virtual ~Statement() {}
		void commit() override;
//		void set_array_size(unsigned int) override {};
		void set(const float &v, IField &f) override;
		void set(const bool &v, IField &f) override;
		void set(const std::int16_t &v, IField &f) override;
		void set(const std::int32_t &v, IField &f) override;
		void set(const std::int64_t &v, IField &f) override;
		void set(const std::uint16_t &v, IField &f) override;
		void set(const std::uint32_t &v, IField &f) override;
		void set(const std::uint64_t &v, IField &f) override;
		void set(const double &v, IField &f) override;
		void set(const char &v, IField &f) override;
		void set(const struct tm &v, IField &f) override;
		virtual void set(const std::string &v, IField &f) override =0;
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
	inline const std::string MetaData::typeBool(const IField &f)
		const {
		return IMetaData::type<int64_t>(f);
	}
	inline const std::string MetaData::typeInt16(const IField &f)
		const {
		return IMetaData::type<int64_t>(f);
	}

	inline const std::string MetaData::typeInt32(const IField &f)
		const {
		return IMetaData::type<int64_t>(f);
	}

	inline const std::string MetaData::typeInt64(const IField &f)
		const {
		return IMetaData::type<std::string>(f);
	}

	inline const std::string MetaData::typeFloat(const IField &f)
		const {
		return IMetaData::type<double>(f);
	}

	inline const std::string MetaData::typeDouble(const IField &f)
		const {
		return IMetaData::type<std::string>(f);
	}
	
	inline const std::string MetaData::typeDate(const IField &f)
		const {
		return IMetaData::type<std::string>(f);
	}

	inline const std::string MetaData::typeString(const IField &f)
		const {
		return IMetaData::type<std::string>(f);
	}

	inline std::string MetaData::selectSQL(const IRecord &record) const {
		std::stringstream ss;
		ss << "select ";
		bool first = true;
		for (const std::unique_ptr<IField> &field : record.getFields()) {
			if (!first) ss << ",";
			else first = false;
			ss << (field->is_selectable() ? "" : "null as ");
			ss << field->getName();
		}
		ss << " from " << record.getName();
		return ss.str();
	}

	inline std::string MetaData::insertSQL(const IRecord &record) const {
		std::stringstream ss;
		ss << "insert into " << record.getName() << "(";
		bool first = true;
		for (const std::unique_ptr<IField> &field : record.getFields()) {
			if (!first) ss << ",";
			else first = false;
			ss << field->getName();
		}
		ss << " ) values ( ";
		first = true;
		for (const std::unique_ptr<IField> &field : record.getFields()) {
			if (!first) ss << ",";
			else first = false;
			ss << ":" << field->getName();
		}
		ss << ")";

		return ss.str();
	}

	inline std::string MetaData::createSQL(const IRecord &record) const { // todo primary key
		std::stringstream ss;
		ss << "create table " << record.getName() << "(";
		bool first = true;
		for (const std::unique_ptr<IField> &field : record.getFields()) {
			if (!first) ss << ",";
			else first = false;
			ss << field->getName() << " " << field->type(*this);
		}
		ss << ")";
		return ss.str();
	}

	/****************************************** Resultset ***************************/
	template<class T>
	inline void ResultSet::getString(T &t, const IField &f) {
		std::string str;
		get(str, f);
		if (!str.empty())
			t = boost::lexical_cast<T>(str);
	}

	template<class T>
	inline void ResultSet::getInt64(T &t, const IField &f) {
		int64_t i;
		get(i, f);
		t = boost::lexical_cast<T>(i);
	}
	
	inline void ResultSet::getDouble(float &t, const IField &f) {
		double d;
		get(d, f);
		t = boost::lexical_cast<float>(d);
	}
	inline void ResultSet::get(float &v, const IField &f) { getDouble(v, f); }
	inline void ResultSet::get(double &v, const IField &f) { v = 0; getString(v, f); }
	inline void ResultSet::get(char &v, const IField &f) { v = 0; getString(v, f); }
	inline void ResultSet::get(bool &v, const IField &f) { getInt64(v, f); }
	inline void ResultSet::get(int16_t &v, const IField &f) { getInt64(v, f); }
	inline void ResultSet::get(int32_t &v, const IField &f) { getInt64(v, f); }
	inline void ResultSet::get(int64_t &v, const IField &f) { v = 0; getString(v, f); }
	inline void ResultSet::get(uint16_t &v, const IField &f) { getInt64(v, f); };
	inline void ResultSet::get(uint32_t &v, const IField &f) { getInt64(v, f); }
	inline void ResultSet::get(uint64_t &v, const IField &f) { v = 0; getString(v, f); }

	static void fromString(const std::string str, tm &data) {
		data.tm_hour = data.tm_min = data.tm_sec = 0;
		sscanf(str.c_str(), "%4d-%02d-%02d %02d:%02d:%02d",
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

	inline void ResultSet::get(struct tm &v, const IField &f) {
		std::string str;
		get(str, f);
		fromString(str, v);
	}

	/****************************************** Statement ***************************/
	template<class S,class T>
	inline void Statement::set_(const T &t, IField &f) {
		set( boost::lexical_cast<S>(t),f);
	}

	template<class T>
	inline void Statement::setString(const T &t, IField &f) {
		std::string str = boost::lexical_cast<std::string>(t);
		set(str, f);
//		set(boost::lexical_cast<std::string>(t), f);
	}

/*	template<class T>
	inline void Statement::setInt64(const T &t, IField &f) {
		set(boost::lexical_cast<int64_t>(t), f);
	}
	
	inline void Statement::setDouble(const float &t, IField &f) {
		set(boost::lexical_cast<double>(t), f);
	}*/
/*	inline void Statement::set(const float &v, IField &f) { setDouble(v, f); }
	inline void Statement::set(const bool &v, IField &f) { setInt64(v, f); }
	inline void Statement::set(const std::int16_t &v, IField &f) { setInt64(v, f); }
	inline void Statement::set(const std::int32_t &v, IField &f) { setInt64(v, f); }
	inline void Statement::set(const std::int64_t &v, IField &f) { setString(v, f); }
	inline void Statement::set(const std::uint16_t &v, IField &f) { setInt64(v, f); };
	inline void Statement::set(const std::uint32_t &v, IField &f) { setInt64(v, f); }
	inline void Statement::set(const std::uint64_t &v, IField &f) { setString(v, f); }
	inline void Statement::set(const double &v, IField &f) { setString(v, f); };
	inline void Statement::set(const char &v, IField &f) { setString(v, f); };
*/
	inline void Statement::set(const float &v, IField &f) { set_<double>(v, f); }
	inline void Statement::set(const bool &v, IField &f) { set_<int16_t>(v, f); }
	inline void Statement::set(const std::int16_t &v, IField &f) { set_<int32_t>(v, f); }
	inline void Statement::set(const std::int32_t &v, IField &f) { set_<int64_t>(v, f); }
	inline void Statement::set(const std::int64_t &v, IField &f) { setString(v, f); }
	inline void Statement::set(const std::uint16_t &v, IField &f) { set_<int32_t>(v, f); };
	inline void Statement::set(const std::uint32_t &v, IField &f) { set_<int64_t>(v, f); }
	inline void Statement::set(const std::uint64_t &v, IField &f) { setString(v, f); }
	inline void Statement::set(const double &v, IField &f) { setString(v, f); };
	inline void Statement::set(const char &v, IField &f) { setString(v, f); };

	static std::string toString(const tm &data) {
		char dt[60];
		sprintf(
			dt,
			"%4d-%02d-%02d %02d:%02d:%02d",
			data.tm_year + 1900,
			data.tm_mon + 1,
			data.tm_mday,
			data.tm_hour,
			data.tm_min,
			data.tm_sec
		);
		return dt;
	}
	inline void Statement::set(const struct tm &v, IField &f) {
		std::string str = toString(v);
		setString(str, f);
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
