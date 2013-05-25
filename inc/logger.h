#ifndef WEBTTY_LOGGER__H
#define WEBTTY_LOGGER__H

/// Standard library includes
#include <string>

namespace WebTTY
{
	class Logger
	{
		public:
			static void Die(std::string);
			static void Log(std::string);
	};
}

#endif
