/// Standard library includes
#include <cstdlib>
#include <cstring>
#include <iostream> /// !!!

/// Program-specific includes
#include "daemon.h"
#include "socket_helper.h"
#include "session.h"


int main(int argc, char **argv)
{
	if (argc <= 1) {
		/// Server / daemon
		WebTTY::Daemon::Start();
		if (WebTTY::Session::isSession == 1) {
			WebTTY::Session::Start();
		}
	} else {
		/// CLI client
		if (!strcmp(argv[1], "q")) {
			/// Send quit signal
			int socketFd;
			WebTTY::SocketHelper::connect(socketFd, WebTTY::Daemon::getSocketPath());
			WebTTY::SocketHelper::write(socketFd, "q");
			char *message = WebTTY::SocketHelper::read(socketFd);
			if (message != NULL) {
				std::cout << message << std::endl;
				free(message);
			}
			close(socketFd);
		} else if (!strcmp(argv[1], "c") && argc >= 3) {
			/// Send create new session signal
			int socketFd;
			WebTTY::SocketHelper::connect(socketFd, WebTTY::Daemon::getSocketPath());
			WebTTY::SocketHelper::write(socketFd, "c");
			WebTTY::SocketHelper::write(socketFd, argv[2]);
			char *message = WebTTY::SocketHelper::read(socketFd);
			if (message != NULL) {
				std::cout << message << std::endl;
				free(message);
			}
			close(socketFd);
		} else if (!strcmp(argv[1], "w") && argc >= 4) {
			/// Write data to session
			int socketFd;
			WebTTY::SocketHelper::connect(socketFd, WebTTY::Session::getSocketPath(argv[2]));
			WebTTY::SocketHelper::write(socketFd, argv[3]);
			close(socketFd);
		} else if (!strcmp(argv[1], "r") && argc >= 3) {
			/// Read data from session
			int socketFd;
			WebTTY::SocketHelper::connect(socketFd, WebTTY::Session::getSocketPath(argv[2]));
			char *message = WebTTY::SocketHelper::read(socketFd);
			if (message != NULL) {
				std::cout << message << std::endl;
				free(message);
			}
			close(socketFd);
		} else {
			/// Invalid usage
		}
	}

	exit(EXIT_SUCCESS);
}
