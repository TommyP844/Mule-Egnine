#pragma once

#include "ECS/Guid.h"
#include "WeakRef.h"
#include "Ref.h"
#include "CollisionLayer.h"
#include "Shape3D/Shape3D.h"
#include "KinematicContactListener.h"

// Constraints
#include "Constraint/RotationConstraint.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <map>

namespace Mule
{
	enum BodyType : uint32_t
	{
		Dynamic,
		Static,
		Kinematic
	};

	struct RigidBody3DInfo
	{
		Guid Guid;
		glm::vec3 Position;
		glm::quat Orientation;
		BodyType Type;
		uint16_t CollisionLayers;
		float Mass;
		Ref<Shape3D> Shape;
	};

	class PhysicsContext
	{
	public:
		PhysicsContext();
		~PhysicsContext();

		void Init();
		void Shutdown();

		void SetGravity(const glm::vec3& gravity);
		const glm::vec3 GetGravity() const { return mGravity; }

		void Step(float dt);

		void CreateRigidBody(const RigidBody3DInfo& info);
		
		glm::vec3 GetPosition(Guid entityGuid) const;
		glm::vec3 GetRotation(Guid entityGuid) const;
		float GetMass(Guid entityGuid) const;
		glm::vec3 GetLinearVelocity(Guid entityGuid) const;
		
		void MoveKinematic(Guid entityGuid, const glm::vec3& targetPosition, const glm::quat& targetRotation, float timeToMove);
		void SetPosition(Guid entityGuid, const glm::vec3& position);
		void SetMass(Guid entityGuid, float mass);
		void AddForce(Guid entityGuid, const glm::vec3& force);
		void AddTorque(Guid entityGuid, const glm::vec3& torque);
		void AddImpulse(Guid entityGuid, const glm::vec3& impulse);
		void AddAngularImpulse(Guid entityGuid, const glm::vec3& angularImpulse);
		void SetLinearVelocity(Guid entityGuid, const glm::vec3& velocity);
		void SetAngularVelocity(Guid entityGuid, const glm::vec3& angularVelocity);

	private:
		JPH::PhysicsSystem* mSystem;
		JPH::PhysicsSettings mSettings;
		JPH::JobSystem* mJobSystem;
		JPH::TempAllocator* mTempAllocator;
		
		// Operational Objects
		ObjectLayerPairFilterImpl mObjectPairFilter;
		BPLayerInterfaceImpl mBroadphaseInterfaceImpl;
		ObjectVsBroadPhaseLayerFilterImpl mObjectVsBroadPhaseLayerFilterImpl;

		// Contact Listener
		KinematicContactListener mKinematicContactListener;

		std::map<Guid, JPH::BodyID> mBodies;

		glm::vec3 mGravity;
	};
}
