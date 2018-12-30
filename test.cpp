#include "db_loopback.h"
#include "interface.h"
#include <memory>
#include "column.h"
#include <gtest/gtest.h>

using namespace dk;
class ColumnTest : public ::testing::Test {
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

TEST_F(ColumnTest, CharArray) {
	Column<char[10]> column(0,"col1",1);

	char a[]="123456";
	char b[10];

	column.set(*stmt.get(), a);
	column.get(*rset.get(), b);

	EXPECT_STREQ(a,b);
}

TEST_F(ColumnTest, CharArrayTruncate) {
	Column<char[10]> column(0,"col1",1);
	size_t n;

	char a[]="123456";
	char b[10];

	n = strlen(a)-1;
	column.setSize(n);

	column.set(*stmt.get(), a);
	column.get(*rset.get(), b);

	std::string c = a;
	c.resize(n);
	EXPECT_STREQ(c.c_str(),b);
}

TEST_F(ColumnTest, CharArrayUndersize) {
	Column<char[3]> column(0,"col1",1);
	size_t n = column.getSize();

	char a[]="123456";
	char b[10];


	column.set(*stmt.get(), a);
	column.get(*rset.get(), b);

	std::string c = a;
	c.resize(n);
	EXPECT_STREQ(c.c_str(),b);
}

TEST_F(ColumnTest, String) {
	Column<std::string> column(0,"col1",1);

	std::string a="123456";
	std::string b;

	column.set(*stmt.get(), &a);
	column.get(*rset.get(), &b);

	EXPECT_EQ(a,b);
}

TEST_F(ColumnTest, StringTruncated) {
	Column<std::string> column(0,"col1",1);
	size_t n;

	std::string a="123456";
	n = a.length()-1;
	column.setSize(n);

	column.set(*stmt.get(), &a);
	std::string b;
	column.get(*rset.get(), &b);

	std::string c;
	c=a;
	c.resize(n);
	EXPECT_EQ(c,b);
}


int test(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
