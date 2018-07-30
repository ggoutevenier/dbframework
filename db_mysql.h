#pragma once
#include "interface.h"
#include <memory>

namespace dk {
	namespace mysql {
	std::unique_ptr<IConnection> make_connection(
					const std::string &schema,
					const std::string &db,
					const std::string &usr,
					const std::string &pwd);
	}
}
