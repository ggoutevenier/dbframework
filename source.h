#pragma once
//#include <memory>
//#include "interface.h"
#include "metadata.h"
#include "store.h"
#include <boost/noncopyable.hpp>
#include <boost/algorithm/string/join.hpp>

namespace dk {
	template<class T>
	class Source : public boost::noncopyable {
		std::shared_ptr<IConnection> conn;
		std::unique_ptr<IStatement> stmt;
	protected:
		class iterator {
			std::unique_ptr<IResultSet> rset;
			T t;
			Source *src;
		public:
			explicit iterator():src(0) {}
			explicit iterator(Source &s):src(&s) {
				assert(src);
				rset = src->stmt->executeQuery();
				this->operator++();
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
					src->pos = 0;
				}
				return *this;
			}
			const T& operator*() {
				metadata<T>::record().get(*rset.get(), &t);
				if (src->store)
					src->store->resolve(t);
				return t;
			}
		};
		int pos;
		std::list<std::string> conditions;
		std::map<int,std::unique_ptr<IField> > fields; 
		template<typename T>
		void bind(const T &t) {
			++pos;
			if (fields.find(pos) == fields.end()) {
				fields.insert(
					decltype(fields)::value_type(
						pos,
						std::make_unique<Field<T> >(
							-1,
							":" + boost::lexical_cast<std::string>(pos),
							pos)
					)
				);
			}
			stmt->bind(t, *fields.at(pos).get());
		}
		template<typename T, typename... Args>
		void bind(T first, Args... args) {
			bind(first);
			bind(args...);
		}
		Store *store;
	public:
		Source(std::shared_ptr<IConnection> &conn) : conn(conn),store(0) {}
		Source(std::shared_ptr<IConnection> &conn,Store &s) : conn(conn), store(&s) {}
		~Source() {
		}
		bool is_open() { return stmt.get(); }
		void open() {
			std::stringstream ss;
			ss << conn->getMetaData().selectSQL(metadata<T>::record());
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
			return iterator(*this); 
		}
		iterator end() { return iterator(); }
		bool fetch(T &t) {
			iterator it = begin();
			if (it == end()) return false;
			t = *it;
			return true;
		}
	};
}