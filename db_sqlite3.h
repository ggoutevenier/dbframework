#pragma once
#include "interface.h"
#include <memory>
//#pragma comment(lib,"sqlite3")
struct sqlite3;
struct sqlite3_stmt;
namespace dk {
	namespace sqlite {
		std::unique_ptr<IConnection> make_connection(const std::string &db, int flags = 0);
	}
}
