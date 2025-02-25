#pragma once

#include "Resource.h"

#include "Ref.h"

#include <string>
#include <vector>
#include <functional>

namespace Mule::RenderGraph
{
	struct PassContext
	{

	};

	class RenderGraph
	{
	public:
		RenderGraph();
		~RenderGraph();

		void AddResource(const std::string& name, Ref<IResource> resource);
		void AddPass(const std::string& name, const std::vector<std::string>& inputs, const std::vector<std::string>& outputs, std::function<void(PassContext&)> func);
		void Compile();
		void Execute();

		bool IsValid() const { return mIsValid; }

	private:
		struct RenderPass;
		std::unordered_map<std::string, Ref<IResource>> mResources;
		std::unordered_map<std::string, RenderPass> mPassesToCompile;
		bool mIsValid;

		struct RenderPass
		{
			std::vector<std::string> Inputs;
			std::vector<std::string> Outputs;
			std::function<void(PassContext&)> CallBack;
		};
	};
}