/// Standard library includes
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

/// Program-specific includes
#include "logger.h"

std::stringstream WebTTY::Logger::buffer;

void WebTTY::Logger::Die(std::string message, int flush)
{
	Log(message, flush);
	exit(EXIT_FAILURE);
}

void WebTTY::Logger::Log(std::string message, int flush)
{
	Logger::buffer << message;
	if (flush > 0) {
	    Logger::Flush();
	}
}

void WebTTY::Logger::Log(long int number, int flush)
{
    Logger::buffer << number;
    if (flush > 0) {
        Logger::Flush();
    }
}

void WebTTY::Logger::Flush(void)
{
    std::cout << "webttyd (" << getpid() << ", " << pthread_self() << "): " << Logger::buffer.str() << std::endl;
    Logger::buffer.str("");
}
