#include "Engine Context/EngineContext.h"
#include "Engine Context/EngineAssets.h"

#include "Scripting/ScriptContext.h"

#include "Graphics/Vertex.h"

// Asset Loaders
#include "Asset/Serializer/ModelSerializer.h"
#include "Asset/Serializer/SceneSerializer.h"
#include "Asset/Serializer/TextureSerializer.h"
#include "Asset/Serializer/EnvironmentMapSerializer.h"
#include "Asset/Serializer/GraphicsShaderSerializer.h"
#include "Asset/Serializer/MaterialSerializer.h"
#include "Asset/Serializer/ScriptSerializer.h"
#include "Asset/Serializer/ComputeShaderSerializer.h"


namespace Mule
{
	EngineContext::EngineContext(EngineContextDescription& description)
		:
		mFilePath(description.ProjectPath)
	{
		mWindow = MakeRef<Window>(description.WindowName);

		mServiceManager = MakeRef<ServiceManager>();

		auto assetManager = mServiceManager->Register<AssetManager>();
		description.GraphicsDescription.Window = mWindow;
		auto graphicsContext = mServiceManager->Register<GraphicsContext>(description.GraphicsDescription);
		mServiceManager->Register<ImGuiContext>(graphicsContext);
		mServiceManager->Register<ScriptContext>(this);
		mServiceManager->Register<SceneRenderer>(graphicsContext, assetManager);

		assetManager->LoadRegistry(mFilePath / "Registry.mrz");
		assetManager->RegisterLoader<SceneSerializer>(mServiceManager);
		assetManager->RegisterLoader<ScriptSerializer>(mServiceManager);
		assetManager->RegisterLoader<EnvironmentMapSerializer>(mServiceManager);
		assetManager->RegisterLoader<ModelSerializer>(mServiceManager);
		assetManager->RegisterLoader<TextureSerializer>(mServiceManager);
		assetManager->RegisterLoader<GraphicsShaderSerializer>(mServiceManager);
		assetManager->RegisterLoader<MaterialSerializer>(mServiceManager);
		assetManager->RegisterLoader<ComputeShaderSerializer>(mServiceManager);

		LoadEngineAssets();
	}

	EngineContext::~EngineContext()
	{
		auto assetManager = mServiceManager->Get<AssetManager>();
		assetManager->SaveRegistry(mFilePath / "Registry.mrz");
	}

	WeakRef<GraphicsContext> EngineContext::GetGraphicsContext() const
	{
		return mServiceManager->Get<GraphicsContext>();
	}

	WeakRef<SceneRenderer> EngineContext::GetSceneRenderer() const
	{
		return mServiceManager->Get<SceneRenderer>();
	}

	WeakRef<ImGuiContext> EngineContext::GetImGuiContext() const
	{
		return mServiceManager->Get<ImGuiContext>();
	}

	WeakRef<ScriptContext> EngineContext::GetScriptContext() const
	{
		return mServiceManager->Get<ScriptContext>();
	}
	WeakRef<AssetManager> EngineContext::GetAssetManager() const
	{
		return mServiceManager->Get<AssetManager>();
	}
	void EngineContext::LoadEngineAssets()
	{
		auto sceneRenderer = mServiceManager->Get<SceneRenderer>();
		auto am = mServiceManager->Get<AssetManager>();
		auto graphicsContext = mServiceManager->Get<GraphicsContext>();

		// Engine Assets		
		uint8_t blackImageData[] = {
			0, 0, 0, 0,		0, 0, 0, 0,
			0, 0, 0, 0,		0, 0, 0, 0,

			0, 0, 0, 0,		0, 0, 0, 0,
			0, 0, 0, 0,		0, 0, 0, 0,

			0, 0, 0, 0,		0, 0, 0, 0,
			0, 0, 0, 0,		0, 0, 0, 0,

			0, 0, 0, 0,		0, 0, 0, 0,
			0, 0, 0, 0,		0, 0, 0, 0,

			0, 0, 0, 0,		0, 0, 0, 0,
			0, 0, 0, 0,		0, 0, 0, 0,

			0, 0, 0, 0,		0, 0, 0, 0,
			0, 0, 0, 0,		0, 0, 0, 0
		};

		uint8_t whiteImageData[] = {
			255, 255, 255, 255,		255, 255, 255, 255,
			255, 255, 255, 255,		255, 255, 255, 255
		};

		auto blackTexture = MakeRef<Texture2D>(graphicsContext, &blackImageData[0], 2, 2, TextureFormat::RGBA8U);
		blackTexture->SetHandle(MULE_BLACK_TEXTURE_HANDLE);
		am->InsertAsset(blackTexture);

		auto blackTextureCube = MakeRef<TextureCube>(graphicsContext, &blackImageData[0], 2, 1, TextureFormat::RGBA8U);
		blackTextureCube->SetHandle(MULE_BLACK_TEXTURE_CUBE_HANDLE);
		am->InsertAsset(blackTextureCube);

		auto whiteTexture = MakeRef<Texture2D>(graphicsContext, &whiteImageData[0], 2, 2, TextureFormat::RGBA8U);
		whiteTexture->SetHandle(MULE_WHITE_TEXTURE_HANDLE);
		am->InsertAsset(whiteTexture);

		sceneRenderer->RefreshEngineObjects();

		mAssetLoadFuture = std::async(std::launch::async, [&]() {
			auto assetManager = mServiceManager->Get<AssetManager>();
			auto sceneRenderer = mServiceManager->Get<SceneRenderer>();
			// Cube
			{
				auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Cube.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				assetManager->UpdateAssetHandle(mesh->Handle(), MULE_CUBE_MESH_HANDLE);
			}

			// Sphere
			{
				auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Sphere.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				assetManager->UpdateAssetHandle(mesh->Handle(), MULE_SPHERE_MESH_HANDLE);
			}

			// Cylinder
			{
				auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Cylinder.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				assetManager->UpdateAssetHandle(mesh->Handle(), MULE_CYLINDER_MESH_HANDLE);
			}

			// Cone
			{
				auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Cone.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				assetManager->UpdateAssetHandle(mesh->Handle(), MULE_CONE_MESH_HANDLE);
			}

			// Plane
			{
				auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Plane.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				assetManager->UpdateAssetHandle(mesh->Handle(), MULE_PLANE_MESH_HANDLE);
			}

			// Torus
			{
				auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Torus.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				assetManager->UpdateAssetHandle(mesh->Handle(), MULE_TORUS_MESH_HANDLE);
			}

			// Beveled Block
			{
				auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Beveled Block.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				assetManager->UpdateAssetHandle(mesh->Handle(), MULE_BEVELED_BLOCK_MESH_HANDLE);
			}

			// Capsule
			{
				auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Capsule.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				assetManager->UpdateAssetHandle(mesh->Handle(), MULE_CAPSULE_MESH_HANDLE);
			}

			// TODO: Wrap in macro

			// Point Light Icon
			{
				auto texture = assetManager->LoadAsset<Texture2D>("../Assets/Textures/point-light-icon.png");
				assetManager->UpdateAssetHandle(texture->Handle(), MULE_POINT_LIGHT_ICON_TEXTURE_HANDLE);
				sceneRenderer->AddTexture(texture);
			}

			// Spot Light
			{
				auto texture = assetManager->LoadAsset<Texture2D>("../Assets/Textures/spot-light-icon.png");
				assetManager->UpdateAssetHandle(texture->Handle(), MULE_SPOT_LIGHT_ICON_TEXTURE_HANDLE);
				sceneRenderer->AddTexture(texture);
			}

			// Graphics Shaders
			{
				auto shader = assetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/DefaultGeometryShader.glsl");
				assetManager->UpdateAssetHandle(shader->Handle(), MULE_GEOMETRY_SHADER_HANDLE);

				shader = assetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/DefaultTransparentGeometryShader.glsl");
				assetManager->UpdateAssetHandle(shader->Handle(), MULE_TRANSPARENT_SHADER_HANDLE);

				shader = assetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/EnvironmentMapShader.glsl");
				assetManager->UpdateAssetHandle(shader->Handle(), MULE_ENVIRONMENT_MAP_SHADER_HANDLE);

				shader = assetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/HighlightEntityShader.glsl");
				assetManager->UpdateAssetHandle(shader->Handle(), MULE_ENTITY_MASK_SHADER_HANDLE);

				shader = assetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/WireFrameShader.glsl");
				assetManager->UpdateAssetHandle(shader->Handle(), MULE_WIRE_FRAME_SHADER_HANDLE);

				shader = assetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/BillBoardShader.glsl");
				assetManager->UpdateAssetHandle(shader->Handle(), MULE_BILLBOARD_SHADER_HANDLE);
			}

			// Compute Shaders
			{
				auto shader = assetManager->LoadAsset<ComputeShader>("../Assets/Shaders/Compute/HighlightShader.glsl");
				assetManager->UpdateAssetHandle(shader->Handle(), MULE_ENTITY_HIGLIGHT_SHADER_HANDLE);
			}
			});
	}
}