#include "Rendering/RenderPass.h"

Mule::RenderPass::~RenderPass()
{
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
}
