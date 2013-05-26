#ifndef WEBTTY_SOCKET_HELPER__H
#define WEBTTY_SOCKET_HELPER__H

namespace WebTTY
{
	class SocketHelper
	{
		public:
			static char *read(int);
			static void write(int, char *);
	};
}

#endif
