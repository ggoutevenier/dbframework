#pragma once
#include <tuple>
#include <string>
#include <map>
#include <ctime>
#include <cstring>
#include "compare.h"
#include "sequence.h"

namespace dk {
	namespace key {
		struct empty {
			friend bool operator<(const empty &a, const empty &b) {
				return true;
			}
		};

		struct vendor {
			uint32_t vendor_id;
			friend bool operator<(const vendor &a, const vendor &b) {
				return a.vendor_id < b.vendor_id;
			}
		};

		struct holiday_names  {
			uint32_t holiday_names_id;
			friend bool operator<(const holiday_names &a, const holiday_names &b) {
				return a.holiday_names_id < b.holiday_names_id;
			}
		};

		struct holiday_dates : holiday_names {
			timestamp holiday_date;
			friend bool operator<(const holiday_dates &a, const holiday_dates &b) {
				return 
					less_<holiday_names>(a,b) ? true :
					less_<holiday_names>(b, a) ? false :
					less_(a.holiday_date, b.holiday_date);
			}
		};

		struct vendor_holidays : vendor, holiday_names
		{
			friend bool operator<(const vendor_holidays &a, const vendor_holidays &b) {
				return less_<vendor, holiday_names>(a, b);
			}
		};
		struct holidays : vendor
		{
			timestamp holiday_date;
			friend bool operator<(const holidays &a, const holidays &b) {
				return
					less_<vendor>(a,b) ? true :
					less_<vendor>(b, a) ? false:
					less_(a.holiday_date, b.holiday_date);
			}
		};
		struct unit {
			uint32_t unit_id;
			friend bool operator<(const unit &a, const unit &b) {
				return a.unit_id < b.unit_id;
			}
		};
		struct bundle_type {
			uint32_t bundle_type_id;
			friend bool operator<(const bundle_type &a, const bundle_type &b) {
				return a.bundle_type_id < b.bundle_type_id;
			}
			friend bool operator==(const bundle_type &a, const bundle_type &b) {
				return !(a < b || b < a);
			}
		};
		struct calltype {
			Sequence seq;
			uint32_t calltype_id; 
			calltype(const calltype &a) : seq(a.seq), calltype_id(a.calltype_id) {}
			calltype() : calltype_id(0) {}
			friend bool operator<(const calltype &a, const calltype &b) {
				return a.calltype_id < b.calltype_id;
			}
		};

		struct feetype {
			uint32_t feetype_id; 
			friend bool operator<(const feetype &a, const feetype &b) {
				return a.feetype_id < b.feetype_id;
			}
		};
//			feetype(const feetype &a) : feetype_id(a.feetype_id) {};
//			feetype() : feetype_id(0) {};
//		};
		struct tod {
			uint32_t time_of_day;
			tod() : time_of_day(0) {}
			tod(const tod &a) : time_of_day(a.time_of_day) {}
			friend bool operator<(const tod &a, const tod &b) {
				return a.time_of_day < b.time_of_day;
			}
			friend bool operator != (const tod &a, const tod &b) {
				return a < b || b < a;
			}
		};

		struct tod_set {
			uint32_t tod_set_id;
			tod_set() : tod_set_id(0) {}
			tod_set(const tod_set &a) : tod_set_id(a.tod_set_id) {}
			friend bool operator<(const tod_set &a, const tod_set &b) {
				return a.tod_set_id < b.tod_set_id;
			}
			friend bool operator != (const tod_set &a, const tod_set &b) {
				return a < b || b < a;
			}
		};
		struct dow {
			uint32_t dow_id; 
			dow() : dow_id(0) {}
			dow(const dow &a):dow_id(a.dow_id){}
			friend bool operator<(const dow &a, const dow &b) {
				return a.dow_id < b.dow_id;
			}
		};
		struct tod_times : tod_set,	dow
		{ 
			timestamp start_time;
			tod_times() {}
			tod_times(const tod_times&a) :
				tod_set(a), dow(a),start_time(a.start_time) {}
			friend bool operator<(const tod_times &a, const tod_times &b) {
				return
					less_<tod_set, dow>(a, b) ? true :
					less_<tod_set, dow>(b, a) ? false :
					a.start_time < b.start_time;
			}
		};

		struct zone {
			uint32_t zone_id;
			zone(const zone &a) : zone_id(a.zone_id) {}
			zone() : zone_id(0) {}
			friend bool operator<(const zone &a, const zone &b) {
				return a.zone_id < b.zone_id;
			}
		};

		struct rate_plan : Sequence { 
			uint32_t rate_plan_id; 
			rate_plan() : rate_plan_id(0) {}
			rate_plan(const rate_plan &a) : rate_plan_id(a.rate_plan_id),Sequence(a) {};
			friend bool operator<(const rate_plan &a, const rate_plan &b) {
				return a.rate_plan_id < b.rate_plan_id;
			}
		};
		struct rate : rate_plan,calltype,zone
		{
			rate() {}
			rate(const rate &a) :
				rate_plan(a),
				calltype(a),
				zone(a) {}
			friend bool operator<(const rate &a, const rate &b) {
				return less_<rate_plan, calltype, zone>(a,b);
			}
		};

		struct plan  : Sequence {
			uint32_t plan_id;
			plan(uint32_t id) : plan_id(id) {}
			plan(const plan &a) : plan_id(a.plan_id),Sequence(a) {}
			plan() : plan_id(0) {}
			friend bool operator<(const plan &a, const plan &b) {
				return a.plan_id < b.plan_id;
			}
		};

		struct plan_included_calltype_key {
			uint32_t calltype_group_id;
			friend bool operator<(
				const plan_included_calltype_key &a, 
				const plan_included_calltype_key &b
			) {
				return a.calltype_group_id < b.calltype_group_id;
			}
		};
		struct bell_sharing_group  {
			std::string group_code; 
			friend bool operator<(
				const bell_sharing_group &a,
				const bell_sharing_group &b
				) {
				return a.group_code < b.group_code;
			}
		};
		struct plan_group  { 
			uint32_t plan_group_id; 
			friend bool operator<(
				const plan_group &a, const plan_group &b) {
				return a.plan_group_id < b.plan_group_id;
			}
			plan_group() :plan_group_id(0) {}
			plan_group(const plan_group&a) :plan_group_id(a.plan_group_id) {}
			plan_group(uint32_t a) :plan_group_id(a) {}
			friend bool operator==(const plan_group &a, const plan_group &b) {
				return !(a < b || b < a);
			}
		};
		struct fee : plan,feetype {
			friend bool operator<(const fee &a, const fee &b) {
				return less_<plan, feetype>(a,b);
			}
		};

		struct tagging { 
			uint32_t tag_id;	
			friend bool operator<(const tagging &a, const tagging  &b) {
				return a.tag_id < b.tag_id;
			}
		};
		struct fee_tagging { 
			uint32_t tag_id; 
			friend bool operator<(const fee_tagging &a, const fee_tagging  &b) {
				return a.tag_id < b.tag_id;
			}
		};

		struct discount_type  {
			uint32_t discount_type_id;
			discount_type(const discount_type &a) : 
				discount_type_id(a.discount_type_id) {}
			discount_type() : discount_type_id(0) {}
			discount_type(uint32_t v) : discount_type_id(v) {}
			friend bool operator<(const discount_type &a, const discount_type  &b) {
				return a.discount_type_id < b.discount_type_id;
			}
		};

		struct discount {
			uint32_t discount_id;
			discount(uint32_t id) : discount_id(id) {}
			discount(const discount &a) : discount_id(a.discount_id) {}
			discount() : discount_id(0) {}
			friend bool operator<(const discount &a, const discount  &b) {
				return a.discount_id < b.discount_id;
			}
		};
		struct discount_usage :	discount,calltype
		{
			friend bool operator<(
				const discount_usage &a, 
				const discount_usage &b
			) {
				return less_<discount, calltype>(a,b);
			}
		};
		
		struct discount_fee : discount,feetype
		{
			friend bool operator<(
				const discount_fee &a,
				const discount_fee &b
				) {
				return less_<discount, feetype>(a,b);
			}
		};
		struct tier { 
			uint32_t tier_nr; 

			friend bool operator<(const tier &a, const tier &b) {
				return a.tier_nr < b.tier_nr;
			}
			tier() :tier_nr(0) {}
			tier(const tier &a) :tier_nr(a.tier_nr) {}
			tier(uint32_t tier_nr) :tier_nr(tier_nr) {}
		};
		struct discount_tier : discount,tier
		{
			friend bool operator<(
				const discount_tier &a,
				const discount_tier &b
				) {
				return less_<discount, tier>(a,b);
			}
		};

		struct calltype_group {
			uint32_t calltype_group_id;
			friend bool operator<(
				const calltype_group &a, 
				const calltype_group &b
			) {
				return a.calltype_group_id < b.calltype_group_id;
			}
			calltype_group() :calltype_group_id(0) {}
			calltype_group(const calltype_group &a) :
				calltype_group_id(a.calltype_group_id) {}
			calltype_group(uint32_t calltype_group_id) :
				calltype_group_id(calltype_group_id) {}
		};
		/*struct discount_calltype_group : discount, calltype_group
		{
			friend bool operator<(
				const discount_calltype_group &a,
				const discount_calltype_group &b
				) {
				return less_<discount, calltype_group>(a,b);
			}
		};*/
					
		struct bundle_map :	discount {
			friend bool operator<(
				const bundle_map &a,
				const bundle_map &b
				) {
				return less_<discount>(a, b);
			}
		};
		struct ordering { 
			uint32_t order_nr; 
			friend bool operator<(
				const ordering &a,
				const ordering &b
				) {
				return a.order_nr < b.order_nr;
			}
		};
		struct discount_plan : plan,ordering
		{
			friend bool operator<(
				const discount_plan &a,
				const discount_plan &b
				) {
				return less_<plan, ordering>(a,b);
			}
		};

		struct plan_req_bundles : plan,bundle_type
		{
			friend bool operator<(
				const plan_req_bundles &a,
				const plan_req_bundles &b
				) {
				return less_<plan, bundle_type>(a,b);
			}
		};
		
		struct request_category {
			uint32_t request_category_id;
			friend bool operator<(
				const request_category &a,
				const request_category &b
				) {
				return a.request_category_id < b.request_category_id;
			}
		};
		struct client_plan { 
			uint32_t client_plan_id; 
			friend bool operator<(
				const client_plan &a,
				const client_plan &b
				) {
				return a.client_plan_id < b.client_plan_id;
			}
		};
		struct value_segment { 
			uint32_t value_segment_id;
			friend bool operator<(
				const value_segment &a,
				const value_segment &b
				) {
				return a.value_segment_id < b.value_segment_id;
			}
		};

		struct package  {
			uint32_t package_id;
			package(uint32_t id) : package_id(0) {}
			package(const package &a) : package_id(a.package_id) {}
			package() : package_id(0) {}
			friend bool operator<(
				const package &a,
				const package &b
				) {
				return a.package_id < b.package_id;
			}

		};
		struct soc {
			std::string soc_code;
//			soc(soc &a) : soc_code(a.soc_code) {}
//			soc() {}
			friend bool operator<(
				const soc &a,
				const soc &b
				) {
				return a.soc_code < b.soc_code;
			}
		};
		struct advice_type {
			uint32_t advice_type_id;
			friend bool operator<(
				const advice_type &a,
				const advice_type &b
				) {
				return a.advice_type_id < b.advice_type_id;
			}
		};
		using advice = advice_type;
/*		struct tier {
			uint32_t tier_id;
			friend bool operator<(const tier &a, const tier &b) {
				return a.tier_id < b.tier_id;
			}
		};*/
/*		struct aci_soc {
			std::string soc;
			friend bool operator<(const aci_soc &a, const aci_soc &b) {
				return a.soc < b.soc;
			}
		};*/

		struct soc_agent_tier : soc, tier {
			bool friend operator<(
				const soc_agent_tier &a, 
				const soc_agent_tier &b
				) {
				return less_<soc, tier>(a, b);
			}
		};
		struct discount_agent_tier : discount, tier {
			bool friend operator<(
				const discount_agent_tier &a,
				const discount_agent_tier &b
				) {
				return less_<discount, tier>(a, b);
			}
		};
		struct soc_advice : soc, advice_type {
			bool friend operator<(
				const soc_advice &a,
				const soc_advice &b
				) {
				return less_<soc, advice_type>(a, b);
			}
		};
		struct discount_advice : discount, advice_type {
			bool friend operator<(
				const discount_advice &a,
				const discount_advice &b
				) {
				return less_<discount, advice_type>(a, b);
			}
		};
		struct advice_parameter_type {
			uint32_t parameter_type_id;
			bool friend operator<(
				const advice_parameter_type &a,
				const advice_parameter_type &b
				) {
				return a.parameter_type_id < b.parameter_type_id;
			}
			advice_parameter_type() : parameter_type_id(0) {}
			advice_parameter_type(const advice_parameter_type &v) : 
				parameter_type_id(v.parameter_type_id) {}
			advice_parameter_type(uint32_t v) : parameter_type_id(v) {}
		};
		struct inclusion {
			uint32_t inclusion_id;
			bool friend operator<(
				const inclusion &a,
				const inclusion &b
				) {
				return a.inclusion_id < b.inclusion_id;
			}
			inclusion() : inclusion_id(0) {}
			inclusion(const inclusion &v) : inclusion_id(v.inclusion_id) {}
			inclusion(uint32_t v) : inclusion_id(v) {}

		};
/*		struct advice_type {
			unsigned int advice_type_id;
			bool friend operator<(
				const advice_type &a,
				const advice_type &b
				) {
				return a.advice_type_id < b.advice_type_id;
			}
		};*/

		struct advice_inclusion : advice_type , inclusion {
			bool friend operator<(
				const advice_inclusion &a,
				const advice_inclusion &b
				) {
				return less_<advice_type, inclusion>(a, b);
			}
		};

		struct log {
			uint32_t log_id;
			friend bool operator<(const log &a, const log &b) {
				return a.log_id < b.log_id;
			}
		};

		struct Period  {
			uint32_t YYYYMM;
			Period(uint32_t v) : YYYYMM(v) {}
			Period(const Period &v) : YYYYMM(v.YYYYMM) {}
			Period() : YYYYMM(0) {}
			friend bool operator<(
				const Period &a,
				const Period &b
				) {
				return a.YYYYMM < b.YYYYMM;
			}
			uint32_t month() { return YYYYMM % 100; }
			uint32_t year() { return YYYYMM / 100; }
			Period(uint32_t year,uint32_t month) : YYYYMM(year*100+month) {}
			friend bool operator == (
				const Period &a,
				const Period &b
				) {
				return !(a < b || b < a);
			}
			friend bool operator != (
				const Period &a,
				const Period &b
				) {
				return (a < b || b < a);
			}
			Period &operator++() {
				YYYYMM+=(YYYYMM % 100 == 12) ? 89 : 1;
				return *this;
			}
			friend bool operator<=(const Period &a, const Period &b) {
				return !(b < a);
			}
		};

		struct plan_discount :plan,discount {
			friend bool operator<(
				const plan_discount &a,
				const plan_discount &b
				) {
				return less_<plan,discount>(a,b);
			}
		};
	
		struct plan_package : plan,package {
			friend bool operator<(
				const plan_package &a,
				const plan_package &b
				) {
				return less_<plan, package>(a,b);
			}
		};

		struct Account {
			uint64_t account_nr;
			Account(uint64_t account_nr) : account_nr(account_nr) {}
			Account(const Account &a) : account_nr(a.account_nr) {}
			Account() : account_nr(0) {}
			friend bool operator<(
				const Account &a,
				const Account &b
				) {
				return a.account_nr < b.account_nr;
			}
		};
		struct Subscriber  {
			uint64_t subscriber_nr;
			Subscriber(uint64_t subscriber_nr) :
				subscriber_nr(subscriber_nr) {}
			Subscriber(const Subscriber &s) : 
				subscriber_nr(s.subscriber_nr) {}
			Subscriber() :	subscriber_nr(0) {}
			friend bool operator<(
				const Subscriber &a,
				const Subscriber &b
				) {
				return a.subscriber_nr < b.subscriber_nr;
			}
			friend bool operator ==(
				const Subscriber &a,
				const Subscriber &b
			){
				return !(a < b || b < a);
			}
		};

		struct CallSummary : Account, Subscriber, Period, calltype, zone
		{
			friend bool operator<(
				const CallSummary &a,
				const CallSummary &b
				) {
				return less_<Account, Subscriber, Period, calltype, zone>(a, b);
			}
		};
		struct FeeSummary : Account, Subscriber, Period, feetype {
			friend bool operator<(
				const FeeSummary &a,
				const FeeSummary &b
				) {
				return less_<Account, Subscriber, Period, feetype>(a, b);
			}
		};

		struct subscriber_hist : Subscriber, Period {
			friend bool operator<(
				const subscriber_hist &a,
				const subscriber_hist &b
				) {
				return less_<Subscriber, Period>(a,b);
			}
		};
		struct planlist {
			uint32_t planlist_id;
			friend bool operator<(
				const planlist &a,
				const planlist &b
				) {
				return a.planlist_id < b.planlist_id;
			}
		};
		struct  request {
			uint32_t request_id;
			friend bool operator<(
				const request &a,
				const request &b
				) {
				return a.request_id < b.request_id;
			}
		};
		struct request_plan : plan,request {
			friend bool operator<(
				const request_plan &a,
				const request_plan &b
				) {
				return less_<plan, request>(a,b);
			}
		};
		struct plan_planlist : plan,planlist {
			friend bool operator<(
				const plan_planlist &a,
				const plan_planlist &b
				) {
				return less_<plan, planlist>(a,b);
			}
		};
		struct plan_sharing : plan,calltype {
			friend bool operator<(
				const plan_sharing &a,
				const plan_sharing &b
				) {
				return less_<plan, calltype>(a,b);
			}
		};
		struct package_discount : package,discount {
			friend bool operator<(
				const package_discount &a,
				const package_discount &b
			) {
				return less_<package, discount>(a,b);
			}
		};
		struct package_plan : package,plan {
			friend bool operator<(
				const package_plan &a,
				const package_plan &b
			) {
				return less_<package, plan>(a,b);
			}
		};
		struct package_map : package {
			friend bool operator<(
				const package_map &a,
				const package_map &b
			) {
				return less_<package>(a,b);
			}
		};
		struct predicate {
			uint32_t predicate_id;
			friend bool operator<(
				const predicate &a,
				const predicate &b
				) {
				return a.predicate_id < b.predicate_id;
			}
			predicate() :predicate_id(0) {}
			predicate(const predicate &a) :predicate_id(a.predicate_id) {}
			predicate(uint32_t predicate_id) :predicate_id(predicate_id) {}
		};

		struct condition {
			uint32_t condition_id; 
			friend bool operator<(
				const condition &a,
				const condition &b
				) {
				return a.condition_id < b.condition_id;
			}
		};

		struct predicate_condition : predicate, condition {
			friend bool operator<(
				const predicate_condition &a,
				const predicate_condition &b
				) {
				return less_<predicate, condition>(a,b);
			}
		};

		template<class T>
		struct conditions : condition, T {
			friend bool operator<(
				const conditions<T> &a,
				const conditions<T> &b
				) {
				return less_<condition, T>(a, b);
			}
		};

/*		struct predicate_discount : predicate, discount {
			friend bool operator<(
				const predicate_discount &a,
				const predicate_discount &b
				) {
				return less_<predicate discount>(a,b);
			}
		};
		struct predicate_discount_condition : predicate_discount, condition{
			friend bool operator<(
				const predicate_discount_condition &a,
				const predicate_discount_condition &b
				) {
				return less_<predicate_discount, condition>(a,b);
			}
		};*/
		struct rule_type {
			uint32_t rule_type_id;
			friend bool operator<(const rule_type &a, const rule_type &b) {
				return a.rule_type_id < b.rule_type_id;
			}
			rule_type() :rule_type_id(0) {}
			rule_type(const rule_type &a) :rule_type_id(a.rule_type_id) {}
			rule_type(uint32_t rule_type_id) :rule_type_id(rule_type_id) {}
		};
		struct rule {
			uint32_t rule_id;
			friend bool operator<(const rule &a, const rule &b) {
				return a.rule_id < b.rule_id;
			}
		};
		template<class S,class T>
		struct rules_by : rule {
			S from;
			T to;
			friend bool operator<(const rules_by<S,T> &a, const rules_by<S,T> &b) {
				return	less_<rule>(a, b) ? true :
					less_<rule>(b, a) ? false :
					a.from < b.from ? true :
					b.from < a.from ? false :
					a.to < b .to;
			}
		};

		struct rules_by_group : rules_by<plan_group, plan_group> {
			using T = rules_by<plan_group, plan_group>;
			friend bool operator<(const rules_by_group &a, const rules_by_group &b) {
				return	less_<T>(a, b);
			}
		};
		struct rules_by_plan : rules_by<client_plan,plan> {
			using T = rules_by<client_plan, plan>;
			friend bool operator<(const rules_by_plan &a, const rules_by_plan &b) {
				return	less_<T>(a, b);
			}
		};
		struct advice_category {
			uint32_t advice_category_id;
			friend bool operator<(
				const advice_category &a, 
				const advice_category &b
			) {
				return a.advice_category_id < b.advice_category_id;
			}
		};

		struct subscriber_soc : Subscriber, soc {
			friend bool operator<(
				const subscriber_soc &a,
				const subscriber_soc &b
				) {
				return less_<Subscriber, soc>(a,b);
			}
		};

		struct invoice : Subscriber, Period {
			friend bool operator<(
				const invoice &a,
				const invoice &b
				) {
				return less_<Subscriber, Period>(a,b);
			}
		};

		struct plan_t :	Subscriber, Period, plan{
			plan_t(const plan_t &p) :
				Subscriber(p),
				Period(p),
				plan(p) 
			{}
			plan_t(const Subscriber &s,const Period &period,const plan &plan) :
				Subscriber(s),
				Period(period),
				plan(plan) {}
			friend bool operator<(
				const plan_t &a,
				const plan_t &b
				) {
				return less_<Subscriber, Period, plan>(a,b);
			}
		};

		struct discount_t :	plan_t,discount {
			discount_t(const plan_t &p, const discount &d) :plan_t(p), discount(d) {}
			friend bool operator<(
				const discount_t &a,
				const discount_t &b
				) {
				return less_<plan_t, discount>(a,b);
			}
		};

		struct package_t : plan_t,package
		{
			package_t(const plan_t &plan, const package &pack) :
				plan_t(plan), package(pack) {}
			friend bool operator<(
				const package_t &a,
				const package_t &b
				) {
				return less_<plan_t, package>(a,b);
			}
		};
		struct Advice : Subscriber, advice_type
		{
			Advice(const Subscriber &s, const advice_type &a) :
				Subscriber(s), advice_type(a) {}
			friend bool operator<(
				const Advice &a,
				const Advice &b
				) {
				return less_<Subscriber, advice_type>(a,b);
			}
		};


/*		struct Exclusion : plan, discount, package {
			Exclusion(const Exclusion &e) :
				plan(e),
				discount(e),
				package(e) {}
			Exclusion() {}
			Exclusion(const plan &plan,const discount &d,const package &pack) :
					plan(plan), discount(d), package(pack) {}
			friend bool operator<(
				const Exclusion &a,
				const Exclusion &b
				) {
				return less_<plan, discount, package>(a,b);
			}
		};*/

		struct handset {
			uint32_t handset_id;
			bool friend operator<(const handset &a, const handset &b) { 
				return a.handset_id<b.handset_id; 
			}
		};

		struct brand {
			uint32_t brand_id;
			bool friend operator<(const brand &a, const brand &b) { 
				return a.brand_id<b.brand_id; 
			}
		};

		struct region {
			uint32_t region_id;
			bool friend operator<(const region &a, const region &b) { 
				return a.region_id<b.region_id; 
			}
		};
		struct discount_region : region, discount {
			bool friend operator<(const discount_region &a, const discount_region &b) {
				return less_<region, discount>(a, b);
			}
		};
		struct plan_handset : public plan, handset {
			bool friend operator<(const plan_handset &a, const plan_handset &b) { 
				return less_<plan, handset>(a, b);
			}
		};

		struct plan_brand : public plan, brand {
			bool friend operator<(const plan_brand &a, const plan_brand &b) { 
				return less_<plan, brand>(a, b);
			}
		};

		struct plan_region : public plan, region {
			bool friend operator<(const plan_region &a, const plan_region &b) {
				return less_<plan, region>(a, b);
			}
		};

		struct promo_group {
			uint32_t promo_group_id;
			bool friend operator<(const promo_group &a, const promo_group &b) { 
				return a.promo_group_id < b.promo_group_id;
			}
		};
		struct plan_promo_group : public plan, promo_group {
			bool friend operator<(
				const plan_promo_group &a, 
				const plan_promo_group &b
			) {
				return less_<plan, promo_group>(a, b);
			}

		};
		struct promo_group_map {
			key::promo_group from, to;
			bool friend operator<(const promo_group_map &a, const promo_group_map &b) { 
				return a.from < b.from ? true :
					b.from < a.from ? false :
					a.to < b.to;
			}
		};
/*		struct bell_soc {
			std::string soc;
			bool friend operator<(const bell_soc &a, const bell_soc &b) { 
				return a.soc<b.soc; 
			}
		};*/
		struct contract_type_mapping {
			char type_code;
			bool friend operator<(
				const contract_type_mapping &a, 
				const contract_type_mapping &b
			) { 
				return a.type_code < b.type_code; 
			}
		};
		struct contract_type {
			uint32_t type_id;
			bool friend operator<(
				const contract_type &a,
				const contract_type &b
				) {
				return a.type_id < b.type_id;
			}
		};

		struct contract_term {
			uint32_t term_id;
			bool friend operator<(
				const contract_term &a,
				const contract_term &b
				) {
				return a.term_id < b.term_id;
			}
		};

/*		struct contract_type_term : contract_type, contract_term {
			bool friend operator<(
				const contract_type_term &a, 
				const contract_type_term &b
				) { 
				return less_<contract_type, contract_term >(a, b);
			}
		};*/

/*		struct plan_contract_term_type : contract_type_term, plan {
			bool friend operator<(
				const plan_contract_term_type &a, 
				const plan_contract_term_type &b
				) { 
				return less_<contract_type_term, plan>(a, b);
			}
		};*/

		struct bell_soc_compatibilty {
			soc from_soc;
			soc to_soc;

			friend bool operator<(
				const bell_soc_compatibilty &a, 
				const bell_soc_compatibilty &b
			) { 
				return a.from_soc < b.from_soc ? true :
					b.from_soc < b.from_soc ? false :
					a.to_soc < b.to_soc;
			}
		};

		struct bell_subscriber_soc : Subscriber, soc {
			friend bool operator<(
				const bell_subscriber_soc &a, 
				const bell_subscriber_soc &b
				) { 
				return less_<Subscriber, soc>(a, b);
			}
		};

		struct bell_subscriber_soc_hist : subscriber_hist, soc {
			bool friend operator<(
				const bell_subscriber_soc_hist &a, 
				const bell_subscriber_soc_hist &b
				) { 
				return less_<subscriber_hist, soc>(a, b);
			}
		};
		struct uom {
			char uom_code;
			bool friend operator<(const uom &a, const uom &b) {
				return a.uom_code < b.uom_code;
			}
		};
		struct bell_subscriber_option : subscriber_hist, soc ,uom {
			bool friend operator<(
				const bell_subscriber_option &a, 
				const bell_subscriber_option &b
			) { 
				return less_<subscriber_hist, soc, uom>(a, b);
			}
		};

		struct bell_subscriber_minimum_allowance : Subscriber,calltype_group {
			friend bool operator<(
				const bell_subscriber_minimum_allowance &a,
				const bell_subscriber_minimum_allowance &b
				) {
				return less_<Subscriber, calltype_group>(a, b);
			}
		};

		struct bell_account_minimum_allowance : public Account,calltype_group {
			friend bool operator<(
				const bell_account_minimum_allowance &a,
				const bell_account_minimum_allowance &b
				) {
				return less_<Account, calltype_group>(a, b);
			}
		};
		struct bell_account_usage_modifier : log, calltype_group
		{
			friend bool operator < (
				const bell_account_usage_modifier &a, 
				const bell_account_usage_modifier &b
			) { 
				return less_<log, calltype_group>(a, b);
			}
		};

		struct bell_subscriber_usage_modifier : Subscriber,log,calltype_group
		{
			friend bool operator < (
				const bell_subscriber_usage_modifier &a,
				const bell_subscriber_usage_modifier &b
				) {
				return less_<Subscriber,log,calltype_group>(a,b);
			}
		};

		struct bell_customer_override : Subscriber, log {
			friend bool operator<(
				const bell_customer_override &a, 
				const bell_customer_override &b
				) { 
				return less_<Subscriber, log>(a, b);
			}
		};
	}
}