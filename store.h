#pragma once
#include "source.h"
#include <list>
#include <map>
#include <typeinfo>
#include <string>
#include "sink.h"

namespace dk {
	class IObject {
	public:
		virtual void save(std::shared_ptr<IConnection>  &conn) const {};
		virtual void load(std::shared_ptr<IConnection>  &conn) {};
	};
			
	template<class T,class ...Ts>
	class Object;

	template<class T>
	class Object<std::deque<T> > : std::deque<T> , public IObject {
		friend Store;
		std::deque<T> &get() { return *this; }
	public:
		const std::deque<T> &get() const { return *this; }
		void load(std::shared_ptr<IConnection> &conn) override {
			for (auto &v : Source<T>(conn))
				this->push_back(v);
		}
		void save(std::shared_ptr<IConnection> &conn) const override {
			Sink<T> sink(conn);
			for (auto &v : *this) {
				sink.push_back(v);
			}
		}
	};

	template<class T>
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
	};

	template<class T>
	class Object<std::map<typename T::key_type,const T*> > :  
		private std::map<typename T::key_type, const T*>,
		public IObject {
	public:
		Object(const Object<std::deque<T> > &obj) {
			for (const T &t : obj.get()) {
				if (!this->try_emplace(t, &t).second) {
					throw std::runtime_error("Duplicate error");
				}
			}
		}
		const std::map<typename T::key_type, const T*> &get() const { return *this; }
	};

	template<class K,class T>
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
//				if (insert(std::multimap<K, const T*>::value_type(K(t), &t))==this->end()) {
//					throw std::runtime_error("error multimap insert");
//				}
			}
		}
		const std::multimap<K, const T*> &get() const { return *this; }
	};

/*	template<class K, class T>
	void stransform(std::map<K, const T*> &dst, const std::list<const T*> &src) {
		for (const T *v : src)
			dst.try_emplace(static_cast<K>(*v), v);
	}*/

	class Store {
		std::shared_ptr<IConnection> conn;

		typedef std::map<std::string, std::unique_ptr<IObject> > Objects;
		Objects deques, maps, maplists;

		template<class T>
		Object<std::deque<T> > &add(std::string name) {
			Objects::iterator it;
			it = deques.try_emplace(
				name, 
				std::make_unique<Object<std::deque<T>> >()).first;
			assert(it != deques.end());
			return *static_cast<Object<std::deque<T> > *>(it->second.get());
		}
		template<class T>
		const Object<std::deque<T> > *find()  const{
			std::string name = typeid(T).name();
			//find
			Objects::const_iterator it = deques.find(name);
			if (it == deques.end()) {
				return 0;
			}
			return static_cast<Object<std::deque<T> > *>(it->second.get());
		}

		template<class T>
		Object<std::deque<T> > &load() {
			std::string name= typeid(T).name();
//find
			Objects::iterator it = deques.find(name);
			if (it == deques.end()) {
//add object
				Object<std::deque<T> > &obj = add<T>(name);
//load data
				obj.load(conn);
				getMap<T>();

//resolve pointers				
				for (T &v : obj.get())
					resolve(v);

				return obj;
			}
			return *static_cast<Object<std::deque<T> > *>(it->second.get());
		}

	public:
		Store(std::shared_ptr<IConnection> &conn) :conn(conn) {}
		~Store() {}
		template<class T>
		void resolve(T &data) {
			const Record &record = metadata<T>::record();
			record.resolve(*this, &data);
		}
		template<class K,class T>
		std::string makeName() const {
			std::stringstream ss;
			ss << typeid(K).name() << "," << typeid(T).name();
			return ss.str();
		}
		template<class T>
		const std::map<typename T::key_type, const T*> &getMap() const {
			using K = typename T::key_type;
			using M = std::map<typename T::key_type, const T*>;

			std::string name = makeName<K, T>();
			Objects::const_iterator it = maps.find(name);
			assert(it != maps.end());
			const Object<M> &obj = *static_cast<Object<M>*>((it->second).get());
			const M &rtn = obj.get();
			return rtn;
		}

		template<class T>
		const std::map<typename T::key_type, const T*> &getMap() {
			using K = typename T::key_type;
			using M = std::map<typename T::key_type, const T*>;

			std::string name = makeName<K, T>();
			Objects::const_iterator it=maps.find(name);
			if (it == maps.end()) {
				maps.try_emplace(name, std::make_unique<Object<M> >(load<T>()));
				it = maps.find(name);
			}
			assert(it != maps.end());
			const Object<M> &obj = *static_cast<Object<M> *>((it->second).get());
			const M &rtn=obj.get();
			return rtn;
		}
		void save(std::shared_ptr<IConnection> &conn) const {
			for (auto &v : deques) {
				v.second->save(conn);
			}
		}
		template<class S>
		void save(std::shared_ptr<IConnection> &conn) const {
			auto ptr=find<S>();
			if(ptr)
				ptr->save(conn);
		}

		template<class K, class T>
		const std::map<K, std::list<const T*> > &getMapList() {
			using ML = std::map<K, std::list<const T*> >;
			std::string name = makeName<K, T>();
			Objects::const_iterator it = maplists.find(name);
			if (it == maplists.end()) {
				maplists.try_emplace(name, std::make_unique<Object<ML> >(load<T>()));
				it = maplists.find(name);
			}
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
		template<class T>
		const std::list<const T*>  projection() {
			std::list<const T*> rtn;
			for (auto v : getMap<T>())
				rtn.push_back(v.second);
			return rtn;
		}
		template<class T>
		const std::list<const T*>  projection() const {
			std::list<const T*> rtn;
			for (auto v : getMap<T>())
				rtn.push_back(v.second);
			return rtn;
		}

		template<class T, class T2>
		const std::list<const T*>  projection(const T2 &t2) {
			return getList<T>(static_cast<typename T2::key_type>(t2));
		}
		template<class T>
		bool reference(const typename T::key_type &k, const T **ptr) {
			return (*ptr=resolve_(k, getMap<T>()))!=0;
		}
		template<class T>
		bool reference(const typename T::key_type &k, const T **ptr) const {
			return (*ptr = resolve_(k, getMap<T>())) != 0;
		}

		template<class T>
		bool reference(Ref<T> &ref) {
			return ref.resolve(getMap<T>());
		}

		template<class T>
		bool reference(Ref<T> &ref) const {
			return ref.resolve(getMap<T>());
		}
	};

/*	template<class T,class K>
	const T *reference(Store &s, const typename K &key) {
		Ref<T> ref(key);
		s.reference(ref);
		return ref.ptr;
	}
	template<class T, class K>
	const T *reference(const Store &s, const typename K &key) {
		Ref<T> ref(key);
		s.reference(ref);
		return ref.ptr;
	}*/
}