#include "Physics/RigidBody3D.h"

#include <glm/gtx/quaternion.hpp>
#include <Jolt/Physics/Body/MotionProperties.h>

namespace Mule
{
	RigidBody3D::RigidBody3D(JPH::PhysicsSystem* system, JPH::BodyID id, JPH::BodyInterface& bodyInterface)
		:
		mID(id),
		mBodyInterface(bodyInterface),
		mSystem(system)
	{
	}

	void RigidBody3D::MoveKinematic(const glm::vec3& targetPosition, const glm::quat& targetRotation, float timeToMove)
	{
		mBodyInterface.MoveKinematic(mID, {targetPosition.x, targetPosition.y, targetPosition.z}, {targetRotation.x, targetRotation.y, targetRotation.z, targetRotation.w}, timeToMove);
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

	float RigidBody3D::GetMass() const
	{
		auto& lockInterface = mSystem->GetBodyLockInterface();
		JPH::BodyLockRead lock(lockInterface, mID);
		float mass = 0.f;

		if (lock.Succeeded())
		{
			const JPH::Body& body = lock.GetBody();
			float mass = body.GetMotionProperties()->GetInverseMass();
		}

		return 1.f / mass;
	}

	void RigidBody3D::SetPosition(const glm::vec3& position)
	{
		mBodyInterface.SetPosition(mID, {position.x, position.y, position.z}, JPH::EActivation::Activate);
	}

	void RigidBody3D::SetMass(float mass)
	{
		auto& lockInterface = mSystem->GetBodyLockInterface();
		JPH::BodyLockWrite lock(lockInterface, mID);

		mass = glm::clamp(mass, 0.f, FLT_MAX);
		if (mass > 0.f) mass = 1.f / mass;

		if (lock.Succeeded())
		{
			JPH::Body& body = lock.GetBody();
			body.GetMotionProperties()->SetInverseMass(mass);
		}
	}
}