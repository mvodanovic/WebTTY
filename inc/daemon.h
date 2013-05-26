#ifndef WEBTTY_DAEMON__H
#define WEBTTY_DAEMON__H

/// Standard library includes
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <vector>

namespace WebTTY
{
	class Daemon
	{
		public:
			static void Start(void);
			Daemon(void);
			~Daemon(void);
			static const std::string getSocketPath(void);

		protected:
			static void End(void);
			int handleClient(int);

			int socketFd;
			struct sockaddr_un serverName;
			static const std::string socketPath;
			std::vector<pid_t> sessionList;
			int doCleanup;
	};
}

#endif
