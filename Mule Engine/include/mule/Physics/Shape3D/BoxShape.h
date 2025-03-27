#pragma once

#include "Shape3D.h"

namespace Mule
{
	class BoxShape : public Shape3D
	{
	public:
		BoxShape(const glm::vec3& extents, const glm::vec3& offset, bool trigger = false)
			:
			mExtents(extents),
			Shape3D(ShapeType::Box, offset, trigger)
		{}

		const glm::vec3& GetExtents() const { return mExtents; }

	private:
		glm::vec3 mExtents;
	};
}