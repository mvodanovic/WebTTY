/// Standard library includes
#include <unistd.h>
#include <cstdlib> /// !!!
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <cerrno>
#include <string>

/// Program-specific includes
#include "daemon.h"
#include "logger.h"

int server(int client_socket)
{
	while (1) {
		int length;

		if (read(client_socket, &length, sizeof(length)) == 0) {
			return 0;
		}

		char *text = (char *) malloc(length * sizeof(char));

		read(client_socket, text, length);
		if (text != NULL) {
			WebTTY::Logger::Log(text);
		}

		if (!strcmp(text, "quit")) {
			free(text);
			return 1;
		}

		free(text);
	}
}

const std::string WebTTY::Daemon::socketPath = "/tmp/webttyd.socket";

const std::string WebTTY::Daemon::getSocketPath(void)
{
	return Daemon::socketPath;
}

void WebTTY::Daemon::Start(void)
{
	/// Daemonize the process
	if (daemon(0, 1) == -1) {
		WebTTY::Logger::Die("Couldn't create a daemon");
	}

	/// Initialize the daemon
	Daemon *daemon = new Daemon();

	/// Cleanup on exit
	delete daemon;

	End();
}

WebTTY::Daemon::Daemon()
{
	/// Create the socket
	if ((this->socketFd = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1) {
		Logger::Die(strerror(errno));
	}
	/// Indicate that this is a server
	this->serverName.sun_family = AF_LOCAL;
	strcpy(this->serverName.sun_path, Daemon::getSocketPath().c_str());
	if (bind(this->socketFd, (sockaddr *) &this->serverName, SUN_LEN(&this->serverName)) == -1) {
		Logger::Die(strerror(errno));
	}
	/// Listen for connections
	if (listen(this->socketFd, 5) == -1) {
		Logger::Die(strerror(errno));
	}

	/// Handle new clients
	int quitMessageRecieved;
	do {
		int clientSocketFd = 0;
		struct sockaddr_un clientName;
		socklen_t clientNameLength = 0;

		if ((clientSocketFd = accept(this->socketFd, (sockaddr *) &clientName, &clientNameLength)) == -1) {
			Logger::Log(strerror(errno));
			continue;
		}
		quitMessageRecieved = server(clientSocketFd);
		close(clientSocketFd);

	} while (!quitMessageRecieved);
}

WebTTY::Daemon::~Daemon()
{
	close(this->socketFd);
	unlink(Daemon::getSocketPath().c_str());
	kill(getpgrp(), SIGTERM);
	/// while wait();
}

void WebTTY::Daemon::End(void)
{
	WebTTY::Logger::Log("Everything went ok");
	exit(EXIT_SUCCESS);
}
