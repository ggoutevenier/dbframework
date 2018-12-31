#include "db_loopback.h"
#include "interface.h"
#include <memory>
#include "column.h"
#include <gtest/gtest.h>

using namespace dk;
class TypeTest : public ::testing::Test {
	protected:
	virtual void SetUp() {
		dst =	loopback::make_connection();
		stmt = dst->createStatement("");
		rset = stmt->executeQuery();
	}

  // virtual void TearDown() {}

  	std::unique_ptr<IConnection>  dst;
  	std::unique_ptr<IStatement> stmt;
  	std::unique_ptr<IResultSet> rset;
};

TEST_F(TypeTest, CharArray) {
	Type<char[10]> type;

	char a[]="123456";
	char b[10];

	type.set(*stmt.get(), a, 1);
	type.get(*rset.get(), b, 1);

	EXPECT_STREQ(a,b);
}

TEST_F(TypeTest, CharArrayFull) {
	Type<char[6]> type;

	char a[]="123456\0";
	char b[7];

	type.set(*stmt.get(), a, 1);
	type.get(*rset.get(), b, 1);

	EXPECT_STREQ(a,b);
}

TEST_F(TypeTest, CharArrayUndersize) {
	Type<char[3]> type;

	char a[]="123456\0";
	char b[7];

	type.set(*stmt.get(), a, 1);
	type.get(*rset.get(), b, 1);
	a[3]=0;
	EXPECT_STREQ(a,b);
}

TEST_F(TypeTest, CharArrayTruncate) {
	Type<char[10]> type;
	size_t n;

	char a[]="123456";
	char b[10];

	n = strlen(a)-1;
	type.setSize(n);

	type.set(*stmt.get(), a, 1);
	type.get(*rset.get(), b, 1);

	std::string c = a;
	c.resize(n);
	EXPECT_STREQ(c.c_str(),b);
}

TEST_F(TypeTest, String) {
	Type<std::string> type;

	std::string a="123456";
	std::string b;

	type.set(*stmt.get(), &a, 1);
	type.get(*rset.get(), &b, 1);

	EXPECT_EQ(a,b);
}

TEST_F(TypeTest, StringUndersize) {
	Type<std::string> type;
	size_t n;

	std::string a="123456";
	n = a.length()-1;
	type.setSize(n);

	type.set(*stmt.get(), &a, 1);
	std::string b;
	type.get(*rset.get(), &b, 1);

	std::string c;
	c=a;
	c.resize(n);
	EXPECT_EQ(c,b);
}

TEST_F(TypeTest, Decimal) {
	Type<dec::decimal<2> > type;

	dec::decimal<2> a;
	a = 10.22;

	type.set(*stmt.get(),&a,1);
	dec::decimal<2> b;

	type.get(*rset.get(),&b,1);
	EXPECT_EQ(a,b);
}

TEST_F(TypeTest, int64) {
	Type<int64_t> type;

	int64_t a;
	a = 999999999999;

	type.set(*stmt.get(),&a,1);
	int64_t b;

	type.get(*rset.get(),&b,1);
	EXPECT_EQ(a,b);
}

TEST_F(TypeTest, uint32) {
	Type<uint32_t> type;

	uint32_t a;
	a = 0xFFFFFFFF;

	type.set(*stmt.get(),&a,1);
	uint32_t b;

	type.get(*rset.get(),&b,1);
	EXPECT_EQ(a,b);
}

TEST_F(TypeTest, uint16) {
	Type<uint32_t> type;

	uint32_t a;
	a = 0xFFFF;

	type.set(*stmt.get(),&a,1);
	uint32_t b;

	type.get(*rset.get(),&b,1);
	EXPECT_EQ(a,b);
}

TEST_F(TypeTest, tm) {
	Type<tm> type;

	time_t aa=1546213650;

	tm a;
	tm b;
	memcpy(&a,localtime(&aa),sizeof(tm));

	type.set(*stmt.get(),&a,1);

	type.get(*rset.get(),&b,1);
	time_t bb=mktime(&b);

	EXPECT_TRUE(abs(aa-bb)==0);
}

TEST_F(TypeTest, bool) {
	Type<bool> type;

	bool a=true;
	type.set(*stmt.get(),(const void*)&a,1);
	bool b;
	type.get(*rset.get(),(void*)&b,1);

	EXPECT_TRUE(a==b);
}

int test(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
