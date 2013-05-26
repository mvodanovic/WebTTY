#ifndef WEBTTY_SOCKET_HELPER__H
#define WEBTTY_SOCKET_HELPER__H

#include <string>
#include <sys/socket.h>
#include <sys/un.h>

namespace WebTTY
{
	class SocketHelper
	{
		public:
			static char *read(int);
			static void write(int, char *);
			static void listen(int &, struct sockaddr_un &, std::string);
	};
}

#endif
