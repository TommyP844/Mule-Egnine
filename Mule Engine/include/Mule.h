#pragma once

// Util
#include "Ref.h"
#include "WeakRef.h"

// Application
#include "Application/Application.h"
#include "Application/Window.h"

// Asset
#include "Asset/Asset.h"
#include "Asset/AssetManager.h"

// Layers
#include "Layer/ILayer.h"
#include "Layer/LayerStack.h"

// ECS
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/Scene.h"
#include "ECS/Guid.h"

// Rendering
#include "Graphics/ImGuiContext.h"
#include "Graphics/Model.h"
#include "Graphics/Material.h"
#include "Graphics/EnvironmentMap.h"
#include "Graphics/API/GraphicsContext.h"
#include "Graphics/API/GraphicsCore.h"
#include "Graphics/API/Texture.h"
#include "Graphics/API/UniformBuffer.h"
#include "Graphics/API/ShaderResourceGroup.h"
#include "Graphics/API/CommandBuffer.h"
#include "Graphics/API/GraphicsQueue.h"
#include "Graphics/API/Fence.h"
#include "Graphics/API/Semaphore.h"
#include "Graphics/API/Framebuffer.h"
#include "Graphics/API/GraphicsPipeline.h"

// Events
#include "Application/Events/Event.h"
#include "Application/Events/WindowResizeEvent.h"
#include "Application/Events/DropFileEvent.h"
#include "Application/Events/CharEvent.h"
#include "Application/Events/KeyboardEvent.h"
#include "Application/Events/MouseButtonEvent.h"
#include "Application/Events/MouseMoveEvent.h"

// Engine
#include "Engine Context/EngineContext.h"
#include "Engine Context/EngineAssets.h"

// Scripting
#include "Scripting/ScriptContext.h"
#include "Scripting/ScriptClass.h"

// Core
#include "Timer.h"

// Services
#include "JobSystem/JobSystem.h"

// Generators
#include "Asset/Generator/EnvironmentMapGenerator.h"