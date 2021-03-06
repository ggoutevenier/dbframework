#include <chrono>
#include <thread>
#include "database.h"
#include "db_mysql.h"
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <sstream>
//SELECT * FROM INFORMATION_SCHEMA.TABLES
namespace dk {
	namespace mysql {
		class Statement;
		class Connection : public dk::Connection {
			friend class Statement;
		protected:
			std::unique_ptr<sql::Connection> conn;
			std::string schema;
			std::string db;
			std::string usr;
			std::string pwd;
			void open();
		public:
			virtual ~Connection();
			Connection(
				const std::string &schema,
				const std::string &db,
				const std::string &usr,
				const std::string &pwd
			);
			void close();
			std::unique_ptr<IStatement> createStatement() override;
			void rollback() override;
			void commit() override;

			void execute(const std::string &sql) override;
			void enableExtensions();
			void use(const std::string &schema);
		};

		std::unique_ptr<IConnection> make_connection(
				const std::string &schema,
				const std::string &db,
				const std::string &usr,
				const std::string &pwd) {
			return std::make_unique<Connection>(schema,db,usr,pwd);
		}

		class MetaData : public dk::MetaData {
		private:
			Connection & getConnection() { return (Connection&)conn; }
		protected:
			const std::string typeInt64(const IColumn &f) const override;
			const std::string typeDouble(const IColumn &f) const override;
			const std::string typeString(const IColumn &f) const override;
			const std::string typeNumber(const IColumn &f) const override;
			std::string bindVar(const std::string name) const override;
		public:
			virtual ~MetaData() {}
			MetaData(IConnection &conn);
//			std::string insertSQL(const IRecord &record) const override;
		};
		class Statement;
		class ResultSet : public dk::ResultSet {
			std::unique_ptr<sql::ResultSet> rset;
		protected:
			Statement & getStatement() { return (Statement&)stmt; }
		public:
			virtual ~ResultSet();
			ResultSet(IStatement &stmt);
//			virtual void get(std::string &v, const IColumn &f) override;
			virtual void get(char *v, IColumn &f) override;
			virtual void get(double &v, IColumn &f) override;
			virtual void get(int64_t &v, IColumn &f) override;
//			virtual void get(IColumn &f) override;
			bool next() override;
		};

		class Statement : public dk::Statement {
			friend class ResultSet;
		private:
			std::unique_ptr<sql::PreparedStatement> stmt;
			Connection &getConnection() { return (Connection&)conn; }
		public:
			Statement(IConnection &conn);
			size_t executeUpdate() override;
			~Statement();
			void flush() override;
			void query(const std::string &sql);
			std::unique_ptr<IResultSet> executeQuery() override;
			bool execute() override;
			virtual void set(const char *v, IColumn &f) override;
			virtual void set(const std::int64_t &v, IColumn &f) override;
			virtual void set(const double &v, IColumn &f) override;
//			virtual void set(const std::string &v, IColumn &f) override;
//			virtual void set(const IColumn &f) override;
		};

		inline ResultSet::ResultSet(IStatement &stmt) : dk::ResultSet(stmt) {
			rset = std::unique_ptr<sql::ResultSet>(
					getStatement().stmt->executeQuery());
		}

		Connection::~Connection() {
			close();
		}

		Connection::Connection(
			const std::string &schema,
			const std::string &db,
			const std::string &usr,
			const std::string &pwd
		) : schema(schema),db(db),usr(usr),pwd(pwd) {
			mdata = std::make_unique<dk::mysql::MetaData>(*this);

			open();
			use(schema);
		}

		void Connection::use(const std::string &schema) {
			execute("use "+schema);
		}
		MetaData::MetaData(IConnection &conn) : dk::MetaData(conn) {}

		bool ResultSet::next() {
			return rset->next();
		}

		std::unique_ptr<IStatement> Connection::createStatement() {
			return std::make_unique<Statement>(*this);
		}

		std::unique_ptr<IResultSet> Statement::executeQuery() {
			return std::make_unique<ResultSet>(*this);
		}
		size_t Statement::executeUpdate() {
			return stmt->executeUpdate();
		}
		bool Statement::execute() {
			return stmt->execute();
		}

		void Connection::open() {
			conn = std::unique_ptr<sql::Connection>(
					get_driver_instance()->connect(db, usr, pwd));
			conn->setSchema(schema);
		}

		void Connection::execute(const std::string &sql) {
			std::unique_ptr<sql::Statement>(
					conn->createStatement())->execute(sql);
		}

		Statement::Statement(IConnection &conn) :
				dk::Statement(conn) {
		}
		void Statement::query(const std::string &sql) {
			this->sql = sql;
			stmt = std::unique_ptr<sql::PreparedStatement>(
						getConnection().conn->prepareStatement(sql));
		}

		void Connection::rollback() {
			conn->rollback();
		}

		void Connection::commit() {
			conn->commit();
		}

		void Connection::close() {
		}

		void Statement::flush() {
			getConnection().commit();
		}

		Statement::~Statement() {
			flush();
		}

//		void Statement::set(const std::string &v, IColumn &f) {
//			stmt->setString(f.getColumn(), v);
//		}

		void Statement::set(const char *v, IColumn &f) {
			std::string str(v,&v[std::min(strlen(v),f.getSize()-1)]);
			stmt->setString(f.getColumn(), str);
		}
		void Statement::set(const std::int64_t &v, IColumn &f) {
			stmt->setInt(f.getColumn(), v);
		}

		void Statement::set(const double &v, IColumn &f) {
			stmt->setDouble(f.getColumn(), v);
		}

//		void Statement::set(const IColumn &f) {
//			std::string str;
//			std::copy(f.getBuff().begin(),f.getBuff().end(),str.begin());
//			stmt->setString(f.getColumn(), str);
//		}
		std::string MetaData::bindVar(const std::string column) const { return "?";}
		const std::string MetaData::typeInt64(const IColumn &f) const { return "BIGINT"; }
		const std::string MetaData::typeDouble(const IColumn &f) const { return "DOUBLE"; }
		const std::string MetaData::typeNumber(const IColumn &f) const {
			std::stringstream ss;

			ss << "decimal(" << f.getPrecision()<<","<<f.getScale()<<")";
			return ss.str();
		}
		const std::string MetaData::typeString(const IColumn &f) const {
			if(f.getSize()>0) {
				std::stringstream ss;

				ss << "varchar("<<f.getSize()<<")";
				return ss.str();
			}
			return "char(1)";
		}

/*		std::string MetaData::insertSQL(const IRecord &record) const {
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
					if (!first) ss << ",";
					else first = false;
					ss << "?";
				}
				ss << ")";

				return ss.str();
			}*/
		ResultSet::~ResultSet() {
		}
		void ResultSet::get(char *v, IColumn &f) {
			std::string str = rset->getString(f.getColumn());
			for(size_t i=0;i<std::min(str.length(),f.getSize());i++)
				v[i]=str.at(i);
		}
		void ResultSet::get(double &v, IColumn &f) {
			v = rset->getDouble(f.getColumn());
		}
		void ResultSet::get(int64_t &v, IColumn &f) {
			v = rset->getInt64(f.getColumn());
		}
//		void ResultSet::get(std::string &v, const IColumn &f) {
//			v = rset->getString(f.getColumn());
//		}

//		void ResultSet::get(IColumn &f) {
//			std::string str = rset->getString(f.getColumn());
//			assert(f.getBuff().size()>=str.length());
//			std::copy(str.begin(),str.end(),f.getBuff().begin());
//		}
	}
}
