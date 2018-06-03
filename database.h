#pragma once
#include "interface.h"
//#include "record.h"
#include <string>
#include <memory>
#include <boost/lexical_cast.hpp>

namespace dk {
	class MetaData : public IMetaData{
	protected:
		IConnection &conn;
		const char *dataType(const bool &, const IField &f) const override;
		const char *dataType(const int16_t &, const IField &f) const override;
		const char *dataType(const int32_t &, const IField &f) const override;
		const char *dataType(const int64_t &, const IField &f) const override;
		const char *dataType(const uint16_t &, const IField &f) const override;
		const char *dataType(const uint32_t &, const IField &f) const override;
		const char *dataType(const uint64_t &, const IField &f) const override;
		const char *dataType(const float &, const IField &f) const override;
		const char *dataType(const double &, const IField &f) const override;
		const char *dataType(const struct tm &, const IField &f) const override;
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
		virtual void getColumn(std::string &v, const IField &f) = 0;
		void getColumn(float &v, const IField &f) override;
		void getColumn(double &v, const IField &f) override;
		void getColumn(char &v, const IField &f) override;
		void getColumn(bool &v, const IField &f) override;
		void getColumn(int16_t &v, const IField &f) override;
		void getColumn(int32_t &v, const IField &f) override;
		void getColumn(int64_t &v, const IField &f) override;
		void getColumn(uint16_t &v, const IField &f) override;
		void getColumn(uint32_t &v, const IField &f) override;
		void getColumn(uint64_t &v, const IField &f) override;
		void getColumn(struct tm &v, const IField &f) override;
	};

	class Statement : public IStatement {
	protected:
		IConnection &conn;
		std::string sql;

		template<class T> void bindString(const T &t, IField &f);
		template<class T> void bindInt64(const T &t, IField &f);
		void bindDouble(const float &t, IField &f);
	public:
		Statement(IConnection &conn) :conn(conn) {}
		virtual ~Statement() {}
		void commit() override;
//		void bind_array_size(unsigned int) override {};
		void bind(const float &v, IField &f) override;
		void bind(const bool &v, IField &f) override;
		void bind(const std::int16_t &v, IField &f) override;
		void bind(const std::int32_t &v, IField &f) override;
		void bind(const std::int64_t &v, IField &f) override;
		void bind(const std::uint16_t &v, IField &f) override;
		void bind(const std::uint32_t &v, IField &f) override;
		void bind(const std::uint64_t &v, IField &f) override;
		void bind(const double &v, IField &f) override;
		void bind(const char &v, IField &f) override;
		void bind(const struct tm &v, IField &f) override;
		virtual void bind(const std::string &v, IField &f) override =0;
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
	inline const char *MetaData::dataType(const bool &, const IField &f)
		const {
		return IMetaData::dataType<int64_t>(f);
	}
	inline const char *MetaData::dataType(const int16_t &, const IField &f)
		const {
		return IMetaData::dataType<int64_t>(f);
	}

	inline const char *MetaData::dataType(const int32_t &, const IField &f)
		const {
		return IMetaData::dataType<int64_t>(f);
	}

	inline const char *MetaData::dataType(const int64_t &, const IField &f)
		const {
		return IMetaData::dataType<std::string>(f);
	}

	inline const char *MetaData::dataType(const uint16_t &, const IField &f)
		const {
		return IMetaData::dataType<int64_t>(f);
	}
	
	inline const char *MetaData::dataType(const uint32_t &, const IField &f)
		const {
		return IMetaData::dataType<int64_t>(f);
	}
	
	inline const char *MetaData::dataType(const uint64_t &, const IField &f)
		const {
		return IMetaData::dataType<int64_t>(f);
	}

	inline const char *MetaData::dataType(const float &, const IField &f)
		const {
		return IMetaData::dataType<double>(f);
	}

	inline const char *MetaData::dataType(const double &, const IField &f)
		const {
		return IMetaData::dataType<std::string>(f);
	}
	
	inline const char *MetaData::dataType(const struct tm &, const IField &f)
		const {
		return IMetaData::dataType<std::string>(f);
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
			ss << field->getName() << " " << field->dataType(*this);
		}
		ss << ")";
		return ss.str();
	}

	/****************************************** Resultset ***************************/
	template<class T>
	inline void ResultSet::getString(T &t, const IField &f) {
		std::string str;
		getColumn(str, f);
		if (!str.empty())
			t = boost::lexical_cast<T>(str);
	}

	template<class T>
	inline void ResultSet::getInt64(T &t, const IField &f) {
		int64_t i;
		getColumn(i, f);
		t = boost::lexical_cast<T>(i);
	}
	
	inline void ResultSet::getDouble(float &t, const IField &f) {
		double d;
		getColumn(d, f);
		t = boost::lexical_cast<float>(d);
	}
	inline void ResultSet::getColumn(float &v, const IField &f) { getDouble(v, f); }
	inline void ResultSet::getColumn(double &v, const IField &f) { v = 0; getString(v, f); }
	inline void ResultSet::getColumn(char &v, const IField &f) { v = 0; getString(v, f); }
	inline void ResultSet::getColumn(bool &v, const IField &f) { getInt64(v, f); }
	inline void ResultSet::getColumn(int16_t &v, const IField &f) { getInt64(v, f); }
	inline void ResultSet::getColumn(int32_t &v, const IField &f) { getInt64(v, f); }
	inline void ResultSet::getColumn(int64_t &v, const IField &f) { v = 0; getString(v, f); }
	inline void ResultSet::getColumn(uint16_t &v, const IField &f) { getInt64(v, f); };
	inline void ResultSet::getColumn(uint32_t &v, const IField &f) { getInt64(v, f); }
	inline void ResultSet::getColumn(uint64_t &v, const IField &f) { v = 0; getString(v, f); }

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

	inline void ResultSet::getColumn(struct tm &v, const IField &f) {
		std::string str;
		getColumn(str, f);
		fromString(str, v);
	}

	/****************************************** Statement ***************************/

	template<class T>
	inline void Statement::bindString(const T &t, IField &f) {
		std::string str = boost::lexical_cast<std::string>(t);
		bind(str, f);
//		bind(boost::lexical_cast<std::string>(t), f);
	}

	template<class T>
	inline void Statement::bindInt64(const T &t, IField &f) {
		bind(boost::lexical_cast<int64_t>(t), f);
	}
	
	inline void Statement::bindDouble(const float &t, IField &f) {
		bind(boost::lexical_cast<double>(t), f);
	}
	inline void Statement::bind(const float &v, IField &f) { bindDouble(v, f); }
	inline void Statement::bind(const bool &v, IField &f) { bindInt64(v, f); }
	inline void Statement::bind(const std::int16_t &v, IField &f) { bindInt64(v, f); }
	inline void Statement::bind(const std::int32_t &v, IField &f) { bindInt64(v, f); }
	inline void Statement::bind(const std::int64_t &v, IField &f) { bindString(v, f); }
	inline void Statement::bind(const std::uint16_t &v, IField &f) { bindInt64(v, f); };
	inline void Statement::bind(const std::uint32_t &v, IField &f) { bindInt64(v, f); }
	inline void Statement::bind(const std::uint64_t &v, IField &f) { bindString(v, f); }
	inline void Statement::bind(const double &v, IField &f) { bindString(v, f); };
	inline void Statement::bind(const char &v, IField &f) { bindString(v, f); };
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
	inline void Statement::bind(const struct tm &v, IField &f) {
		std::string str = toString(v);
		bindString(str, f);
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
