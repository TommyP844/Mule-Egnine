
#include "Asset/Loader/ModelLoader.h"

#define TINYGLTF_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <spdlog/spdlog.h>

namespace Mule
{
	Ref<Model> ModelLoader::LoadText(const fs::path& filepath)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath.string());

		if (!warn.empty())
		{
			SPDLOG_WARN("TING-GLTF Warning: {}", warn);
		}

		if (!err.empty())
		{
			SPDLOG_ERROR("TING-GLTF Error: {}", err);
		}

		if (!ret)
		{
			SPDLOG_ERROR("Failed to parse file: {}", filepath.string());
			return nullptr;
		}

		

		return Ref<Model>::Make();
	}

	void ModelLoader::SaveText(Ref<Model> asset)
	{
	}

	Ref<Model> ModelLoader::LoadBinary(const fs::path& filepath)
	{
		return Ref<Model>();
	}

	void ModelLoader::SaveBinary(Ref<Model> asset)
	{
	}
}