#ifndef WEBTTY_SESSION__H
#define WEBTTY_SESSION__H

/// Standard library includes
#include <string>
#include <unistd.h>
#include <pthread.h>
#include <sstream>
#include "tty.h"


void *output_handler_wrapper(void *p);

namespace WebTTY
{
	class Session
	{
		public:
			static int isSession;
			static std::string sessionHash;

			static void Start();
			static std::string getSocketPath(std::string);
			Session(std::string);
			~Session(void);
			static Session *getInstance(void);
			friend void *::output_handler_wrapper(void *p);
			static void reapChildThread(int);

		protected:
			std::string sessionID;
			int socketFd;
			int clientSocketFd;
			static Session *instance;
			pthread_t outputThreadID;
			std::stringstream buffer;
			TTY *tty;

			void handleInput(void);
			void handleOutput(void);
	};
}

#endif
