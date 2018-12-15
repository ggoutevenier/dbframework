#pragma once
#include "interface.h"
#include <memory>

namespace dk {
	namespace sqlite {
		std::unique_ptr<IConnection> make_connection(const std::string &db, int flags = 0);
	}
}
