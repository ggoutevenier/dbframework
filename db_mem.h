#pragma once
#include "interface.h"
#include <memory>

namespace dk {
	namespace mem {
	std::unique_ptr<IConnection> make_connection();
	}
}
