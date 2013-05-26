#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>

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

void WebTTY::SocketHelper::write(int socketFd, char *buffer)
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
