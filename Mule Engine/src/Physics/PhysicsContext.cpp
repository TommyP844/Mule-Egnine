#include "Physics/PhysicsContext.h"

#include "Physics/Shape3D/SphereShape.h"
#include "Physics/Shape3D/BoxShape.h"
#include "Physics/Shape3D/CapsuleShape.h"
#include "Physics/Shape3D/PlaneShape.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>

#include <spdlog/spdlog.h>

namespace Mule
{
#define CHECK_GUID(guid, ret) auto iter = mBodies.find(guid); if(iter == mBodies.end()) return ret;

	static inline glm::vec3 ToGlm(const JPH::Vec3& vec)
	{
		return { vec.GetX(), vec.GetY(), vec.GetZ() };
	}

	static inline glm::quat ToGlm(const JPH::Quat& q)
	{
		return { q.GetX(), q.GetY(), q.GetZ(), q.GetW() };
	}

	static inline JPH::Vec3 ToJPH(const glm::vec3& vec)
	{
		return { vec.x, vec.y, vec.z };
	}

	static inline JPH::Quat ToJPH(const glm::quat& q)
	{
		return { q.x, q.y, q.z, q.w };
	}

	static void TraceImpl(const char* str, ...)
	{
		va_list list;
		va_start(list, str);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), str, list);
		va_end(list);
		SPDLOG_INFO(buffer);
	}

	PhysicsContext::PhysicsContext()
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

	PhysicsContext::~PhysicsContext()
	{
		if (mSystem)
			delete mSystem;

		delete mJobSystem;
		delete mTempAllocator;
	}

	void PhysicsContext::Init()
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

		mKinematicContactListener.SetBodyInterface(&mSystem->GetBodyInterface());

		mSystem->SetContactListener(&mKinematicContactListener);
	}

	void PhysicsContext::Shutdown()
	{
		if (mSystem)
		{
			delete mSystem;
			mSystem = nullptr;
		}

		mBodies.clear();
	}

	void PhysicsContext::SetGravity(const glm::vec3& gravity)
	{
		mGravity = gravity;
		if (mSystem)
		{
			mSystem->SetGravity({ mGravity.x, mGravity.y, mGravity.x });
		}
	}

	void PhysicsContext::Step(float dt)
	{
		mKinematicContactListener.ClearPositionUpdates();
		mSystem->Update(dt, 16, mTempAllocator, mJobSystem);
		
		auto& bodyInterface = mSystem->GetBodyInterface();
		for (auto [bodyId, pos] : mKinematicContactListener.GetPositionUpdates())
		{
			bodyInterface.SetPosition(bodyId, pos, JPH::EActivation::DontActivate);
		}
	}

	void PhysicsContext::CreateRigidBody(const RigidBody3DInfo& info)
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
		case ShapeType::Capsule:
		{
			WeakRef<CapsuleShape> capsule = info.Shape;
			shape = new JPH::CapsuleShape(capsule->GetHalfHeight(), capsule->GetRadius());
		}
			break;
		case ShapeType::Plane:
			WeakRef<PlaneShape> plane = info.Shape;
			const glm::vec4& planeDir = plane->GetPlane();
			JPH::Plane p = JPH::Plane(JPH::Vec4(planeDir.x, planeDir.y, planeDir.z, -planeDir.w));
			shape = new JPH::PlaneShape(p);
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
		bodySettings.mMotionQuality = JPH::EMotionQuality::LinearCast;
		bodySettings.mCollideKinematicVsNonDynamic = true;
		bodySettings.mAllowSleeping = false;
		bodySettings.mIsSensor = info.Shape->IsTrigger();
		bodySettings.SetShape(trsShape);
		
		JPH::BodyID id = bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);

		mBodies[info.Guid] = id;
	}
	
	glm::vec3 PhysicsContext::GetPosition(Guid entityGuid) const
	{
		CHECK_GUID(entityGuid, glm::vec3());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();
		
		auto pos = bodyInterface.GetPosition(id);

		return ToGlm(pos);
	}

	glm::vec3 PhysicsContext::GetRotation(Guid entityGuid) const
	{
		CHECK_GUID(entityGuid, glm::vec3());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		auto q = bodyInterface.GetRotation(id);
		auto rot = glm::degrees(glm::eulerAngles(ToGlm(q)));

		return rot;
	}
	
	float PhysicsContext::GetMass(Guid entityGuid) const
	{
		CHECK_GUID(entityGuid, -1.f);
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyLockInterface = mSystem->GetBodyLockInterface();

		JPH::BodyLockRead lock(bodyLockInterface, id);
		if (lock.Succeeded())
		{
			const auto& body = lock.GetBody();
			float inverseMass = body.GetMotionProperties()->GetInverseMass();
			if (inverseMass == 0.f)
				return 0.f;
			else
				return 1.f / inverseMass;
		}

		return -1.f;
	}

	glm::vec3 PhysicsContext::GetLinearVelocity(Guid entityGuid) const
	{
		CHECK_GUID(entityGuid, glm::vec3());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		auto linearVelocity = bodyInterface.GetLinearVelocity(id);

		return ToGlm(linearVelocity);
	}
	
	void PhysicsContext::MoveKinematic(Guid entityGuid, const glm::vec3& targetPosition, const glm::quat& targetRotation, float timeToMove)
	{
		CHECK_GUID(entityGuid, void());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		bodyInterface.MoveKinematic(id, ToJPH(targetPosition), ToJPH(targetRotation), timeToMove);
	}

	void PhysicsContext::SetPosition(Guid entityGuid, const glm::vec3& position)
	{
		CHECK_GUID(entityGuid, void());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		bodyInterface.SetPosition(id, ToJPH(position), JPH::EActivation::Activate);
	}
	
	void PhysicsContext::SetMass(Guid entityGuid, float mass)
	{
		CHECK_GUID(entityGuid, void());
		auto id = mBodies.at(entityGuid);
		auto& lockInterface = mSystem->GetBodyLockInterface();
		JPH::BodyLockWrite lock(lockInterface, id);

		mass = glm::clamp(mass, 0.f, FLT_MAX);
		if (mass > 0.f) mass = 1.f / mass;

		if (lock.Succeeded())
		{
			JPH::Body& body = lock.GetBody();
			body.GetMotionProperties()->SetInverseMass(mass);
		}
	}
	
	void PhysicsContext::AddForce(Guid entityGuid, const glm::vec3& force)
	{
		CHECK_GUID(entityGuid, void());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		bodyInterface.AddForce(id, ToJPH(force), JPH::EActivation::Activate);
	}

	void PhysicsContext::AddTorque(Guid entityGuid, const glm::vec3& torque)
	{
		CHECK_GUID(entityGuid, void());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		bodyInterface.AddTorque(id, ToJPH(torque), JPH::EActivation::Activate);
	}

	void PhysicsContext::AddImpulse(Guid entityGuid, const glm::vec3& impulse)
	{
		CHECK_GUID(entityGuid, void());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		bodyInterface.AddImpulse(id, ToJPH(impulse));
	}

	void PhysicsContext::AddAngularImpulse(Guid entityGuid, const glm::vec3& angularImpulse)
	{
		CHECK_GUID(entityGuid, void());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		bodyInterface.AddAngularImpulse(id, ToJPH(angularImpulse));
	}

	void PhysicsContext::SetLinearVelocity(Guid entityGuid, const glm::vec3& velocity)
	{
		CHECK_GUID(entityGuid, void());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		bodyInterface.SetLinearVelocity(id, ToJPH(velocity));
	}

	void PhysicsContext::SetAngularVelocity(Guid entityGuid, const glm::vec3& angularVelocity)
	{
		CHECK_GUID(entityGuid, void());
		JPH::BodyID id = mBodies.at(entityGuid);
		auto& bodyInterface = mSystem->GetBodyInterface();

		bodyInterface.SetLinearVelocity(id, ToJPH(angularVelocity));
	}
}