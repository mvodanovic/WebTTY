/// Standard library includes
#include <string>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <cstring>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <csignal>

/// Program-specific includes
#include "session.h"
#include "logger.h"
#include "socket_helper.h"
#include "tty.h"

int WebTTY::Session::isSession = 0;
std::string WebTTY::Session::sessionHash;
WebTTY::Session *WebTTY::Session::instance = NULL;

WebTTY::Session *WebTTY::Session::getInstance(void)
{
	if (Session::instance == NULL) {
		new Session(Session::sessionHash);
	}

	return Session::instance;
}

void WebTTY::Session::Start()
{
	Session *session = Session::getInstance();
	delete session;

}

std::string WebTTY::Session::getSocketPath(std::string sessionID)
{
	std::stringstream oss;
	oss << "/tmp/webttyd." << sessionID << ".socket";
	return oss.str();
}

WebTTY::Session::Session(std::string sessionID)
{
	Session::instance = this;
	this->sessionID = sessionID;
	this->outputThreadID = 0;

	/// Listen for connections
	SocketHelper::listen(this->socketFd, Session::getSocketPath(Session::sessionHash));

	/// Accept client connection
	struct sockaddr_un clientName;
	socklen_t clientNameLength = 0;
	if ((this->clientSocketFd = accept(this->socketFd, (sockaddr *) &clientName, &clientNameLength)) == -1) {
		if (errno != EINTR) {
			Logger::Log(strerror(errno));
		}
		return;
	}

	/// Set I/O handlers in separate threads
	int terrno = pthread_create(&this->outputThreadID, NULL, output_handler_wrapper, NULL);
	if (terrno != 0) {
		close(this->socketFd);
		close(this->clientSocketFd);
		unlink(Session::getSocketPath(Session::sessionHash).c_str());
		if (terrno != EINTR) {
			Logger::Die(strerror(terrno));
		}
	}

	this->tty = new TTY("");

	/// Setup signal handlers
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &WebTTY::Session::reapChildThread;
	sigaction (SIGINT, &sa, NULL);
	sigaction (SIGTERM, &sa, NULL);

	this->handleInput();

}

WebTTY::Session::~Session()
{
	/// Reset signal handlers
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sigaction (SIGINT, &sa, NULL);
	sigaction (SIGTERM, &sa, NULL);

	close(this->socketFd);
	close(this->clientSocketFd);
	unlink(Session::getSocketPath(Session::sessionHash).c_str());
	delete this->tty;
}

void WebTTY::Session::handleInput(void)
{
	while (1) {
	char *buffer = SocketHelper::read(this->clientSocketFd);
	if (buffer != NULL) {
			this->tty->send(buffer);
		}
	}
	pthread_join(this->outputThreadID, NULL);
	return;
}

void WebTTY::Session::handleOutput(void)
{
	while (1) {
		sleep(1);
		std::string buffer = this->tty->receive();
		if (buffer.length() > 0) {
			SocketHelper::write(this->clientSocketFd, buffer.c_str());
		}
	}
	return;
}

void *output_handler_wrapper(void *p)
{
	WebTTY::Session::getInstance()->handleOutput();
	return NULL;
}

void WebTTY::Session::reapChildThread(int signal)
{
	switch (signal) {
		case SIGINT:
		case SIGTERM:
			Session *instance = Session::getInstance();
			if (instance == NULL) {
				break;
			}

			if (instance->outputThreadID == 0) {
				break;
			}

			pthread_kill(instance->outputThreadID, SIGINT);
			pthread_join(instance->outputThreadID, NULL);
			instance->outputThreadID = 0;
			break;
	}
}
