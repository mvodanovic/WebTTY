/// Standard library includes
#include <string>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <cstring>

/// Program-specific includes
#include "session.h"
#include "logger.h"

int WebTTY::Session::isSession = 0;
std::string WebTTY::Session::sessionHash;

pid_t WebTTY::Session::Start()
{
	pid_t pid = fork();

	if (pid == -1) {
		Logger::Log(strerror(errno));
		return pid;
	} else if (pid > 0) {
		return pid;
	}

	Session *session = new Session(Session::sessionHash);
	delete session;

	return pid;
}

std::string WebTTY::Session::getSocketPath(std::string sessionID)
{
	std::stringstream oss;
	oss << "/tmp/webttyd." << sessionID << ".socket";
	return oss.str();
}

WebTTY::Session::Session(std::string sessionID)
{
	this->sessionID = sessionID;
	Logger::Log(Session::getSocketPath(Session::sessionHash));
}
