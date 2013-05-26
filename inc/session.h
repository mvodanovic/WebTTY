#ifndef WEBTTY_SESSION__H
#define WEBTTY_SESSION__H

/// Standard library includes
#include <string>
#include <unistd.h>

namespace WebTTY
{
	class Session
	{
		public:
		    static int isSession;
		    static std::string sessionHash;
			static pid_t Start();
			static std::string getSocketPath(std::string);
			Session(std::string);

		protected:
			std::string sessionID;
	};
}

#endif
