#include "Core/StringUtil.h"

#include <algorithm>
#include <sstream>

std::string Trim(const std::string& str)
{
	size_t start = str.find_first_not_of(" \t\n\r\f\v"); // Find first non-whitespace character
	if (start == std::string::npos) return ""; // If the string is all whitespace, return empty

	size_t end = str.find_last_not_of(" \t\n\r\f\v"); // Find last non-whitespace character
	return str.substr(start, end - start + 1);
}

std::string ToLower(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return result;
}

std::string RemoveChars(const std::string& str, char c)
{
	std::string s = str;
	s.erase(std::remove(s.begin(), s.end(), c), s.end());
	return s;
}

std::vector<std::string> Split(const std::string& str, char delim)
{
	std::vector<std::string> tokens;

	std::stringstream ss(str);

	std::string item;

	while (std::getline(ss, item, delim))
	{
		tokens.push_back(item);
	}

	return tokens;
}
