#include "Graphics/UI/Rml/RenderInterface.h"

namespace Mule::UI
{
	Rml::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
	{
		return Rml::CompiledGeometryHandle();
	}

	void RmlRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture)
	{
	}

	void RmlRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
	{
	}

	Rml::TextureHandle RmlRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
	{
		return Rml::TextureHandle();
	}

	Rml::TextureHandle RmlRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
	{
		return Rml::TextureHandle();
	}

	void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture)
	{
	}

	void RmlRenderInterface::EnableScissorRegion(bool enable)
	{
	}

	void RmlRenderInterface::SetScissorRegion(Rml::Rectanglei region)
	{
	}
}