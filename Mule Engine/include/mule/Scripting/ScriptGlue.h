#pragma once

#include "Engine Context/EngineContext.h"

namespace Mule
{
	void SetContext(WeakRef<EngineContext> context);
	
#pragma region Entity
	
	void* GetComponentPtr(uint64_t guid, uint32_t componentId);
	void* AddComponentGetPtr(uint64_t guid, uint32_t componentId);
	bool HasComponent(uint64_t guid, uint32_t componentId);
	void RemoveComponent(uint64_t guid, uint32_t componentId);

#pragma endregion

#pragma region Input

	void SetMousePos(glm::vec2 pos);
	float GetMousePosX();
	float GetMousePosY();
	bool IsMouseButtonPressed(uint32_t mouseButton);
	bool IsKeyDown(uint32_t key);

#pragma endregion

#pragma region Camera

	void UpdateCameraVectors(Camera* camera);
	void UpdateCameraViewMatrix(Camera* camera);
	void UpdateCameraProjectionMatrix(Camera* camera);
	void UpdateCameraVPMatrix(Camera* camera);

#pragma endregion

#pragma region Physics

	float GetRigidBodyMass(uint64_t entityGuid);
	void SetRigidBodyMass(uint64_t entityGuid, float mass);
	void MoveRigidBodyKinematic(uint64_t entityGuid, glm::vec3 position, glm::quat rotation, float dt);
	void AddRigidBodyForce(uint64_t entityGuid, glm::vec3 force);
	void AddRigidBodyTorque(uint64_t entityGuid, glm::vec3 torque);
	void AddRigidBodyImpulse(uint64_t entityGuid, glm::vec3 impulse);
	void AddRigidBodyAngularImpulse(uint64_t entityGuid, glm::vec3 angularImpulse);
	void SetRigidBodyLinearVelocity(uint64_t entityGuid, glm::vec3 velocity);
	void SetRigidBodyAngularVelocity(uint64_t entityGuid, glm::vec3 angularVelocity);
	glm::vec3 GetRigidBodyLinearVelocity(uint64_t entityGuid);
	
#pragma endregion

}