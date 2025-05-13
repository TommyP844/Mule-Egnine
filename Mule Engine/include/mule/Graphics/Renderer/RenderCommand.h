#pragma once

#include "Graphics/API/Texture.h"
#include "Graphics/API/TextureCube.h"

#include "Graphics/Mesh.h"
#include "Graphics/Material.h"
#include "Graphics/Renderer/RenderGraph/ResourceHandle.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/Renderer/RenderGraph/ResourceRegistry.h"


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
		ClearFramebuffer,
		TransitionLayout,
		BeginRendering,
		EndRendering,
		BindGraphicsPipeline
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
			: 
			BaseCommand(RenderCommandType::DrawEnvironmentMap), CubeMap(cubeMap), IrradianceMap(irradianceMap), PrefilterMap(prefilterMap), AmbientStrength(ambientStrength) 
		{}

		WeakRef<TextureCube> CubeMap = nullptr;
		WeakRef<TextureCube> IrradianceMap = nullptr;
		WeakRef<TextureCube> PrefilterMap = nullptr;
		float AmbientStrength = 1.f;
	};

	struct ClearFramebufferCommand : BaseCommand
	{
		ClearFramebufferCommand() : BaseCommand(RenderCommandType::ClearFramebuffer) {}
		ClearFramebufferCommand(ResourceHandle framebufferHandle) 
			: 
			BaseCommand(RenderCommandType::ClearFramebuffer),
			FramebufferHandle(framebufferHandle)
		{}

		ResourceHandle FramebufferHandle;
	};

	struct TransitionLayoutCommand : BaseCommand
	{
		TransitionLayoutCommand() : BaseCommand(RenderCommandType::TransitionLayout) {}	
		
		TransitionLayoutCommand(ResourceHandle textureHandle, ImageLayout newLayout)
			:
			BaseCommand(RenderCommandType::TransitionLayout),
			TextureHandle(textureHandle),
			NewLayout(newLayout)
		{}

		ResourceHandle TextureHandle;
		ImageLayout NewLayout;
	};

	struct BeginRenderingCommandAttachment
	{
		ResourceHandle AttachmentHandle;
		bool ClearOnLoad = false;
		uint32_t index = 0;
	};

	struct BeginRenderingCommand : BaseCommand
	{
		BeginRenderingCommand() : BaseCommand(RenderCommandType::BeginRendering) {}
		BeginRenderingCommand(RegistryVariable width, RegistryVariable height, const std::vector<BeginRenderingCommandAttachment>& colorAttachments, BeginRenderingCommandAttachment depthAttachment)
			:
			BaseCommand(RenderCommandType::BeginRendering),
			Width(width),
			Height(height),
			ColorAttachments(colorAttachments),
			DepthAttachment(depthAttachment)
		{}

		RegistryVariable Width, Height;
		std::vector<BeginRenderingCommandAttachment> ColorAttachments;
		BeginRenderingCommandAttachment DepthAttachment;
	};

	struct EndRenderingCommand : BaseCommand
	{
		EndRenderingCommand() : BaseCommand(RenderCommandType::EndRendering) {}
	};

	struct BindGraphicsPipelineCommand : BaseCommand
	{
		BindGraphicsPipelineCommand() : BaseCommand(RenderCommandType::BindGraphicsPipeline) {}
		BindGraphicsPipelineCommand(WeakRef<GraphicsPipeline> pipeline, const std::vector<ResourceHandle>& shaderResourceGroups) 
			:
			BaseCommand(RenderCommandType::BindGraphicsPipeline),
			Pipeline(pipeline),
			ShaderResourceGroups(shaderResourceGroups)
		{}

		WeakRef<GraphicsPipeline> Pipeline;
		std::vector<ResourceHandle> ShaderResourceGroups;
	};

	class RenderCommand
	{
	public:
		~RenderCommand() {}

		RenderCommand(const DrawCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const DrawInstancedCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const EnvironmentMapCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const ClearFramebufferCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const TransitionLayoutCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const BeginRenderingCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const EndRenderingCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const BindGraphicsPipelineCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}

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
			EnvironmentMapCommand,
			ClearFramebufferCommand,
			TransitionLayoutCommand,
			BeginRenderingCommand,
			EndRenderingCommand,
			BindGraphicsPipelineCommand> mCommand;
	};
}
