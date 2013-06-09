/// Standard library includes
#include <string>
#include <vector>
#include <sstream>
#include <utility>

/// Program-specific includes
#include "functions.h"

std::vector<std::string> WebTTY::explode(std::string const & s, char delim)
{
	std::vector<std::string> result;
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim); )
	{
		result.push_back(token);
	}

	return result;
}
