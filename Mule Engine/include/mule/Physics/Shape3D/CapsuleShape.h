#pragma once

#include "Shape3D.h"

namespace Mule
{
	class CapsuleShape : public Shape3D
	{
	public:
		CapsuleShape(float radius, float halfHeight, const glm::vec3& offset = glm::vec3(0.f), bool trigger = false)
			:
			Shape3D(ShapeType::Capsule, offset, trigger),
			mRadius(radius),
			mHalfHeight(halfHeight)
		{ }

		float GetRadius() const { return mRadius; }
		float GetHalfHeight() const { return mHalfHeight; }

	private:
		float mRadius, mHalfHeight;
	};
}