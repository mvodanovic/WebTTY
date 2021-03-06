/// Standard library includes
#include <string>
#include <vector>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <cstring>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <csignal>
#include <cstdlib>

/// Program-specific includes
#include "session.h"
#include "logger.h"
#include "socket_helper.h"
#include "tty.h"
#include "functions.h"

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

	Session *session;
	try {
		session = Session::getInstance();
	} catch (const char *msg) {
		Logger::Log(msg);
	}
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
	this->socketFd = 0;
	this->clientSocketFd = 0;
	this->tty = NULL;
	this->isCloseInProgress = 0;

	/// Listen for connections
	try {
		SocketHelper::listen(this->socketFd, Session::getSocketPath(Session::sessionHash));
	} catch (const char *msg) {
		this->close();
		throw msg;
	}

	/// Accept client connection
	struct sockaddr_un clientName;
	socklen_t clientNameLength = 0;
	if ((this->clientSocketFd = accept(this->socketFd, (sockaddr *) &clientName, &clientNameLength)) == -1) {
		if (errno != EINTR) {
			Logger::Log(strerror(errno));
		}
		this->close();
		throw "Client socket accept failed";
	}

	/// Receive TTY parameters
	char *buffer = SocketHelper::read(this->clientSocketFd);
	if (buffer == NULL) {
		buffer = (char *) malloc(sizeof(char));
		*buffer = '\0';
	}
	std::vector<std::string> TTYParams = explode(buffer, ';');
	free(buffer);

	try {
		this->tty = new TTY(TTYParams);
	} catch (const char *msg) {
		this->close();
		throw msg;
	}

	/// Set I/O handlers in separate threads
	int terrno = pthread_create(&this->outputThreadID, NULL, output_handler_wrapper, NULL);
	if (terrno != 0) {
		if (terrno != EINTR) {
			Logger::Log(strerror(terrno));
		}
		this->close();
		throw "Thread create failed";
	}

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
	this->close();
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

void WebTTY::Session::close(void)
{
	if (this->isCloseInProgress == 1) {
		return;
	}
	this->isCloseInProgress = 1;

	/// Reset signal handlers
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	if (this->socketFd != 0) {
		::close(this->socketFd);
		this->socketFd = 0;
	}

	if (this->clientSocketFd != 0) {
		::close(this->clientSocketFd);
		this->clientSocketFd = 0;
	}
	unlink(Session::getSocketPath(Session::sessionHash).c_str());

	if (this->tty != NULL) {
		delete this->tty;
		this->tty = NULL;
	}

	this->isCloseInProgress = 0;
}
