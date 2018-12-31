#include <chrono>
#include <thread>
#include "database.h"
#include "db_loopback.h"
#include "xform.h"
#include <sstream>
#include <boost/any.hpp>
#include <deque>

namespace dk {
	namespace mem {
		class DynamicRecord {
			std::map<uint32_t,boost::any> cache;
		public:
			template<class T>
			void put(const T &t,uint32_t pos) {
				cache[pos]=t;
			}
			template<class T>
			const T get(uint32_t pos) {
				return boost::any_cast<T>(cache.at(pos));
			}
		};
		class DynamicTable {
			std::deque<DynamicRecord> table;
			DynamicRecord in,out;
		public:
			template<class T>
			void putValue(const T &value,uint32_t pos) {
				in.put(value,pos);
			}
			template<class T>
			const T getValue(uint32_t pos) {
				return out.get<T>(pos);
			}
			void writeRecord() {
				table.push_back(std::move(in));				
			}
			bool readrecord() {
				if(table.empty()) return false;
				out = std::move(table.front());
				table.pop_front();
				return true;
			}
		};
		class Statement;
		class Connection : public dk::Connection {
			friend class Statement;
		protected:
			void open();
		public:
			DynamicTable table;
			virtual ~Connection() {close();};
			Connection(	);
			void close() {};
			std::unique_ptr<IStatement> createStatement() override;
			void rollback() override {};
			void commit() override {};
		};

		std::unique_ptr<IConnection> make_connection() {
			return std::make_unique<Connection>();
		}

		class MetaData : public dk::MetaData {
		private:
			Connection & getConnection() { return (Connection&)conn; }
		protected:
			const std::string typeInt64(const IType &type) const override;
			const std::string typeDouble(const IType &type) const override;
			const std::string typeString(const IType &type) const override;
			const std::string typeNumber(const IType &type) const override;
			std::string bindVar(const std::string name) const override;
		public:
			virtual ~MetaData() {}
			MetaData(IConnection &conn);
		};
		class Statement;
		class ResultSet : public dk::ResultSet {
		protected:
			Statement & getStatement() { return (Statement&)stmt; }
			Connection & getConnection();
		public:
			virtual ~ResultSet();
			ResultSet(IStatement &stmt);
			virtual void get(char *v, IType &type,uint32_t pos) override;
			virtual void get(double &v, IType &type,uint32_t pos) override;
			virtual void get(int64_t &v, IType &type,uint32_t pos) override;
			bool next() override;
		};

		class Statement : public dk::Statement {
			friend class ResultSet;
		private:
			Connection &getConnection() { return (Connection&)conn; }
		public:
			Statement(IConnection &conn);
			size_t executeUpdate() override {
				getConnection().table.writeRecord();
				return 0;
			}
			~Statement();
			void flush() override {};
			void query(const std::string &sql);
			std::unique_ptr<IResultSet> executeQuery() override;
			bool execute() override;
			virtual void set(const char *v, IType &type,uint32_t pos) override;
			virtual void set(const std::int64_t &v, IType &type,uint32_t pos) override;
			virtual void set(const double &v, IType &type,uint32_t pos) override;
		};
		inline Connection &ResultSet::getConnection() { return getStatement().getConnection(); }
		inline ResultSet::ResultSet(IStatement &stmt) : dk::ResultSet(stmt) {
		}

		Connection::Connection(
		)  {
			mdata = std::make_unique<dk::mem::MetaData>(*this);
		}

		MetaData::MetaData(IConnection &conn) : dk::MetaData(conn) {}

		bool ResultSet::next() {
			return getConnection().table.readrecord();
		}

		std::unique_ptr<IStatement> Connection::createStatement() {
			return std::make_unique<Statement>(*this);
		}

		std::unique_ptr<IResultSet> Statement::executeQuery() {
			getConnection().table.readrecord();
			return std::make_unique<ResultSet>(*this);
		}

		bool Statement::execute() {
			getConnection().table.writeRecord();
			return true;
		}

		Statement::Statement(IConnection &conn) :
				dk::Statement(conn) {
		}
		void Statement::query(const std::string &sql) {
			this->sql = sql;
		}

		Statement::~Statement() {
			flush();
		}

		void Statement::set(const char *v, IType &itype,uint32_t pos) {
			auto type = static_cast<Type<char*>*>(&itype);
			auto &m = getConnection().table;
			m.putValue(type->asString(v), pos);
		}
		void Statement::set(const std::int64_t &v, IType &,uint32_t pos) {
			auto &m = getConnection().table;
			m.putValue(v, pos);
		}

		void Statement::set(const double &v, IType &,uint32_t pos) {
			auto &m = getConnection().table;
			m.putValue(v, pos);
		}

		std::string MetaData::bindVar(const std::string ) const { return "";}
		const std::string MetaData::typeInt64(const IType &) const { return ""; }
		const std::string MetaData::typeDouble(const IType &) const { return ""; }
		const std::string MetaData::typeNumber(const IType &) const {return "";}
		const std::string MetaData::typeString(const IType &) const {return "";}

		ResultSet::~ResultSet() {
		}
		void ResultSet::get(char *v, IType &itype, uint32_t pos) {
			auto type = static_cast<Type<char *>*>(&itype);
			auto &m = getConnection().table;
			std::string str = m.getValue<std::string>(pos);
			type->fromString(str, v);
		}
		void ResultSet::get(double &v, IType &itype, uint32_t pos) {
			auto &m = getConnection().table;
			v = m.getValue<double>(pos);
		}
		void ResultSet::get(int64_t &v, IType &itype, uint32_t pos) {
			auto &m = getConnection().table;
			v = m.getValue<int64_t>(pos);
		}
	}
}
