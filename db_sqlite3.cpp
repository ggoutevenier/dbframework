#include <chrono>
#include <thread>
#include "database.h"
#include "db_sqlite3.h"
#include <sqlite3.h>
#include <sstream>

namespace dk {
	namespace sqlite {
		class Statement;
		class Connection : public dk::Connection {
			friend class Statement;
		protected:
			sqlite3 *DB;
			std::string db;
			int flags;
			void open(const std::string &db);
		public:
			virtual ~Connection();
			Connection(const std::string &db, int flags = 0);
			void close();
			std::unique_ptr<IStatement> createStatement() override;
			void rollback() override;
			void commit() override;

			void execute(const std::string &sql) override;
			void enableExtensions();
		};

		std::unique_ptr<IConnection> make_connection(const std::string &db, int flags) {
			return std::make_unique<Connection>(db,flags);
		}

		class MetaData : public dk::MetaData {
		private:
			Connection & getConnection() { return (Connection&)conn; }
		protected:
			const std::string typeInt64(const IField  &f) const override;
			const std::string typeDouble(const IField   &f) const override;
			const std::string typeString(const IField  &f) const override;
		public:
			virtual ~MetaData() {}
			MetaData(IConnection &conn);
		};
		class Statement;
		class ResultSet : public dk::ResultSet {
		protected:
			Statement & getStatement() { return (Statement&)stmt; }
		public:
			virtual ~ResultSet();
			ResultSet(IStatement &stmt);
			void get(std::string &v, const IField &f) override;
			void get(double &v, const IField &f) override;
			void get(int64_t &v, const IField &f) override;
			bool next() override;
		};

		class Statement : public dk::Statement {
			friend class ResultSet;
		private:
			sqlite3_stmt * stmt;
			Connection &getConnection() { return (Connection&)conn; }
		public:
			Statement(IConnection &conn) :
				dk::Statement(conn),stmt(0) {}
			size_t executeUpdate() override;
			~Statement();
			void reset();
			void finalize();
			void flush() override;
			void query(const std::string &sql);
			std::unique_ptr<IResultSet> executeQuery() override;
			bool execute() override;
			virtual void set(const std::int64_t &v, IField &f) override;
			virtual void set(const double &v, IField &f) override;
			virtual void set(const std::string &v, IField &f) override;
		};

		inline ResultSet::ResultSet(IStatement &stmt) :
				dk::ResultSet(stmt) {}

		Connection::~Connection() {
			close();
		}
		
		Connection::Connection(const std::string &db, int flags) : flags(flags) {
			mdata = std::make_unique<dk::sqlite::MetaData>(*this);
			open(db);
			enableExtensions();
		}

		MetaData::MetaData(IConnection &conn) :
				dk::MetaData(conn) {}
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

		void Statement::set(const std::string &v, IField &f) {
			int rc;
			
			if (v.empty())
				rc = sqlite3_bind_null(stmt, f.getColumn());
			else {
				auto &buff = f.getScratch(v.length());
				std::copy(v.begin(), v.end(), buff.begin());
				rc = sqlite3_bind_text(stmt, f.getColumn(), buff.data(), (int)v.length(), 0);
			}
			if (rc != SQLITE_OK) {
				const char *err = sqlite3_errmsg(getConnection().DB);
				throw std::runtime_error(err);
			}
		}

		void Statement::set(const std::int64_t &v, IField &f) {
			int rc;
			rc = sqlite3_bind_int64(stmt, f.getColumn(), v);

			if (rc != SQLITE_OK) {
				const char *err = sqlite3_errmsg(getConnection().DB);
				throw std::runtime_error(err);
			}
		}


		void Statement::set(const double &v, IField &f) {
			int rc;
			rc = sqlite3_bind_double(stmt, f.getColumn(), v);

			if (rc != SQLITE_OK) {
				const char *err = sqlite3_errmsg(getConnection().DB);
				throw std::runtime_error(err);
			}
		}

		const std::string MetaData::typeInt64(const IField &f) const { return "BIGINT"; }
		const std::string MetaData::typeDouble(const IField &f) const { return "DOUBLE"; }
		const std::string MetaData::typeString(const IField &f) const {return "Text";}

		ResultSet::~ResultSet() {
			getStatement().reset();
		}

		void ResultSet::get(double &v, const IField &f) {
			v = sqlite3_column_double(getStatement().stmt, f.getColumn() - 1);
		}
		void ResultSet::get(int64_t &v, const IField &f) {
			v = sqlite3_column_int64(getStatement().stmt, f.getColumn() - 1);
		}
		void ResultSet::get(std::string &v, const IField &f) {
			const char *c= (const char *)sqlite3_column_text(getStatement().stmt, f.getColumn() - 1);
			if (!c)
				v.clear();
			else
				v = c;;
		}
	}
}
