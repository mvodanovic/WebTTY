#ifndef WEBTTY_TTY__H
#define WEBTTY_TTY__H

#include <string>
#include <vector>
#include <libssh/libssh.h>

namespace WebTTY
{
	class TTY
	{
		public:
			TTY(std::vector<std::string> &);
			~TTY();
			void send(std::string);
			std::string receive(void);

		protected:
			ssh_session session;
			ssh_channel channel;
			int verbosity;
			std::string host;
			int port;
			std::string authType;
			std::string password;
			std::string term;
			int width;
			int height;
			int active;
			std::vector<std::string> params;

			int verifyKnownHost(void);
			std::string getParam(std::string);
			void initParams(void);
	};
}

#endif
