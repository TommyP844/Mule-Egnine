#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <glm/glm.hpp>

namespace Mule
{
	class RigidBody3D
	{
	public:
		RigidBody3D(JPH::BodyID id, JPH::BodyInterface& bodyInterface);

		glm::vec3 GetPosition() const;
		glm::vec3 GetRotation() const;

	private:
		JPH::BodyID mID;
		JPH::BodyInterface& mBodyInterface;
	};
}