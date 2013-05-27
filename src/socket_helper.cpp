#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <string>
#include <sys/un.h>

#include "socket_helper.h"
#include "logger.h"

char *WebTTY::SocketHelper::read(int socketFd)
{
	char *buffer = NULL;
	int length;
	int retRead;

	retRead = ::read(socketFd, &length, sizeof(length));
	if (retRead == -1) {
		Logger::Log(strerror(errno));
		return buffer;
	} else if (retRead == 0) {
		return buffer;
	}

	buffer = (char *) malloc(length);
	retRead = ::read(socketFd, buffer, length);
	if (retRead == -1) {
		free(buffer);
		buffer = NULL;
		Logger::Log(strerror(errno));
		return buffer;
	} else if (retRead != length) {
		free(buffer);
		buffer = NULL;
		Logger::Log("buffer read error");
		return buffer;
	}

	return buffer;
}

void WebTTY::SocketHelper::write(int socketFd, const char *buffer)
{
	if (buffer == NULL) {
		return;
	}

	int length = strlen(buffer) + 1;
	int retWrite;

	retWrite = ::write(socketFd, &length, sizeof(length));
	if (retWrite == -1) {
		Logger::Log(strerror(errno));
		return;
	}

	retWrite = ::write(socketFd, buffer, length);
	if (retWrite == -1) {
		Logger::Log(strerror(errno));
		return;
	}
}

void WebTTY::SocketHelper::listen(int &socketFd, std::string socketPath)
{
	struct sockaddr_un name;
	name.sun_family = AF_LOCAL;
	strcpy(name.sun_path, socketPath.c_str());

	/// Create the socket
	if ((socketFd = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1) {
		Logger::Die(strerror(errno));
	}

	/// Indicate that this is a server
	name.sun_family = AF_LOCAL;
	strcpy(name.sun_path, socketPath.c_str());
	if (bind(socketFd, (sockaddr *) &name, SUN_LEN(&name)) == -1) {
		close(socketFd);
		Logger::Die(strerror(errno));
	}

	/// Listen for connections
	if (::listen(socketFd, 5) == -1) {
		close(socketFd);
		Logger::Die(strerror(errno));
	}
}

void WebTTY::SocketHelper::connect(int &socketFd, std::string socketPath)
{
	struct sockaddr_un name;
	name.sun_family = AF_LOCAL;
	strcpy(name.sun_path, socketPath.c_str());

	/// Create the socket
	if ((socketFd = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1 ) {
		Logger::Die(strerror(errno));
	}

	/// Connect to socket
	if (::connect(socketFd, (sockaddr *) &name, SUN_LEN(&name)) == -1) {
		close(socketFd);
		Logger::Die(strerror(errno));
	}
}
