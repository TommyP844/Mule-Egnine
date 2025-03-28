#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyInterface.h>

namespace Mule
{
    class KinematicContactListener : public JPH::ContactListener {
    public:
        virtual JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override {
            return JPH::ValidateResult::AcceptContact;
        }

        // Called when two bodies first come into contact
        virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override {

            // Identify which body is kinematic
            JPH::BodyID kinematicBodyID;
            JPH::Vec3 displacement;
            
            if (inBody1.GetMotionType() == JPH::EMotionType::Kinematic && inBody2.GetMotionType() == JPH::EMotionType::Static) {
                kinematicBodyID = inBody1.GetID();
                displacement = inManifold.mWorldSpaceNormal * inManifold.mPenetrationDepth;
				auto newPos = inBody1.GetPosition() + displacement;
				mPositonUpdates.push_back(std::make_pair(kinematicBodyID, newPos));
            }
            else if (inBody2.GetMotionType() == JPH::EMotionType::Kinematic && inBody1.GetMotionType() == JPH::EMotionType::Static) {
                kinematicBodyID = inBody2.GetID();
                displacement = -inManifold.mWorldSpaceNormal * inManifold.mPenetrationDepth;
				auto newPos = inBody2.GetPosition() + displacement;
				mPositonUpdates.push_back(std::make_pair(kinematicBodyID, newPos));
            }
            else {
                return; // No kinematic-static collision, exit early
            }            
        }

        // Called when two bodies stop colliding
        virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override {
        }

		void SetBodyInterface(JPH::BodyInterface* bodyInterface) {
			mBodyInterface = bodyInterface;
		}

        void ClearPositionUpdates()
        {
            mPositonUpdates.clear();
        }

        const std::vector<std::pair<JPH::BodyID, JPH::Vec3>>& GetPositionUpdates() const
        {
            return mPositonUpdates;
        }

	private:
		JPH::BodyInterface* mBodyInterface;
        std::vector<std::pair<JPH::BodyID, JPH::Vec3>> mPositonUpdates;
    };
}