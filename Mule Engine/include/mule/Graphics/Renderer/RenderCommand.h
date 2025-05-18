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
		BindGraphicsPipeline,
		BindComputePipeline,
		DrawDirectionalLight,
		DrawPointLight,
		DrawSpotLight,
		DrawSkyBox,
		ClearRenderTarget
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
		BeginRenderingCommand(const std::vector<BeginRenderingCommandAttachment>& colorAttachments, BeginRenderingCommandAttachment depthAttachment)
			:
			BaseCommand(RenderCommandType::BeginRendering),
			ColorAttachments(colorAttachments),
			DepthAttachment(depthAttachment)
		{}

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

	struct BindComputePipelineCommand : BaseCommand
	{
		BindComputePipelineCommand() : BaseCommand(RenderCommandType::BindComputePipeline) {}
		BindComputePipelineCommand(WeakRef<ComputePipeline> pipeline, const std::vector<ResourceHandle>& shaderResourceGroups) 
			: 
			BaseCommand(RenderCommandType::BindComputePipeline),
			Pipeline(pipeline),
			ShaderResourceGroups(shaderResourceGroups)
		{}

		WeakRef<ComputePipeline> Pipeline;
		std::vector<ResourceHandle> ShaderResourceGroups;
	};


	struct DrawDirectionalLightCommand : BaseCommand
	{
		DrawDirectionalLightCommand() : BaseCommand(RenderCommandType::DrawDirectionalLight) {}
		DrawDirectionalLightCommand(const glm::vec3& direction, const glm::vec3& color, float intensity)
			:
			BaseCommand(RenderCommandType::DrawDirectionalLight),
			Direction(direction),
			Color(color),
			Intensity(intensity)
		{}

		glm::vec3 Direction;
		glm::vec3 Color;
		float Intensity;
	};

	struct DrawPointLightCommand : BaseCommand
	{
		DrawPointLightCommand() : BaseCommand(RenderCommandType::DrawPointLight) {}
		DrawPointLightCommand(const glm::vec3& position, const glm::vec3& color, float intensity)
			:
			BaseCommand(RenderCommandType::DrawPointLight),
			Position(position),
			Color(color),
			Intensity(intensity)

		{}

		glm::vec3 Position;
		glm::vec3 Color;
		float Intensity;
	};

	struct DrawSpotLightCommand : BaseCommand
	{
		DrawSpotLightCommand() : BaseCommand(RenderCommandType::DrawSpotLight) {}
		DrawSpotLightCommand(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, float intensity, float halfAngle, float fallOff)
			:
			BaseCommand(RenderCommandType::DrawSpotLight),
			Position(position),
			Direction(direction),
			Color(color),
			Intensity(intensity),
			HalfAngle(halfAngle),
			FallOff(fallOff)
		{}

		glm::vec3 Position;
		glm::vec3 Direction;
		glm::vec3 Color;
		float Intensity;
		float HalfAngle;
		float FallOff;
	};

	struct DrawSkyboxCommand : BaseCommand
	{
		DrawSkyboxCommand() : BaseCommand(RenderCommandType::DrawSkyBox) {}
		DrawSkyboxCommand(WeakRef<Mesh> mesh, WeakRef<TextureCube> skyBox, WeakRef<TextureCube> diffuseIBL, WeakRef<TextureCube> prefilterIBL, WeakRef<Texture2D> brdf)
			:
			BaseCommand(RenderCommandType::DrawSkyBox),
			CubeMesh(mesh),
			SkyBox(skyBox),
			DiffuseIBL(diffuseIBL),
			PreFilterIBL(prefilterIBL),
			BRDF(brdf)
		{}

		WeakRef<Mesh> CubeMesh;
		WeakRef<TextureCube> SkyBox;
		WeakRef<TextureCube> DiffuseIBL;
		WeakRef<TextureCube> PreFilterIBL;
		WeakRef<Texture2D> BRDF;
	};

	struct ClearRenderTargetCommand : BaseCommand
	{
		ClearRenderTargetCommand() : BaseCommand(RenderCommandType::ClearRenderTarget) {}
		ClearRenderTargetCommand(ResourceHandle target)
			: 
			BaseCommand(RenderCommandType::ClearRenderTarget),
			ClearTarget(target)
		{}

		ResourceHandle ClearTarget;
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
		RenderCommand(const BindComputePipelineCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const DrawDirectionalLightCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const DrawPointLightCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const DrawSpotLightCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const DrawSkyboxCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}
		RenderCommand(const ClearRenderTargetCommand& cmd) : mCommand(cmd), mType(cmd.Type) {}

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
			BindGraphicsPipelineCommand,
			BindComputePipelineCommand,
			DrawDirectionalLightCommand,
			DrawSpotLightCommand,
			DrawPointLightCommand,
			DrawSkyboxCommand,
			ClearRenderTargetCommand> mCommand;
	};
}
