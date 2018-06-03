#pragma once

#include <ctime>
#ifdef _DEBUG
#include <string>
#include <sstream>
#include <iomanip>
#endif

namespace dk {
	class timestamp {
#ifdef _DEBUG
		std::string _t;
#endif
		//		time_t t;
		struct tm t;
	public:
		timestamp() { memset(&t, 0, sizeof(t)); }
		timestamp(const tm &a) {
			*this = a;
		}
		friend bool operator<(const timestamp &a, const timestamp &b) {
			return a.t.tm_year < b.t.tm_year ? true :
				b.t.tm_year < a.t.tm_year ? false :
				a.t.tm_yday < b.t.tm_yday ? true :
				b.t.tm_year < a.t.tm_year ? false :
				a.t.tm_hour < b.t.tm_hour ? true :
				b.t.tm_hour < a.t.tm_hour ? false :
				a.t.tm_min < b.t.tm_min ? true :
				b.t.tm_min < a.t.tm_min ? false :
				a.t.tm_sec < b.t.tm_sec ? true : false;
		}
		void as_tm(tm &a) const {
			//			memcpy(&a, gmtime(&t), sizeof(tm));
			memcpy(&a, &t, sizeof(a));
		}
		void operator=(const tm &a) {
			memcpy(&t, &a, sizeof(t));
			mktime(&t);
			debug_copy();
		}
		uint32_t yyyymmdd() const {
			return (t.tm_year + 1900) * 10000 +
				(t.tm_mon + 1) * 100 + t.tm_mday;
		}
		uint32_t hh24miss() const {
			return t.tm_hour * 10000 +
				t.tm_min * 100 +
				t.tm_sec;
		}
		uint32_t hh24mi() const {
			return hh24miss() / 100;
		}
		uint32_t dow() const {
			return t.tm_wday + 1;
		}
		static timestamp now() {
			time_t rawtime;
			time(&rawtime);
			return timestamp(*localtime(&rawtime));
		}
		friend time_t operator-(const timestamp &a, const timestamp &b) {
			tm aa, bb;
			a.as_tm(aa);
			b.as_tm(bb);
			return (mktime(&aa) - mktime(&bb));
		}
#ifdef _DEBUG
		timestamp(const timestamp &a) :_t(a._t), t(a.t) {}
	private:
		void debug_copy() {
			struct tm tm_;
			memcpy(&tm_, &t, sizeof(tm));
			std::stringstream ss;
			ss <<
				(tm_.tm_year + 1900) << "-"
				<< std::setfill('0') << std::setw(2) << (tm_.tm_mon + 1) << "-"
				<< std::setfill('0') << std::setw(2) << tm_.tm_mday;
			if (tm_.tm_hour != 0 || tm_.tm_min != 0 || tm_.tm_sec != 0)
				ss << "T"
				<< std::setfill('0') << std::setw(2) << tm_.tm_hour << ":"
				<< std::setfill('0') << std::setw(2) << tm_.tm_min << ":"
				<< std::setfill('0') << std::setw(2) << tm_.tm_sec;
			_t = ss.str();
		}
#else
		timestamp(const timestamp &a) : t(a.t) {}
	private:
		void debug_copy() {}
#endif
		};
	}