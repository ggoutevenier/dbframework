#pragma once
#include <memory>
#include <map>

namespace dk {
	namespace logic {
		template<class T>
		class Function;

		template<class T>
		class Functions : std::map<typename T::key_type, std::unique_ptr<Function<T> > > {
			template<class S>
			void add() {
				try_emplace(S::getId(), std::make_unique<S>());
			}

			Functions();
			static const Functions & getInstance() {
				static Functions rtn;
				return rtn;
			}
		public:
			static const Function<T> *getFunction(const typename T::key_type &key) {
				const Functions<T> &v = getInstance();
//				const_iterator it = v.find(key);
				auto it = v.find(key);
				if (it == v.end()) return 0;
				return it->second.get();
			}
		};
	}
}