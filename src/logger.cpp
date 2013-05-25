/// Standard library includes
#include <cstdlib>
#include <iostream>
#include <string>

/// Program-specific includes
#include "logger.h"

void WebTTY::Logger::Die(std::string message)
{
	Log(message);
	exit(EXIT_FAILURE);
}

void WebTTY::Logger::Log(std::string message)
{
	std::cout << "webttyd: " << message << std::endl;
}
