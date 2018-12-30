#pragma once
#include "store.h"
#include <deque>

namespace dk {
	template<class T>
	class ISink {
	public:
		using DTL = T;
		virtual ~ISink() {}
		virtual void push_back(const T &t) = 0;
	};
	
	template<class T>
	class Sink {
		std::shared_ptr<IConnection> conn;
		std::unique_ptr <IStatement> stmt;
		uint32_t counter;
		std::unique_ptr<IRecord> record;
		void create() {
			std::string sql = conn->getMetaData().createSQL(*record.get());
			conn->execute(sql);
		}
		void open() {
			std::string sql = conn->getMetaData().insertSQL(*record.get());
			stmt = conn->createStatement(sql);
			counter = 0;
		}
		void close() {
			stmt->flush();
			stmt->commit();
		}
	public:
		Sink(std::shared_ptr<IConnection> conn) :
			conn(conn),record(std::make_unique<Record>(metadata<T>()))
		{
			create();
			open();
		}
		~Sink() { close(); }
		void push_back(const T &t) {
			if (!(counter % 1000)) { conn->commit(); }
			counter++;
			assert(stmt.get());
			record->set(*stmt.get(), (void*)&t);
			stmt->executeUpdate();
		}
	};
}
