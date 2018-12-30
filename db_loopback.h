#pragma once
#include "interface.h"
#include <memory>

namespace dk {
	namespace loopback {
	std::unique_ptr<IConnection> make_connection();
	}
}
