#include "stdafx.h"
#include "layout2.h"
#include "metadata.h"
namespace dk {
	template<class T>
	bool complete(Store &, void *data);

	/**** vendor ****/
	metadata<layout::vendor> ::metadata() : Record("acipe_vendor") {
		add(t.vendor_id, "vendor_id");
		add(t.vendor_desc, "vendor_desc");
	}
	/**** unit ****/
	metadata<layout::unit> ::metadata() : Record("acipe_unit") {
		add(t.unit_id, "unit_id");
		add(t.unit_desc, "unit_desc");
		add(t.factor, "conversion_factor");
	}
	/**** bundle_type ****/
	metadata<layout::bundle_type> ::metadata() : Record("acipe_bundle_types") {
		add(t.bundle_type_id, "bundle_type_id");
		add(t.bundle_type_desc, "bundle_type_desc");
		add(t.client_code, "client_code");
		add(t.shareable, "shareable_yn").boolVal("YN");
	}
	/***** calltype ****/
	metadata<layout::calltype> ::metadata() : Record("acipe_calltype") {
		add(t.calltype_id, "calltype_id");
		add(t.unit.unit_id, "unit_id");
		add/*ref*/(t.unit, "unit");
		add(t.calltype_desc, "calltype_desc");
		add(t.calltype_group.calltype_group_id, "calltype_group_id");
		add(t.bundle_type.bundle_type_id, "bundle_type_id");
		add/*ref*/(t.bundle_type, "bundle_type");
		add(t.seq,"sequence").startWith(10).increaseBy(5);
	}
}