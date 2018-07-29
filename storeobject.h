#pragma once
#include "interface.h"
#include "source.h"
#include "sink.h"
//#include <map>

namespace dk {
	class Store;
	template<class T,class ...Ts>
	class StoreObject;

	template<class K, class T>
	class StoreObject<std::map<K,T>> : public IStoreObject {
		friend Store;
		using M=std::map<K,T>;
		M m_;
		auto &get() { return m_; }
	public:
		const auto &get() const { return m_; }
		void load(std::shared_ptr<IConnection> conn) override {
			for (auto v : Source<T>(std::move(conn)))
				m_.insert(typename M::value_type(v,std::move(v)));
		}
		void save(std::shared_ptr<IConnection> conn) const override {
			Sink<T> sink(std::move(conn));
			for (auto &v : m_) {
				sink.push_back(v.second);
			}
		}
	};

	template<class T>
	using DataObject=StoreObject<std::map<typename T::key_type,T>>;

	template<class T>
	class StoreObject<std::vector<T const *>> : public IStoreObject {
		std::vector<T const *> m_;
	public:
		StoreObject(const DataObject<T> &obj) {
			for (const auto &t : obj.get())
				m_.push_back(&t.second);
		}
		const auto &get() const { return m_; }
	};

	template< class S>
	std::unique_ptr<DataObject<S>> makeDataObject() {
		return std::make_unique<DataObject<S> >();
	}
	template< class S,class T>
	std::unique_ptr<IStoreObject> makeStoreObject(const DataObject<T> &t) {
		return std::make_unique<StoreObject<S> >(t);
	}
}
