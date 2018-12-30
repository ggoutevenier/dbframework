#include <chrono>
#include <thread>
#include "database.h"
#include "db_loopback.h"
#include "xform.h"
#include <sstream>

//SELECT * FROM INFORMATION_SCHEMA.TABLES
namespace dk {
	namespace loopback {
		class Statement;
		class Connection : public dk::Connection {
			friend class Statement;
		protected:
			void open();

		public:
			std::map<size_t,std::string> cache;
			virtual ~Connection();
			Connection(	);
			void close();
			std::unique_ptr<IStatement> createStatement() override;
			void rollback() override;
			void commit() override;

			void execute(const std::string &sql) override;
			void enableExtensions();
			void use(const std::string &schema);
		};

		std::unique_ptr<IConnection> make_connection() {
			return std::make_unique<Connection>();
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
		protected:
			Statement & getStatement() { return (Statement&)stmt; }
			Connection & getConnection();
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
		};
		inline Connection &ResultSet::getConnection() { return getStatement().getConnection(); }
		inline ResultSet::ResultSet(IStatement &stmt) : dk::ResultSet(stmt) {
		}

		Connection::~Connection() {
			close();
		}

		Connection::Connection(
		)  {
			mdata = std::make_unique<dk::loopback::MetaData>(*this);

			open();
		}

		void Connection::use(const std::string &schema) {
		}
		MetaData::MetaData(IConnection &conn) : dk::MetaData(conn) {}

		bool ResultSet::next() {
			return true;
		}

		std::unique_ptr<IStatement> Connection::createStatement() {
			return std::make_unique<Statement>(*this);
		}

		std::unique_ptr<IResultSet> Statement::executeQuery() {
			return std::make_unique<ResultSet>(*this);
		}
		size_t Statement::executeUpdate() {
			return 0;
		}
		bool Statement::execute() {
			return false;
		}

		void Connection::open() {
		}

		void Connection::execute(const std::string &sql) {
		}

		Statement::Statement(IConnection &conn) :
				dk::Statement(conn) {
		}
		void Statement::query(const std::string &sql) {
			this->sql = sql;
		}

		void Connection::rollback() {
		}

		void Connection::commit() {
		}

		void Connection::close() {
		}

		void Statement::flush() {
			getConnection().cache.clear();
		}

		Statement::~Statement() {
			flush();
		}

		void Statement::set(const char *v, IColumn &column) {
			auto type = column.getType<char*>();
			auto &m = getConnection().cache;
			m[column.getColumnPosition()]=type.asString(v);
		}
		void Statement::set(const std::int64_t &v, IColumn &column) {
			auto &m = getConnection().cache;
			m[column.getColumnPosition()]=boost::lexical_cast<std::string>(v);
		}

		void Statement::set(const double &v, IColumn &column) {
			auto &m = getConnection().cache;
			m[column.getColumnPosition()]=boost::lexical_cast<std::string>(v);
		}

		std::string MetaData::bindVar(const std::string column) const { return "?";}
		const std::string MetaData::typeInt64(const IColumn &f) const { return "BIGINT"; }
		const std::string MetaData::typeDouble(const IColumn &f) const { return "DOUBLE"; }
		const std::string MetaData::typeNumber(const IColumn &column) const {
			auto type = column.getType<Number>();
			std::stringstream ss;

			ss << "decimal(" << type.getPrecision()<<","<<type.getScale()<<")";
			return ss.str();
		}
		const std::string MetaData::typeString(const IColumn &column) const {
			auto type = column.getType<char*>();;

			if(type.getSize()>0) {
				std::stringstream ss;

				ss << "varchar("<<type.getSize()<<")";
				return ss.str();
			}
			return "char(1)";
		}

		ResultSet::~ResultSet() {
		}
		void ResultSet::get(char *v, IColumn &column) {
			auto type = column.getType<char *>();
			auto &m = getConnection().cache;
			std::string str = m.at(column.getColumnPosition());
			type.fromString(str, v);
		}
		void ResultSet::get(double &v, IColumn &column) {
			auto &m = getConnection().cache;
			std::string str = m.at(column.getColumnPosition());
			v = boost::lexical_cast<double>(str);
		}
		void ResultSet::get(int64_t &v, IColumn &column) {
			auto &m = getConnection().cache;
			std::string str = m.at(column.getColumnPosition());
			v = boost::lexical_cast<int64_t>(str);
		}
	}
}
