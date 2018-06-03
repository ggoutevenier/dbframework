#include "stdafx.h"
#include "db_sqlite3.h"
#include <sqlite3.h>
#include "layout2.h"
#include "store.h"
#include <iostream>

using namespace dk;
void source_test() {
	std::shared_ptr<IConnection> conn=
		std::make_shared<sqlite::Connection>("X://jitre//reference.db", SQLITE_OPEN_READONLY);

	Store store(conn);
	auto o = store.projection<layout::calltype>();
}
int main(int argc, char *argv[]) {
	try {
		source_test();
	}
	catch (std::exception &e) {
		std::string str = e.what();
		std::cout << e.what() << std::endl;

		std::cout << "Exiting with return 1" << std::endl;

		return 1;
	}
	return 0;
}

