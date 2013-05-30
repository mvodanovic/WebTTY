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

/**
 *   Method for handling a client connection to the daemon
 *
 *   @param int the connected client's socket file descriptor
 *   @returns 0 if the daemon should accept new clients, 0 if the daemon should finish
 */
int WebTTY::Daemon::handleClient(int clientSocketFd)
{
	char *text = SocketHelper::read(clientSocketFd);

	/// Invalid input, nothing to do
	if (text == NULL) {
		return 0;
	}

	/// Quit message received
	if (!strcmp(text, "q")) {
		free(text);
		SocketHelper::write(clientSocketFd, "Quitting...");
		return 1;
	}

	/// Create new session message received
	if (!strcmp(text, "c")) {
		free(text);

		/// Receive session ID
		text = SocketHelper::read(clientSocketFd);

		/// Invalid input, nothing to do
		if (text == NULL || strlen(text) == 0) {
			free(text);
			Logger::Log("Invalid session ID given");
			return 0;
		}

		/// Fork new session process
		pid_t pid = fork();
		if (pid == -1) {
			Logger::Log("Cannot fork a new process for the session");
		} else if (pid == 0) {
			/// Session process (exit daemon & start session)
			Session::isSession = 1;
			Session::sessionHash = text;
			this->doCleanup = 0;
			free(text);
			SocketHelper::write(clientSocketFd, Session::getSocketPath(Session::sessionHash).c_str());
			return 1;
		}

		/// Register the session with the daemon
		this->sessionList.push_back(pid);
	}

	free(text);
	return 0;
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
	signal(SIGCHLD, SIG_IGN);
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
	/// Class parameter initialization
	this->doCleanup = 1;
	Daemon::instance = this;

	/// Setup child termination handlers
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &WebTTY::Daemon::reapChildren;
	sigaction (SIGCHLD, &sa, NULL);
	sigaction (SIGINT, &sa, NULL);
	sigaction (SIGTERM, &sa, NULL);

	/// Listen for connections
	SocketHelper::listen(this->socketFd, Daemon::getSocketPath());

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
	if (this->doCleanup != 0) {
		close(this->socketFd);
		unlink(Daemon::getSocketPath().c_str());
		Daemon::reapChildren(SIGINT);
	}

	Daemon::instance = NULL;
}

void WebTTY::Daemon::reapChildren(int signal)
{
	Daemon *instance = Daemon::getInstance();
	if (instance == NULL) {
		return;
	}

	switch (signal) {
		case SIGINT:
		case SIGTERM:
			for (std::vector<pid_t>::iterator i = instance->sessionList.begin(); i != instance->sessionList.end(); i++) {
				kill(*i, SIGINT);
			}
			/// !!! continue below
		case SIGCHLD:

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
