#pragma once
#include "database.h"

//#pragma comment(lib,"sqlite3")
struct sqlite3;
struct sqlite3_stmt;
namespace dk {
	namespace sqlite {
		class Connection;
		class MetaData : public dk::MetaData {
		private:
			Connection &getConnection() { return (Connection&)conn; }
		protected:
			const char *dataType(const std::int64_t &, const IField  &f) const override;
			const char *dataType(const double &, const IField   &f) const override;
			const char *dataType(const std::string &, const IField  &f) const override;
		public:
			virtual ~MetaData() {}
			MetaData(IConnection &conn);
		};
		class Statement;
		class ResultSet : public dk::ResultSet {
		protected:
			Statement &getStatement() { return (Statement&)stmt; }
		public:
			virtual ~ResultSet();
			ResultSet(IStatement &stmt);
			void getColumn(std::string &v, const IField &f) override;
			void getColumn(double &v, const IField &f) override;
			void getColumn(int64_t &v, const IField &f) override;
			bool next() override;
		};

		class Statement : public dk::Statement {
			friend class ResultSet;
		private:
			sqlite3_stmt *stmt;
			Connection &getConnection() { return (Connection&)conn; }
		public:
			Statement(IConnection &conn) : dk::Statement(conn) {}
			size_t executeUpdate() override;
			~Statement();
			void reset();
			void finalize();
			void flush() override;
			void query(const std::string &sql);
			std::unique_ptr<IResultSet> executeQuery() override;
			bool execute() override;
			virtual void bind(const std::int64_t &v, IField &f) override;
			virtual void bind(const double &v, IField &f) override;
			virtual void bind(const std::string &v, IField &f) override;
		};

		inline ResultSet::ResultSet(IStatement &stmt) : dk::ResultSet(stmt) {}

		class Connection : public dk::Connection {
			friend class Statement;
		protected:
			sqlite3 *DB;
			std::string db;
			int flags;
			void open(const std::string &db);
		public:
			virtual ~Connection() {
				close();
			}
			Connection(const std::string &db, int flags=0) : flags(flags) {
				mdata = std::make_unique<dk::sqlite::MetaData>(*this);
				open(db);
			}

			void close();
			std::unique_ptr<IStatement> createStatement() override;
			void rollback() override;
			void commit() override;

			void execute(const std::string &sql) override;
			void enableExtensions();
		};
	}
}