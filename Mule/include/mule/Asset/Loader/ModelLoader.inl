#pragma once

namespace Mule
{
	// TODO: pass in sub type so when we get to skinning we can get joint indicies
	template<typename T>
	inline std::vector<T> Mule::ModelLoader::GetBufferAsVector(const tinygltf::Model& gltfModel, const tinygltf::Primitive& primitive, const std::string& name)
	{
		if (primitive.attributes.find(name) == primitive.attributes.end())
			return {};

		const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.attributes.at(name)];
		const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

		size_t size = accessor.count / sizeof(T);
		std::vector<T> list(size);

		const float* array = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

		memcpy(list.data(), &array[0], size * sizeof(float));

		return list;
	}
}
