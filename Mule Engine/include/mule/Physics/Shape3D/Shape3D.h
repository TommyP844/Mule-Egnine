#pragma once

#include <glm/glm.hpp>

namespace Mule
{
	enum class ShapeType
	{
		Sphere,
		Box,
		Convex,
		HeightField,
		Capsule
	};

	class Shape3D
	{
	public:
		Shape3D() = default;
		Shape3D(ShapeType type, const glm::vec3& offset = glm::vec3(0.f), bool trigger = false)
			:
			mType(type),
			mOffset(offset),
			mTrigger(trigger)
		{
		}

		bool IsTrigger() const { return mTrigger; }
		const glm::vec3& GetOffset() const { return mOffset; }
		ShapeType GetType() const { return mType; }

	private:
		bool mTrigger;
		glm::vec3 mOffset;
		ShapeType mType;
	};
}