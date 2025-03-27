#pragma once

#include "Buffer.h"

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

namespace Mule
{
	static bool ReadFile(const fs::path& path, std::string& data)
	{
		std::ifstream file(path);
		if (!file.is_open())
			return false;

		std::string line;
		while (std::getline(file, line))
			data += line;

		file.close();
		return true;
	}

	static bool ReadFileBytes(const fs::path& path, Buffer& buffer)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file.is_open())
			return false;

		file.seekg(std::ios::end);
		size_t size = file.tellg();
		file.seekg(std::ios::beg);

		buffer.Allocate(size);
		file.read(buffer.As<char>(), size);

		file.close();
		return true;
	}
}