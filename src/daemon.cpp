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


const std::string WebTTY::Daemon::socketPath = "/tmp/webttyd.socket";
WebTTY::Daemon *WebTTY::Daemon::instance = NULL;

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

const std::string WebTTY::Daemon::getSocketPath(void)
{
	return Daemon::socketPath;
}

WebTTY::Daemon *WebTTY::Daemon::getInstance(void)
{
	if (Daemon::instance == NULL) {
		new Daemon();
	}

	return Daemon::instance;
}

void WebTTY::Daemon::Start(void)
{
	/// Daemonize the process
	if (daemon(0, 1) == -1) {
		WebTTY::Logger::Die("Couldn't create a daemon");
	}

	/// Initialize the daemon
	Daemon *daemon = Daemon::getInstance();

	/// Cleanup on exit
	delete daemon;
}

WebTTY::Daemon::Daemon()
{
	Daemon::instance = this;

	signal (SIGCHLD, WebTTY::Daemon::reapChildren);
	this->doCleanup = 1;

	/// Listen for connections
	SocketHelper::listen(this->socketFd, this->serverName, Daemon::getSocketPath());

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
	Daemon::instance = NULL;

	if (this->doCleanup == 0) {
		return;
	}
	close(this->socketFd);
	unlink(Daemon::getSocketPath().c_str());
	for(int i = 0; i <= this->sessionList.size(); i++) {
		kill(this->sessionList[i], SIGTERM);
	}
}

void WebTTY::Daemon::reapChildren(int signal)
{
	switch (signal) {
		case SIGCHLD:
			Daemon *instance = Daemon::getInstance();
			if (instance == NULL) {
				break;
			}
			while (1) {
				pid_t pid = waitpid(-1, NULL, WNOHANG);
				if (pid <= 0) {
					break;
				}
				std::vector<pid_t>::iterator i = find(instance->sessionList.begin(), instance->sessionList.end(), pid);
				instance->sessionList.erase(i);
			}
			break;
	}
}
