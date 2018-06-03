#include "stdafx.h"
#include <chrono>
#include <thread>
#include "db_sqlite3.h"
#include <sqlite3.h>
#include <sstream>

namespace dk {
	namespace sqlite {
		MetaData::MetaData(IConnection &conn) : dk::MetaData(conn) {}
		bool step_(sqlite3_stmt *stmt) {
			assert(stmt);
			int rtn = sqlite3_step(stmt);
			int i = 0;
			while (rtn == SQLITE_LOCKED && i < 60) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				rtn = sqlite3_step(stmt);
				i++;
			}

			if (!(rtn == SQLITE_DONE || rtn == SQLITE_OK || rtn == SQLITE_ROW)) {
				std::string err = sqlite3_errstr(rtn);
				std::string sql = sqlite3_sql(stmt);
				throw std::runtime_error(err + "\nSQL: " + sql);
			}
			return rtn == SQLITE_ROW;
		}

		bool ResultSet::next() {
			return step_(getStatement().stmt);
		}

		void Statement::finalize() {
			if (stmt) {
				sqlite3_finalize(stmt);
				stmt = 0;
			}
		}

		std::unique_ptr<IStatement> Connection::createStatement() {
			return std::make_unique<Statement>(*this);
		}

		std::unique_ptr<IResultSet> Statement::executeQuery() {
			return std::make_unique<ResultSet>(*this);
		}
		size_t Statement::executeUpdate() {
			execute();
			return sqlite3_total_changes(getConnection().DB);
		}
		bool Statement::execute() {
			bool rtn = step_(stmt);
			sqlite3_reset(stmt);
			return rtn;
		}

		void Connection::open(const std::string &db) {
			this->db = db;
/*			int flags;
			if (readonly) {
				flags = SQLITE_OPEN_READONLY;
			}
			else
				flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;*/
			int rtn = sqlite3_open_v2(db.c_str(), &DB, flags, 0);
			int i = 0;
			while (rtn == SQLITE_LOCKED && i < 60) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				i++;
				rtn = sqlite3_open_v2(db.c_str(), &DB, flags, 0);
			}
			if (rtn != SQLITE_OK) {
				const char *err = sqlite3_errstr(rtn);
				throw std::runtime_error(err + std::string(" filename ") + db);
			}
			execute("PRAGMA main.synchronous=OFF;");
			execute("PRAGMA main.journal_mode=OFF;");
			execute("BEGIN TRANSACTION");

		}

		void Connection::execute(const std::string &sql) {
			int rc;
			rc = sqlite3_exec(DB, sql.c_str(), 0, 0, 0);
			if (rc != SQLITE_OK) {
				const char *err = sqlite3_errmsg(DB);
				throw std::runtime_error(err);
			}
			//todo check for errors
		}

		void Connection::enableExtensions() {
			sqlite3_enable_load_extension(this->DB, 1);
		}

		void Statement::query(const std::string &sql) {
			this->sql = sql;
			int rtn = sqlite3_prepare_v2(getConnection().DB, sql.c_str(), -1, &stmt, NULL);
			int i = 0;
			while (rtn == SQLITE_LOCKED && i < 60) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				rtn = sqlite3_prepare_v2(getConnection().DB, sql.c_str(), -1, &stmt, NULL);
				i++;
			}

			if (rtn != SQLITE_OK) {
				const char *err = sqlite3_errstr(rtn);
				std::stringstream ss;
				ss << "sql: " << sql << "\nerror: " << err;
				throw std::runtime_error(ss.str());
			}
		}

		void Connection::rollback() {
			execute("ROLLBACK");
		}

		void Connection::commit() {
			execute("COMMIT");

			if (sqlite3_get_autocommit(DB)) {
				execute("BEGIN TRANSACTION");
			}
		}

		void Connection::close() {
			if (DB) {
				commit();
				sqlite3_close(DB);
			}
		}

		void Statement::flush() {
		}
		Statement::~Statement() {
			sqlite3_finalize(stmt);
		}

		void Statement::reset() {
			if (stmt) {
				sqlite3_reset(stmt);
				sqlite3_clear_bindings(stmt);
			}
		}

		void Statement::bind(const std::string &v, IField &f) {
			int rc;
			
			if (v.empty())
				rc = sqlite3_bind_null(stmt, f.getColumn());
			else {
//				char *ptr=static_cast<char*>(f.getBuff(v.length()));
//				std::copy(v.begin(), v.end(), ptr);
//				rc = sqlite3_bind_text(stmt, f.getColumn(), ptr, (int)v.length(), 0);
				auto &buff = f.getBuff2(v.length());
				std::copy(v.begin(), v.end(), buff.begin());
				rc = sqlite3_bind_text(stmt, f.getColumn(), buff.data(), (int)v.length(), 0);
			}
			if (rc != SQLITE_OK) {
				const char *err = sqlite3_errmsg(getConnection().DB);
				throw std::runtime_error(err);
			}
		}

		void Statement::bind(const std::int64_t &v, IField &f) {
			int rc;
			rc = sqlite3_bind_int64(stmt, f.getColumn(), v);

			if (rc != SQLITE_OK) {
				const char *err = sqlite3_errmsg(getConnection().DB);
				throw std::runtime_error(err);
			}
		}


		void Statement::bind(const double &v, IField &f) {
			int rc;
			rc = sqlite3_bind_double(stmt, f.getColumn(), v);

			if (rc != SQLITE_OK) {
				const char *err = sqlite3_errmsg(getConnection().DB);
				throw std::runtime_error(err);
			}
		}

		const char *MetaData::dataType(const std::int64_t &, const IField &f) const { return "BIGINT"; }
		const char *MetaData::dataType(const double &, const IField &f) const { return "DOUBLE"; }
		const char *MetaData::dataType(const std::string &, const IField &f) const {return "Text";}

		ResultSet::~ResultSet() {
			getStatement().reset();
		}

		void ResultSet::getColumn(double &v, const IField &f) {
			v = sqlite3_column_double(getStatement().stmt, f.getColumn() - 1);
		}
		void ResultSet::getColumn(int64_t &v, const IField &f) {
			v = sqlite3_column_int64(getStatement().stmt, f.getColumn() - 1);
		}
		void ResultSet::getColumn(std::string &v, const IField &f) {
			const char *c= (const char *)sqlite3_column_text(getStatement().stmt, f.getColumn() - 1);
			if (!c)
				v.clear();
			else
				v = c;;
		}
	}
}