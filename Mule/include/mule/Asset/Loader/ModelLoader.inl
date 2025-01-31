#pragma once

namespace Mule
{
	// TODO: pass in sub type so when we get to skinning we can get joint indicies
	template<typename T>
	inline const T* Mule::ModelLoader::GetBufferAsArray(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive, const std::string& name)
	{
		if (primitive.attributes.find(name) == primitive.attributes.end())
			return {};

		const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.at(name)];

		if (accessor.bufferView < 0)
			return {};

		const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

		const float* array = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

		return array;
	}
}
