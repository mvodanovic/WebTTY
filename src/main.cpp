/// Standard library includes
#include <cstdlib>
#include <sys/socket.h> /// !!!
#include <sys/un.h> /// !!!
#include <cstring> /// !!!
#include <unistd.h> /// !!!

/// Program-specific includes
#include "daemon.h"
#include "socket_helper.h"
#include "session.h"


int main(int argc, char **argv)
{
	if (argc <= 1) {
		WebTTY::Daemon::Start();
		if (WebTTY::Session::isSession == 1) {
		    WebTTY::Session::Start();
		}
	} else {
		struct sockaddr_un name;

		int socketFd = socket(PF_LOCAL, SOCK_STREAM, 0);
		name.sun_family = AF_LOCAL;
		strcpy(name.sun_path, WebTTY::Daemon::getSocketPath().c_str());
		connect(socketFd, (sockaddr *) &name, SUN_LEN(&name));
		WebTTY::SocketHelper::write(socketFd, argv[1]);
		close(socketFd);
	}
	exit(EXIT_SUCCESS);
}
