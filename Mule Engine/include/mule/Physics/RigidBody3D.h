#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <glm/glm.hpp>

namespace Mule
{
	class RigidBody3D
	{
	public:
		RigidBody3D(JPH::PhysicsSystem* system ,JPH::BodyID id, JPH::BodyInterface& bodyInterface);

		// Action
		void MoveKinematic(const glm::vec3& targetPosition, const glm::quat& targetRotation, float timeToMove);

		// Getters
		glm::vec3 GetPosition() const;
		glm::vec3 GetRotation() const;
		float GetMass() const;

		// Setters
		void SetPosition(const glm::vec3& position);
		void SetMass(float mass);

	private:
		JPH::PhysicsSystem* mSystem;
		JPH::BodyID mID;
		JPH::BodyInterface& mBodyInterface;
	};
}