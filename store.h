#pragma once
#include "interface.h"
#include "metadata.h"
#include "source.h"
#include "sink.h"
#include <list>
#include <map>
#include <typeinfo>
#include <string>
#include <memory>
#include <deque>

namespace dk {
	class Store {
		class IObject {
		public:
			IObject() {}
			IObject(const IObject &)=delete;
			IObject &operator=(const IObject &)=delete;

			virtual void save(std::shared_ptr<IConnection>  conn) const {};
			virtual ~IObject(){}
		};
		template<class T,class ...Ts>
		class Object;

		template<class K, class T>
		class Object<std::map<K,T>> : public IObject {
			friend Store;
			using M=std::map<K,T>;
			M m_;
			auto &get() { return m_; }
		public:
			const auto &get() const { return m_; }
			Object(std::shared_ptr<IConnection> conn) {
				for (auto v : Source<T>(std::move(conn)))
					m_.insert(typename M::value_type(v,std::move(v)));
			}
			void save(std::shared_ptr<IConnection> conn) const override {
				Sink<T> sink(std::move(conn));
				for (auto &v : m_) {
					sink.push_back(v.second);
				}
			}
			virtual ~Object() {}
		};

		template<class T>
		using DataObject=Object<std::map<typename T::key_type,T>>;

		template<class T>
		class Object<std::vector<T const *>> : public IObject {
			std::vector<T const *> m_;
		public:
			Object(const DataObject<T> &obj) {
				for (const auto &t : obj.get())
					m_.push_back(&t.second);
			}
			const auto &get() const { return m_; }
			virtual ~Object() {}
		};

		template< class S>
		std::unique_ptr<DataObject<S>> makeObject_(std::shared_ptr<IConnection> conn) {
			return std::make_unique<DataObject<S> >(conn);
		}
		template< class S,class T>
		std::unique_ptr<IObject> makeObject_(const DataObject<T> &t) {
			return std::make_unique<Object<S> >(t);
		}

		std::shared_ptr<IConnection> conn;

		using Objects=std::map<std::string, std::unique_ptr<IObject>>;
		Objects data_, projection_;//, maplists;

		template<class T>
		std::string makeName_() const {
			std::string name= typeid(T).name();
			return name;
		}

		template<class T>
		void loadObject_() {
			std::string name= makeName_<T>();

//create dataObject
			auto ptr = makeObject_<T>(conn);
			auto &obj=*ptr.get(); // hold ref before moving

			data_.insert(
				Objects::value_type(
						name,
						std::move(ptr)
				)
			).first;

			projection_.insert(
				Objects::value_type(
					name,
					makeObject_<std::vector<T const *>>(obj)
				)
			);

//resolve pointers
			const metadata<T> r;
			for (auto &v : obj.get())
				r.resolve(*this,(void*)&v.second);
		}

		template<class T,class U>
		IObject *find_(const U &u) const {
			std::string name = makeName_<T>();
			IObject *rtn=0;
			auto it=u.find(name);
			if (it != u.end())
				rtn=it->second.get();
			return rtn;
		}
		template<class T,class U>
		IObject *load_(const U &u) {
			auto obj=find_<T>(u);
			if(!obj) {
				loadObject_<T>();
				obj=find_<T>(u);
			}
			assert(obj);
			return obj;
		}

	public:
		Store(std::shared_ptr<IConnection> conn) :conn(std::move(conn)) {}
		~Store() {}

		void save(std::shared_ptr<IConnection> &conn) const {
			for (auto &v : data_) {
				v.second->save(conn);
			}
		}
		template<class S>
		void save(std::shared_ptr<IConnection> &conn) const {
			auto it = data_.find(makeName_<S>());
			if(it!=data_.end())
				it->save(conn);
		}

		template<class T>
		auto projection() {
			using V=std::vector<T const*>;
			return static_cast<Object<V>*>(load_<T>(projection_))->get();
		}

		template<class T>
		auto projection() const {
			return const_cast<Store*>(*this)->projection<T>();
		}

		template<class T>
		const std::map<typename T::key_type,T> &getMap() {
			using M=std::map<typename T::key_type,T>;
			return static_cast<Object<M>*>(load_<T>(data_))->get();
		}
		template<class T>
		friend const std::map<typename T::key_type,T> &getMap(Store &s) {
			return s.getMap<T>();
		}
	};
}
