#pragma once

#include <string>
#include <vector>

std::string Trim(const std::string& str);

std::string ToLower(const std::string& str);

std::string RemoveChars(const std::string& str, char c);

std::vector<std::string> Split(const std::string& str, char delim);
