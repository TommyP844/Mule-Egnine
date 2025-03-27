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

	float GetRigidBody3DMass(PhysicsObjectHandle handle);
	void SetRigidBody3DMass(PhysicsObjectHandle handle, float mass);
	void MoveRigidBody3DKinematic(PhysicsObjectHandle handle, glm::vec3 position, glm::quat rotation, float dt);

#pragma endregion

}