#pragma once

// Engine
#include "WeakRef.h"
#include "Graphics/VertexLayout.h"
#include "Graphics/RenderTypes.h"
#include "Asset/Asset.h"
#include "IVulkanShader.h"

#include <Volk/volk.h>

// STD
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

namespace Mule
{

	class GraphicsShader : public Asset<AssetType::Shader>, public IVulkanShader
	{
	public:
		GraphicsShader(WeakRef<GraphicsContext> context, const fs::path& filepath);
		~GraphicsShader();

		void Reload();

		const std::pair<uint32_t, uint32_t>& GetPushConstantRange(ShaderStage stage);

		bool IsValid() const { return mIsValid; }
		const std::vector<uint32_t>& AttachmentLocations() const { return mOptions.AttachmentLocations; }
	private:
		bool mIsValid;
		WeakRef<GraphicsContext> mContext;		
	};
}
