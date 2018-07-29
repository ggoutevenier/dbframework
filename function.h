#pragma once
#include <memory>
#include <map>

namespace dk {
	namespace logic {
		template<class T>
		class Function;

		template<class T>
		class Functions {
			using K=typename T::key_type;
			using M=std::map<K, std::unique_ptr<Function<T> > >;
			M m_;
			template<class S>
			void add() {
				m_.insert(typename M::value_type(S::getId(), std::make_unique<S>()));
			}

			Functions(); // must be defined in custom cpp code compile in main project
			static const Functions & getInstance() {
				static Functions rtn;
				return rtn;
			}
		public:
			static const Function<T> *getFunction(const K &key) {
				auto &v = getInstance();
				auto it = v.m_.find(key);
				if (it == v.m_.end()) return 0;
				return it->second.get();
			}
		};
	}
}
