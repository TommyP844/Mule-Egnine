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
		std::ifstream file(path, std::ios::binary | std::ios::ate); // open at end
		if (!file.is_open())
			return false;

		std::streamsize size = file.tellg();
		if (size <= 0)
			return false;

		file.seekg(0, std::ios::beg); // go back to start

		buffer.Allocate(static_cast<size_t>(size));
		if (!file.read(buffer.As<char>(), size))
			return false;

		file.close();
		return true;
	}
}