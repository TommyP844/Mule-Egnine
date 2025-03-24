#include "Physics/RigidBody3D.h"

#include <glm/gtx/quaternion.hpp>

namespace Mule
{
	RigidBody3D::RigidBody3D(JPH::BodyID id, JPH::BodyInterface& bodyInterface)
		:
		mID(id),
		mBodyInterface(bodyInterface)
	{
	}
	
	glm::vec3 RigidBody3D::GetPosition() const
	{
		auto pos = mBodyInterface.GetPosition(mID);
		return glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
	}

	glm::vec3 RigidBody3D::GetRotation() const
	{
		auto q = mBodyInterface.GetRotation(mID);
		glm::vec3 rot = glm::degrees(glm::eulerAngles(glm::quat(q.GetX(), q.GetY(), q.GetZ(), q.GetW())));
		return rot;
	}
}