#pragma once
#include "store.h"
#include <deque>

//#include <memory>
//#include <list>
//#include <map>
//#include <vector>

namespace dk {
	class Sinkable {
	public:
		virtual ~Sinkable() {}
	};
	template<class T>
	class ISink : public Sinkable {
	public:
		using DTL = T;
		virtual ~ISink() {}
		virtual void push_back(const T &t) = 0;
//		static std::unique_ptr<ISink> make(std::shared_ptr<IConnection> conn,Store &store);
	};
	
	template<class T>
	class Sink : public ISink<T> {
		std::shared_ptr<IConnection> conn;
		std::unique_ptr <IStatement> stmt;
		uint32_t counter;
		void create() {
			std::string sql = conn->getMetaData().createSQL(metadata<T>::record());
			conn->execute(sql);
		}
		void open() {
			std::string sql = conn->getMetaData().insertSQL(metadata<T>::record());
			stmt = conn->createStatement(sql);
			counter = 0;
		}
		void close() {
			stmt->flush();
			stmt->commit();
		}
	public:
		Sink(std::shared_ptr<IConnection> conn) :conn(conn) {
			create();
			open();
		}
		~Sink() { close(); }
		void push_back(const T &t) {
			if (!(counter % 1000)) { conn->commit(); }
			counter++;
			assert(stmt.get());
			metadata<T>::record().set(*stmt.get(), (void*)&t);
			stmt->executeUpdate();
		}
	};

	template<class D>
	class SinkContainer : public ISink<D > {
		Sink<typename D::value_type> sink;
		using T = typename D::value_type;
	public:
		SinkContainer(std::shared_ptr<IConnection> conn, Store &store) : sink(conn, store) {}
		void push_back(const D &d) {
			for (auto &v : d)
				sink.push_back(v);
		}
	};

	template<class T>
	class Sink<std::list<T>> : public SinkContainer<std::list<T>> {
	public:
		Sink(std::shared_ptr<IConnection> conn, Store &store) : SinkContainer(conn, store) {}
	};

	template<class T>
	class Sink<std::deque<T> > : public SinkContainer<std::deque<T> > {
	public:
		Sink(std::shared_ptr<IConnection> conn, Store &store) : SinkContainer(conn, store) {}
	};

	template<class T>
	class Sink<std::vector<T> > : public SinkContainer<std::vector<T> > {
	public:
		Sink(std::shared_ptr<IConnection> conn, Store &store) : SinkContainer(conn, store) {}
	};

	template<class T>
	class Sink<std::map<typename T::key_type,T> > : public ISink<std::map<typename T::key_type,T> > {
		using D = std::map<typename T::key_type,T>;
		Sink<T> sink;
	public:
		Sink(std::shared_ptr<IConnection> conn, Store &store) : sink(conn, store) {}
		void push_back(const D &d) {
			for (auto &v : d)
				sink.push_back(v.second);
		}
	};
}