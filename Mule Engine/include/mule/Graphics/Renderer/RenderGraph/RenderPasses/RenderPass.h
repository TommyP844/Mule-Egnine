#pragma once

#include "Ref.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/Renderer/RenderGraph/ResourceRegistry.h"
#include "Graphics/Renderer/RenderCommand.h"
#include "Graphics/Renderer/RenderGraph/ResourceBuilder.h"

#include <string>
#include <vector>
#include <map>

namespace Mule
{
	enum class ResourceUsage
	{
		Read,
		Write
	};

	enum class PassType
	{
		Compute,
		Graphics
	};

	class RenderPass
	{
	public:
		RenderPass(const std::string& name, PassType type);
		virtual ~RenderPass() = default;

		void AddPassDependency(const std::string& dependentPass);

		void SetFrameIndex(uint32_t frameIndex) { mFrameIndex = frameIndex; }
		void SetFenceHandle(ResourceHandle fenceHandle) { mFenceHandle = fenceHandle; }
		void SetCommandBufferHandle(ResourceHandle cmdHandle) { mCommandBufferHandle = cmdHandle; }

		uint32_t GetFrameIndex() const { return mFrameIndex; }
		const std::string& GetName() const { return mName; }
		ResourceHandle GetFenceHandle() const { return mFenceHandle; }
		ResourceHandle GetCommandBufferHandle() const { return mCommandBufferHandle; }
		const std::vector<std::string>& GetDependencies() const { return mDependencies; }
		const std::unordered_map<ResourceHandle, ResourceUsage>& GetResourceUsage() const { return mResourceUsage; }

		virtual void Setup(ResourceRegistry& registry) = 0;
		virtual void Render(Ref<CommandBuffer> cmd, const std::vector<RenderCommand>& commands, const ResourceRegistry& context) = 0;
		virtual void Resize(ResourceRegistry& registry, uint32_t width, uint32_t height) = 0;

	protected:
		std::unordered_map<ResourceHandle, ResourceUsage> mResourceUsage;

	private:
		const std::string mName;
		std::vector<std::string> mDependencies;
		uint32_t mFrameIndex = 0;
		PassType mPassType;

		ResourceHandle mFenceHandle;
		ResourceHandle mCommandBufferHandle;
	};
}