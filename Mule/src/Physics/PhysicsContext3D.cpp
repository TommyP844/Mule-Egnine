#include "Physics/PhysicsContext3D.h"

#include "Physics/Shape3D/SphereShape.h"
#include "Physics/Shape3D/BoxShape.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Core/JobSystemThreadPool.h>

#include <spdlog/spdlog.h>

namespace Mule
{
	static void TraceImpl(const char* str, ...)
	{
		va_list list;
		va_start(list, str);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), str, list);
		va_end(list);
		SPDLOG_INFO(buffer);
	}

	PhysicsContext3D::PhysicsContext3D()
		:
		mGravity({0.f, -9.81f, 0.f}),
		mSystem(nullptr)
	{
		JPH::RegisterDefaultAllocator();
		JPH::Trace = TraceImpl;
		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		mJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
		mTempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
	}

	PhysicsContext3D::~PhysicsContext3D()
	{
		if (mSystem)
			delete mSystem;

		delete mJobSystem;
		delete mTempAllocator;
	}

	void PhysicsContext3D::Init()
	{
		mSystem = new JPH::PhysicsSystem();
		mSystem->Init(
			1024,
			0,
			65536,
			2048,
			mBroadphaseInterfaceImpl,
			mObjectVsBroadPhaseLayerFilterImpl,
			mObjectPairFilter);

		mSystem->SetPhysicsSettings(mSettings);

		mSystem->SetGravity({ mGravity.x, mGravity.y, mGravity.x });
	}

	void PhysicsContext3D::Shutdown()
	{
		if (mSystem)
		{
			delete mSystem;
			mSystem = nullptr;
		}

		mBodies.clear();
	}

	void PhysicsContext3D::SetGravity(const glm::vec3& gravity)
	{
		mGravity = gravity;
		if (mSystem)
		{
			mSystem->SetGravity({ mGravity.x, mGravity.y, mGravity.x });
		}
	}

	void PhysicsContext3D::Step(float dt)
	{
		mSystem->Update(dt, 16, mTempAllocator, mJobSystem);
	}

	PhysicsObjectHandle PhysicsContext3D::CreateRigidBody3D(const RigidBody3DInfo& info)
	{
		auto& bodyInterface = mSystem->GetBodyInterface();

		JPH::RVec3 position = JPH::RVec3(info.Position.x, info.Position.y, info.Position.z);
		JPH::Quat orientation = JPH::Quat(info.Orientation.x, info.Orientation.y, info.Orientation.z, info.Orientation.w);
		
		JPH::EMotionType motionType = JPH::EMotionType::Dynamic;
		switch (info.Type)
		{
		case BodyType::Kinematic: motionType = JPH::EMotionType::Kinematic; break;
		case BodyType::Static: motionType = JPH::EMotionType::Static; break;
		}

		JPH::Shape* shape = nullptr;
		switch (info.Shape->GetType())
		{
		case ShapeType::Sphere:
		{
			WeakRef<SphereShape> s = info.Shape;
			shape = new JPH::SphereShape(s->GetRadius());
		}
			break;
		case ShapeType::Box:
		{
			WeakRef<BoxShape> b = info.Shape;
			const glm::vec3& extents = b->GetExtents();
			shape = new JPH::BoxShape(JPH::RVec3(extents.x, extents.y, extents.z));
		}
			break;
		}

		glm::vec3 offset = info.Shape->GetOffset();
		JPH::RotatedTranslatedShape* trsShape = new JPH::RotatedTranslatedShape(JPH::Vec3(offset.x, offset.y, offset.z), JPH::Quat::sIdentity(), shape);

		JPH::BodyCreationSettings bodySettings;
		bodySettings.mPosition = position;
		bodySettings.mRotation = orientation;
		bodySettings.mMotionType = motionType;
		bodySettings.mObjectLayer = 0; //info.CollisionLayers;
		bodySettings.mMassPropertiesOverride.mMass = info.Mass;
		bodySettings.SetShape(trsShape);
		
		JPH::BodyID id = bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);
		
		auto handle = GenerateHandle();
		mBodies[handle] = MakeRef<RigidBody3D>(id, bodyInterface);

		return handle;
	}

	WeakRef<RigidBody3D> PhysicsContext3D::GetRigidBody(PhysicsObjectHandle handle)
	{
		auto iter = mBodies.find(handle);
		if (iter == mBodies.end())
			return nullptr;

		return iter->second;
	}

	PhysicsObjectHandle PhysicsContext3D::GenerateHandle()
	{
		std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);
		auto handle = dist(mRandomDevice);
		return handle;
	}
}