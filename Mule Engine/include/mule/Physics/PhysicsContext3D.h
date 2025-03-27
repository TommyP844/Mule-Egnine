#pragma once

#include "WeakRef.h"
#include "Ref.h"
#include "RigidBody3D.h"
#include "CollisionLayer.h"
#include "Shape3D/Shape3D.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <random>
#include <unordered_map>

namespace Mule
{
	typedef uint64_t PhysicsObjectHandle;

	enum BodyType : uint32_t
	{
		Dynamic,
		Static,
		Kinematic
	};

	struct RigidBody3DInfo
	{
		glm::vec3 Position;
		glm::quat Orientation;
		BodyType Type;
		uint16_t CollisionLayers;
		float Mass;
		Ref<Shape3D> Shape;
	};

	class PhysicsContext3D
	{
	public:
		PhysicsContext3D();
		~PhysicsContext3D();

		void Init();
		void Shutdown();

		void SetGravity(const glm::vec3& gravity);
		const glm::vec3 GetGravity() const { return mGravity; }

		void Step(float dt);

		PhysicsObjectHandle CreateRigidBody3D(const RigidBody3DInfo& info);
		WeakRef<RigidBody3D> GetRigidBody(PhysicsObjectHandle handle);

	private:
		JPH::PhysicsSystem* mSystem;
		JPH::PhysicsSettings mSettings;
		JPH::JobSystem* mJobSystem;
		JPH::TempAllocator* mTempAllocator;
		
		ObjectLayerPairFilterImpl mObjectPairFilter;
		BPLayerInterfaceImpl mBroadphaseInterfaceImpl;
		ObjectVsBroadPhaseLayerFilterImpl mObjectVsBroadPhaseLayerFilterImpl;

		std::random_device mRandomDevice;
		PhysicsObjectHandle GenerateHandle();

		std::unordered_map<PhysicsObjectHandle, Ref<RigidBody3D>> mBodies;

		glm::vec3 mGravity;
	};
}
