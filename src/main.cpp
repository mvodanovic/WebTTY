/// Standard library includes
#include <cstdlib>
#include <sys/socket.h> /// !!!
#include <sys/un.h> /// !!!
#include <cstring> /// !!!
#include <unistd.h> /// !!!

/// Program-specific includes
#include "daemon.h"

void write_text(int socket_fd, const char *text)
{
	int length = strlen(text) + 1;
	write(socket_fd, &length, sizeof(length));
	write(socket_fd, text, length);
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		WebTTY::Daemon::Start();
	} else {
		struct sockaddr_un name;

		int socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
		name.sun_family = AF_LOCAL;
		strcpy(name.sun_path, WebTTY::Daemon::getSocketPath().c_str());
		connect(socket_fd, (sockaddr *) &name, SUN_LEN(&name));
		write_text(socket_fd, argv[1]);
		close(socket_fd);
	}
	exit(EXIT_SUCCESS);
}
