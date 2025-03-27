#pragma once

#include "Shape3D.h"

namespace Mule
{
	class SphereShape : public Shape3D
	{
	public:
		SphereShape(float radius, const glm::vec3& offset, bool trigger = false)
			:
			mRadius(radius),
			Shape3D(ShapeType::Sphere, offset, trigger)
		{ }

		float GetRadius() const { return mRadius; }

	private:
		float mRadius;
	};
}