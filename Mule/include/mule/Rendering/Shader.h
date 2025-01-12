#pragma once

#include <bgfx/bgfx.h>

namespace Mule
{
	class Shader
	{
	public:
		Shader();
	private:
		bgfx::ShaderHandle mHandle;
	};
}