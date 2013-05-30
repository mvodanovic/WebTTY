#ifndef WEBTTY_LOGGER__H
#define WEBTTY_LOGGER__H

/// Standard library includes
#include <string>
#include <sstream>

namespace WebTTY
{
	class Logger
	{
		public:
			static void Die(std::string, int = 1);
			static void Log(std::string, int = 1);
			static void Log(long int, int = 1);
			static void Flush(void);

		protected:
		    static std::stringstream buffer;
	};
}

#endif
