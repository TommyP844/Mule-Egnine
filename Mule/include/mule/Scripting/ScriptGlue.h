#pragma once

#include "Engine Context/EngineContext.h"

namespace Mule::Scripting
{
	void SetContext(WeakRef<EngineContext> context);
	
	void* GetComponentPtr(uint32_t entityId, uint32_t componentId);
	void* AddComponentGetPtr(uint32_t entityId, uint32_t componentId);
	bool HasComponent(uint32_t entityId, uint32_t componentId);
	void RemoveComponent(uint32_t entityId, uint32_t componentId);

	void SetMousePos(glm::vec2 pos);
	glm::vec2 GetMousePos();	
	bool IsMouseButtonPressed(uint32_t mouseButton);
	bool IsKeyDown(uint32_t key);

}