#pragma once

#include "typeinfo"
#include <cassert>
#include <tuple>
#include <memory>
#include "source.h"

namespace xxxtransition {
	class Store;
	template<class S, class T>
	class ITransform {
	protected:
		using DST = S;
		using SRC = T;
		ITransform() {}
	public:
		virtual ~ITransform() {}
		virtual void operator()(DST &dst, const SRC &src) const = 0;
		const DST operator()(const SRC &src) const {
			DST dst;
			this->operator()(dst, src);
			return dst;
		}
		static const ITransform &getInstance(Store &store);
		static const ITransform &getInstance();
	};

	template<class DST, class SRC>
	class Transform : public ITransform<DST, SRC> {
		friend ITransform<DST, SRC>;
		const Store *store;
	public:
		Transform(Store &store);
		Transform();// :store(0) {};

		~Transform() {}
		void operator()(DST &dst, const SRC &src) const override;
	};

/*	template<class DST, class SRC, class MID>
	class MTransform : public ITransform<DST, SRC> {
		friend ITransform<DST, SRC>;
		using TRANS1 = ITransform<DST, MID>;
		using TRANS2 = ITransform<MID, SRC>;

		const TRANS1 &trans1;
		const TRANS2 &trans2;
		MTransform(Store &store) :
			trans1(TRANS1::getInstance(store)),
			trans2(TRANS2::getInstance(store)) {}
	public:
		~MTransform() {}
		void operator()(DST &dst, const SRC &src) const override {
			trans1(dst, trans2(src));
		}
	};*/
}
