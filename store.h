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
//			using V=std::vector<T const*>;
//			return static_cast<Object<V>*>(find_<T>(projection_))->get();
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
/*
  	template<class STL>
	class Object<STL> : STL , public IObject {
		using T = typename STL::value_type;
		friend Store;
		STL &get() { return *this; }
	public:
		const STL &get() const { return *this; }
		void load(std::shared_ptr<IConnection> conn) override {
			for (auto v : Source<T>(std::move(conn)))
				this->push_back(std::move(v));
		}
		void save(std::shared_ptr<IConnection> conn) const override {
			Sink<T> sink(std::move(conn));
			for (auto &v : *this) {
				sink.push_back(v);
			}
		}
	};

 */
/*	template<class T>
	class Object<std::vector<const T*> > : 
		private std::vector<const T*>,
		public IObject {
		friend Store;
		std::vector<const T*> &get() { return *this; }
	public:
		Object(const Object<T> &obj) {
			for (const T &t : obj.get()) 
				insert(&t);
		}
		const std::vector<const T*> &get() const { return *this; }
	};*/

/*	template<class K,class T>
	class Object<std::map<K, std::list<const T*> > > :
		private std::map<K, std::list<const T*> >,
		public IObject {
	public:
		Object(const Object<std::deque<T> > &obj) {
			for (const T &t : obj.get()) {
				this->operator[](K(t)).push_back(&t);
			}
		}
		const std::map<K, std::list<const T*> > &get() const { return *this; }
	};

	template<class K,class T>
	class Object<std::multimap<K, const T*> > :
		private std::multimap<K, const T*>,
		public IObject {
	public:
		Object(const Object<std::deque<T> > &obj) {
			for (const T &t : obj.get()) {
				insert(std::multimap<K, const T*>::value_type(K(t), &t));
			}
		}
		const std::multimap<K, const T*> &get() const { return *this; }
	};*/
/*		template<class K, class T>
		const std::map<K, std::list<const T*> > &getMapList() {
			using ML = std::map<K, std::list<const T*> >;
			std::string name = makeName<K, T>();

			auto it = maplists.find(name);
			if (it == maplists.end())
				it=maplists.emplace(name, load<T>()).first;
			assert(it != maplists.end());
			const Object<ML> &obj = *static_cast<const Object<ML> *>(it->second.get());
			const ML &rtn = obj.get();
			return rtn;
		}

		template<class T,class K>
		const std::list<const T*> &getList(const K &k) {
			using M = const std::map<K, std::list<const T*> >;
			const M &m = getMapList<K, T>();
			typename M::const_iterator it=m.find(k);
			if (it == m.end()) {
				static std::list<const T*> empty;
				return empty;
			}
			return it->second;
		}

		template<class T, class T2>
		const std::list<const T*>  projection(const T2 &t2) {
			return getList<T>(static_cast<typename T2::key_type>(t2));
		}*/
		/*template<class T>
		bool reference(const typename T::key_type &k, const T **ptr) {
			return (*ptr=resolve_(k, getMap<T>()))!=0;
		}
		template<class T>
		bool reference(const typename T::key_type &k, const T **ptr) const {
			return (*ptr = resolve_(k, getMap<T>())) != 0;
		}*/

/*		template<class T>
		bool reference(Ref<T> &ref) {
			return ref.resolve(getMap<T>());
		}

		template<class T>
		bool reference(Ref<T> &ref) const {
			return ref.resolve(getMap<T>());
		}*/
	/*template<class T>
	friend const std::map<typename T::key_type, const T*> &getMap(const Store &s) {
		return s.getMap<T>();
	}*/
/*		template<class T>
	friend void resolve(Store &s,T &t) {
//			const T *ptr=&t;
		s.resolve(t);
	}*/

/*	template<class T,class K>
	const T *reference(Store &s, const K &key) {
		Ref<T> ref(key);
		s.reference(ref);
		return ref.ptr;
	}
	template<class T, class K>
	const T *reference(const Store &s, const K &key) {
		Ref<T> ref(key);
		s.reference(ref);
		return ref.ptr;
	}
	template<class T>
	void resolve(Store &s,T &t) {
		const T *ptr=&t;
		s.reference((const typename T::key_type&)t,&ptr);
	}*/

/*		template<class T>
		class Object<std::vector<T> > : public IObject {
			friend Store;
			std::vector<T> m_;
			auto &get() { return m_; }
		public:
			const auto &get() const { return m_; }
			void load(std::shared_ptr<IConnection> conn) override {
				for (auto v : Source<T>(std::move(conn)))
					m_.push_back(std::move(v));
			}
			void save(std::shared_ptr<IConnection> conn) const override {
				Sink<T> sink(std::move(conn));
				for (auto &v : m_) {
					sink.push_back(v);
				}
			}
		};*/

/*		template<class K,class T>
		class Object<std::map<K, std::vector<T const *>>> :	public IObject {
			std::map<K, std::vector<T const *>> m_;
		public:
			template<class STL>
			Object(const STL &obj) {
				for (const T &t : obj.get())
					m_[K(t)].push_back(&t);
			}
			const auto &get() const { return m_; }
		};*/

/*		template<class T>
		class Object<std::map<typename T::key_type,T const*>> :	public IObject {
			std::map<typename T::key_type, T const*> m_;
		public:
			template<class OBJ>
			Object(const OBJ &obj) {
				for (const T &t : obj.get()) {
					if (!m_.emplace(t, &t).second) {
						throw std::runtime_error("Duplicate error");
					}
				}
			}
			const auto &get() const { return m_; }
		};*/

/*		template<class T>
		Object<std::deque<T>> &add(std::string name) {
			auto it=deques.insert(Objects::value_type(name,
					std::make_unique<Object<std::deque<T>> >()
			)).first;
			assert(it != deques.end());
			return *static_cast<Object<std::deque<T> > *>(it->second.get());
		}*/
/*		template<class T>
		const DataObject<T> *find()  const{
			std::string name = makeName<T>();

			Objects::const_iterator it = data.find(name);
			if (it == data.end()) {
				return 0;
			}
			return static_cast<DataObject<T> *>(it->second.get());
		}*/
