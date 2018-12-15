#pragma once
#include "timestamp.h"
#include "type.h"
#include <string>

namespace dk {
	template<class T> class XForm : public Type<T> {};


	template<size_t N>
	class XForm<dec::decimal<N> > : public Type<double> {
	public:
		XForm() {}
		~XForm() {}
		void set(IStatement &writer, const  void *data, IField &field) const override {
			auto &t = *static_cast<const dec::decimal<N>*>(data);
			double src = t.getAsDouble();
			Type<double>::set(writer, &src, field);
		}
		void get(IResultSet &reader, void *data, IField &field) const override {
			double src;
			auto &t = *static_cast<dec::decimal<N>*>(data);
			Type<double>::get(reader, &src, field);
			t = src;
		}
	};

	template<>
	class XForm<bool> : public Type<char> {
		const std::array<char, 2> TF; 
	protected:
	public:
		XForm(std::array<char, 2> TF = { '1','0' }) : TF(TF) {}
		~XForm() {}
		void set(IStatement &writer, const  void *data, IField &field) const override {
			char src;
			auto &t = *static_cast<const bool*>(data);
			src = TF.at(t == false);
			Type<char>::set(writer, &src, field);
		}
		void get(IResultSet &reader, void *data, IField &field) const override {
			char src;
			auto &t = *static_cast<bool*>(data);
			Type<char>::get(reader, &src, field);
			if (!(src == TF.at(0) || src == TF.at(1) || src == 0)) {
				//convertion error
			}
			t = (src == TF.at(0));
		}
	};

	template<>
	class XForm<timestamp> : public Type<tm> {
		typedef timestamp T;
	public:
		XForm() {}
		~XForm() {}
		void set(IStatement &writer, const  void *data, IField &field) const override {
			auto &t = *static_cast<const timestamp*>(data);
			tm src;
			t.as_tm(src);
			Type<tm>::set(writer, &src, field);
		}
		void get(IResultSet &reader, void *data, IField &field) const override {
			tm src;
			auto &t = *static_cast<timestamp*>(data);
			Type<tm>::get(reader, &src, field);
			t = src;
		}
	};
}
