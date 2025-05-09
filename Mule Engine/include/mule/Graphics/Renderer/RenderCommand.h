#pragma once

#include "Graphics/API/Texture.h"
#include "Graphics/API/TextureCube.h"

#include "Graphics/Mesh.h"
#include "Graphics/Material.h"

#include <glm/glm.hpp>

#include <variant>

namespace Mule
{
	enum class RenderCommandType
	{
		None,
		Draw,
		DrawInstanced,
		DrawEnvironmentMap,
	};

	struct BaseCommand
	{
		BaseCommand() : Type(RenderCommandType::None) {}
		explicit BaseCommand(RenderCommandType type) : Type(type) {}

		const RenderCommandType Type = RenderCommandType::None;
	};

	struct DrawCommand : BaseCommand
	{
		DrawCommand() : BaseCommand(RenderCommandType::Draw) {}
		DrawCommand(const WeakRef<Mesh>& mesh, const WeakRef<Material>& material, const glm::mat4& modelMatrix)
			: BaseCommand(RenderCommandType::Draw), Mesh(mesh), Material(material), ModelMatrix(modelMatrix) {
		}

		WeakRef<Mesh> Mesh = nullptr;
		WeakRef<Material> Material = nullptr;
		glm::mat4 ModelMatrix = glm::mat4(1.0f);
	};

	struct DrawInstancedCommand : BaseCommand
	{
		DrawInstancedCommand() : BaseCommand(RenderCommandType::DrawInstanced) {}
		DrawInstancedCommand(const WeakRef<Mesh>& mesh, const WeakRef<Material>& material, const std::vector<glm::mat4>& modelMatrices)
			: BaseCommand(RenderCommandType::DrawInstanced), Mesh(mesh), Material(material), ModelMatrices(modelMatrices) {
		}

		WeakRef<Mesh> Mesh = nullptr;
		WeakRef<Material> Material = nullptr;
		std::vector<glm::mat4> ModelMatrices;
	};

	struct EnvironmentMapCommand : BaseCommand
	{
		EnvironmentMapCommand() : BaseCommand(RenderCommandType::DrawEnvironmentMap) {}
		EnvironmentMapCommand(const WeakRef<TextureCube>& cubeMap, const WeakRef<TextureCube>& irradianceMap, const WeakRef<TextureCube>& prefilterMap, float ambientStrength)
			: BaseCommand(RenderCommandType::DrawEnvironmentMap), CubeMap(cubeMap), IrradianceMap(irradianceMap), PrefilterMap(prefilterMap), AmbientStrength(ambientStrength) {
		}

		WeakRef<TextureCube> CubeMap = nullptr;
		WeakRef<TextureCube> IrradianceMap = nullptr;
		WeakRef<TextureCube> PrefilterMap = nullptr;
		float AmbientStrength = 1.f;
	};

	class RenderCommand
	{
	public:
		~RenderCommand() {}

		RenderCommand(const DrawCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const DrawInstancedCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const EnvironmentMapCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}

		template<typename T>
		const T& GetCommand() const
		{
			return std::get<T>(mCommand);
		}

		RenderCommandType GetType() const
		{
			return mType;
		}

	private:
		RenderCommandType mType;

		std::variant<std::monostate, 
			BaseCommand,
			DrawCommand,
			DrawInstancedCommand,
			EnvironmentMapCommand> mCommand;
	};
}
