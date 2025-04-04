#pragma once

#include "Shape3D.h"

namespace Mule
{
	class PlaneShape : public Shape3D
	{
	public:
		PlaneShape(const glm::vec4& plane, bool trigger = false)
			:
			Shape3D(ShapeType::Plane, glm::vec3(), trigger),
			mPlane(plane)
		{}

		const glm::vec4& GetPlane() const { return mPlane; }
	private:
		glm::vec4 mPlane;
	};
}
