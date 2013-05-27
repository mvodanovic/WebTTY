#ifndef WEBTTY_SOCKET_HELPER__H
#define WEBTTY_SOCKET_HELPER__H

#include <string>

namespace WebTTY
{
	class SocketHelper
	{
		public:
			static char *read(int);
			static void write(int, const char *);
			static void listen(int &, std::string);
			static void connect(int &, std::string);
	};
}

#endif
