#include <memory>
#include <gtest/gtest.h>
#include "db_mem.h"
#include "metadata.h"

using namespace dk;
class RecordTest : public testing::Test {
	protected:
	virtual void SetUp() {
		dst =	mem::make_connection();
	}

  	std::unique_ptr<IConnection>  dst;
};

struct R {
	bool 		b;
	int16_t		i16;
	uint32_t 	ui32;
	int64_t 	i64;
	double		d;
	float		f;
	tm			time;
	std::string str;
	dec::decimal<2> dec;
	char		c10[10];
	char		c;
	friend bool operator==( R &a, R &b) {
		if(a.b!=b.b) return false;
		if(a.i16!=b.i16) return false;
		if(a.ui32!=b.ui32) return false;
		if(a.i64!=b.i64) return false;
		if(abs(a.d-b.d)>.00001) return false;
		if(abs(a.f-b.f)>.00001) return false;
		if(mktime(&a.time)-mktime(&b.time)>0) return false;
		if(a.str!=b.str) return false;
		if(a.dec!=b.dec) return false;
		if(memcmp(a.c10,&b.c10,sizeof(R::c10))!=0) return false;
		if(a.c!=b.c) return false;
		return true;
	}
};


TEST_F(RecordTest, Test1) {
	R a,b;
	a.b=true;
	a.i16 = -1;
	a.ui32 = 0xFFFFFFFF;
	a.i64 = 0xFFFFFFFFFFFFFFF;
	a.d = 1.277777777777777777777777777e128;
	a.f =1.444;
	a.str = "string";
	a.dec=10.22;
	memcpy(a.c10,"0123456789",sizeof(a.c10));
	a.c='C';
	time_t aa=1546213650;
	memcpy(&a.time,localtime(&aa),sizeof(tm));

	RecordBuilder record("");
	record.add(a,a.b,"b");
	record.add(a,a.i16,"i16");
	record.add(a,a.ui32,"ui32");
	record.add(a,a.i64,"i64");
	record.add(a,a.d,"d");
	record.add(a,a.f,"f");
	record.add(a,a.str,"str").setSize(6);
	record.add(a,a.dec,"dec");
	record.add(a,a.c10,"c10");
	record.add(a,a.c,"c");
	record.add(a,a.time,"time");

	auto stmt = dst->createStatement("");
	record.set(*stmt.get(), &a);
	stmt->executeUpdate();
	auto rset = stmt->executeQuery();
	record.get(*rset.get(), &b);

	EXPECT_TRUE(a==b);
}

