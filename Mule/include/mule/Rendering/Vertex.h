#pragma once

#include <glm/glm.hpp>

#include <bgfx/bgfx.h>

namespace Mule
{
	struct StaticVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec2 UV;
		glm::vec4 Color;

		static bgfx::VertexLayout CreateLayout() 
		{
			bgfx::VertexLayout layout;

			layout
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
				.end();

			return layout;
		}
	};
}
