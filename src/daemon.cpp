/// Standard library includes
#include <unistd.h>
#include <cstdlib> /// !!!
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <cstring>
#include <cerrno>
#include <string>
#include <csignal>
#include <vector>
#include <algorithm>

/// Program-specific includes
#include "daemon.h"
#include "logger.h"
#include "socket_helper.h"
#include "session.h"

int WebTTY::Daemon::handleClient(int clientSocketFd)
{
	while (1) {
		char *text = WebTTY::SocketHelper::read(clientSocketFd);
		if (text == NULL) {
			return 0;
		}

		pid_t pid = fork();
		if (pid == -1) {
			return 0;
		} else if (pid == 0) {
			Logger::Log(text);
			Session::isSession = 1;
			Session::sessionHash = text;
			this->doCleanup = 0;
			return 1;
		}

		this->sessionList.push_back(pid);

		if (!strcmp(text, "q")) {
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
}

WebTTY::Daemon::Daemon()
{
	this->doCleanup = 1;

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
		quitMessageRecieved = handleClient(clientSocketFd);
		close(clientSocketFd);

	} while (!quitMessageRecieved);
}

WebTTY::Daemon::~Daemon()
{
	if (this->doCleanup == 0) {
		return;
	}
	close(this->socketFd);
	unlink(Daemon::getSocketPath().c_str());
	for(int i = 0; i <= this->sessionList.size(); i++) {
		kill(this->sessionList[i], SIGTERM);
	}
	while (this->sessionList.size() > 0) {
		pid_t pid = wait(NULL);
		if (pid == -1) {
			if (errno == ECHILD) {
				break;
			} else {
				Logger::Log(strerror(errno));
				break;
			}
		}
		std::vector<pid_t>::iterator i = find(this->sessionList.begin(), this->sessionList.end(), pid);
		this->sessionList.erase(i);
	}
}
