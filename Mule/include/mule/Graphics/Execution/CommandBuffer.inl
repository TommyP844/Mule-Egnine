
namespace Mule
{
	template<typename T>
	inline void Mule::CommandBuffer::SetPushConstants(WeakRef<GraphicsShader> shader, ShaderStage stage, const T& value, uint32_t offset)
	{
		vkCmdPushConstants(mCommandBuffer, shader->GetPipelineLayout(), (VkShaderStageFlags)stage, offset, sizeof(T), &value);
	}
}