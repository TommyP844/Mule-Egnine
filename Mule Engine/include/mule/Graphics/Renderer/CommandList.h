#pragma once

#include "RenderCommand.h"

#include <vector>

namespace Mule
{
	class CommandList
	{
	public:
		CommandList() = default;
		~CommandList() = default;

		void AddCommand(const RenderCommand& command)
		{
			mCommands.emplace_back(command);
		}

		void Flush()
		{
			mCommands.clear();
		}

		const std::vector<RenderCommand>& GetCommands() const { return mCommands; }

	private:
		std::vector<RenderCommand> mCommands;
	};
}
