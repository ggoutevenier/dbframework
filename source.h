#pragma once
#include "metadata.h"
#include <boost/algorithm/string/join.hpp>

namespace dk {
	template<class T>
	class Source {
		std::shared_ptr<IConnection> conn;
		std::unique_ptr<IStatement> stmt;
		class iterator {
			std::unique_ptr<IResultSet> rset;
			Source &src;
		public:
			explicit iterator(Source &s):src(s) {}
			iterator &end() {
				rset.reset();
				return *this;
			}
			iterator &begin() {
				rset = src.stmt->executeQuery();
				this->operator++();
				return *this;
			}
			bool operator==(const iterator &other) const { 
				return rset.get() == other.rset.get(); 
			}
			bool operator!=(const iterator &other) const { 
				return !(*this == other); 
			}
			iterator& operator++() {
				if (!rset->next()) {
					rset.reset();
				}
				return *this;
			}
			const T operator*() {
				T t;
				src.record->get(*rset.get(),&t);
				src.resolve(t);

				return t;
			}
		};
		int pos;
		std::list<std::string> conditions;
		std::map<int,std::unique_ptr<IField> > fields; 
		template<typename U>
		void bind(const U u) {
			++pos;
			if (fields.find(pos) == fields.end()) {
				fields.insert(
					decltype(fields)::value_type(
						pos,
						std::make_unique<Field<U> >(
							-1,
							":" + std::to_string(pos),
							pos)
					)
				);
			}
			stmt->bind(u, *fields.at(pos).get());
		}
		template<typename U, typename... Args>
		void bind(U first, Args... args) {
			bind(first);
			bind(args...);
		}
		Store *store;
		std::unique_ptr<IRecord> record;
	public:
		Source(const Source &)=delete;
		Source(std::shared_ptr<IConnection> conn) :
			conn(std::move(conn)),
			pos(0),
			store(0),
			record(std::make_unique<Record>(metadata<T>())){}
		Source(std::shared_ptr<IConnection> conn,Store &s) :
			conn(std::move(conn)),
			pos(0),
			store(&s),
			record(std::make_unique<Record>(metadata<T>())){}
		~Source() {
		}
		bool is_open() { return stmt.get(); }
		void open() {
			std::stringstream ss;
			ss << conn->getMetaData().selectSQL(*record.get());
			if (!conditions.empty())
				ss << " where " << boost::algorithm::join(conditions, " and ");
			stmt = conn->createStatement(ss.str());
			pos = 0;
		}
		template<typename... ARGS>
		Source &open(ARGS... args) {
			open();
			bind(args...);
			return *this;
		}
		Source &condition(const std::string &s) {
			conditions.push_back(s);
			return *this;
		}
		iterator begin() { 
			if (!is_open()) open();
			auto rtn=iterator(*this);
			rtn.begin();
			return rtn;
		}
		iterator end() {
			auto rtn=iterator(*this);
			rtn.end();
			return rtn;
		}
		bool fetch(T &t) {
			iterator it = begin();
			if (it == end()) return false;
			t = *it;
			return true;
		}
		void resolve(T &t) {
			if (store)
				record->resolve(*store,&t);
		}
	};
}
