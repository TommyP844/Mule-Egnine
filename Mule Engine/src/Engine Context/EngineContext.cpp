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
		description.GraphicsDescription.Window = mWindow;
		mGraphicsContext = MakeRef<GraphicsContext>(description.GraphicsDescription);
		mImguiContext = MakeRef<ImGuiContext>(mGraphicsContext);
		mAssetManager = MakeRef<AssetManager>();		
		mSceneRenderer = MakeRef<SceneRenderer>(mGraphicsContext, mAssetManager);
		mScriptContext = MakeRef<ScriptContext>(this);

		mAssetManager->LoadRegistry(mFilePath / "Registry.mrz");

		mAssetManager->RegisterLoader<SceneSerializer>(this);
		mAssetManager->RegisterLoader<ScriptSerializer>();
		mAssetManager->RegisterLoader<EnvironmentMapSerializer>(mGraphicsContext, WeakRef<EngineContext>(this));
		mAssetManager->RegisterLoader<ModelSerializer>(mGraphicsContext, WeakRef<EngineContext>(this));
		mAssetManager->RegisterLoader<TextureSerializer>(mGraphicsContext);
		mAssetManager->RegisterLoader<GraphicsShaderSerializer>(this);
		mAssetManager->RegisterLoader<MaterialSerializer>();
		mAssetManager->RegisterLoader<ComputeShaderSerializer>(this);

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

		auto blackTexture = MakeRef<Texture2D>(mGraphicsContext, &blackImageData[0], 2, 2, TextureFormat::RGBA8U);
		blackTexture->SetHandle(MULE_BLACK_TEXTURE_HANDLE);
		InsertAsset(blackTexture);

		auto blackTextureCube = MakeRef<TextureCube>(mGraphicsContext, &blackImageData[0], 2, 1, TextureFormat::RGBA8U);
		blackTextureCube->SetHandle(MULE_BLACK_TEXTURE_CUBE_HANDLE);
		InsertAsset(blackTextureCube);

		auto whiteTexture = MakeRef<Texture2D>(mGraphicsContext, &whiteImageData[0], 2, 2, TextureFormat::RGBA8U);
		whiteTexture->SetHandle(MULE_WHITE_TEXTURE_HANDLE);
		InsertAsset(whiteTexture);

		mSceneRenderer->RefreshEngineObjects();

		mAssetLoadFuture = std::async(std::launch::async, [&]() {

			// Cube
			{
				auto model = LoadAsset<Model>("../Assets/Meshes/Primitives/Cube.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				UpdateAssetHandle(mesh->Handle(), MULE_CUBE_MESH_HANDLE);
			}

			// Sphere
			{
				auto model = LoadAsset<Model>("../Assets/Meshes/Primitives/Sphere.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				UpdateAssetHandle(mesh->Handle(), MULE_SPHERE_MESH_HANDLE);
			}

			// Cylinder
			{
				auto model = LoadAsset<Model>("../Assets/Meshes/Primitives/Cylinder.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				UpdateAssetHandle(mesh->Handle(), MULE_CYLINDER_MESH_HANDLE);
			}

			// Cone
			{
				auto model = LoadAsset<Model>("../Assets/Meshes/Primitives/Cone.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				UpdateAssetHandle(mesh->Handle(), MULE_CONE_MESH_HANDLE);
			}

			// Plane
			{
				auto model = LoadAsset<Model>("../Assets/Meshes/Primitives/Plane.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				UpdateAssetHandle(mesh->Handle(), MULE_PLANE_MESH_HANDLE);
			}

			// Torus
			{
				auto model = LoadAsset<Model>("../Assets/Meshes/Primitives/Torus.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				UpdateAssetHandle(mesh->Handle(), MULE_TORUS_MESH_HANDLE);
			}

			// Beveled Block
			{
				auto model = LoadAsset<Model>("../Assets/Meshes/Primitives/Beveled Block.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				UpdateAssetHandle(mesh->Handle(), MULE_BEVELED_BLOCK_MESH_HANDLE);
			}

			// Capsule
			{
				auto model = LoadAsset<Model>("../Assets/Meshes/Primitives/Capsule.obj");
				auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
				UpdateAssetHandle(mesh->Handle(), MULE_CAPSULE_MESH_HANDLE);
			}

			// TODO: Wrap in macro

			// Point Light Icon
			{
				auto texture = mAssetManager->LoadAsset<Texture2D>("../Assets/Textures/point-light-icon.png");
				UpdateAssetHandle(texture->Handle(), MULE_POINT_LIGHT_ICON_TEXTURE_HANDLE);
				mSceneRenderer->AddTexture(texture);
			}

			// Spot Light
			{
				auto texture = mAssetManager->LoadAsset<Texture2D>("../Assets/Textures/spot-light-icon.png");
				UpdateAssetHandle(texture->Handle(), MULE_SPOT_LIGHT_ICON_TEXTURE_HANDLE);
				mSceneRenderer->AddTexture(texture);
			}

			// Graphics Shaders
			{
				auto shader = mAssetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/DefaultGeometryShader.glsl");
				UpdateAssetHandle(shader->Handle(), MULE_GEOMETRY_SHADER_HANDLE);

				shader = mAssetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/DefaultTransparentGeometryShader.glsl");
				UpdateAssetHandle(shader->Handle(), MULE_TRANSPARENT_SHADER_HANDLE);

				shader = mAssetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/EnvironmentMapShader.glsl");
				UpdateAssetHandle(shader->Handle(), MULE_ENVIRONMENT_MAP_SHADER_HANDLE);

				shader = mAssetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/HighlightEntityShader.glsl");
				UpdateAssetHandle(shader->Handle(), MULE_ENTITY_MASK_SHADER_HANDLE);

				shader = mAssetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/WireFrameShader.glsl");
				UpdateAssetHandle(shader->Handle(), MULE_WIRE_FRAME_SHADER_HANDLE);

				shader = mAssetManager->LoadAsset<GraphicsShader>("../Assets/Shaders/Graphics/BillBoardShader.glsl");
				UpdateAssetHandle(shader->Handle(), MULE_BILLBOARD_SHADER_HANDLE);
			}

			// Compute Shaders
			{
				auto shader = mAssetManager->LoadAsset<ComputeShader>("../Assets/Shaders/Compute/HighlightShader.glsl");
				UpdateAssetHandle(shader->Handle(), MULE_ENTITY_HIGLIGHT_SHADER_HANDLE);
			}
			});
	}

	EngineContext::~EngineContext()
	{
		mAssetManager->SaveRegistry(mFilePath / "Registry.mrz");

		mImguiContext = nullptr;
		mSceneRenderer = nullptr;
		mAssetManager = nullptr;
		mGraphicsContext = nullptr;
	}

	void EngineContext::RemoveAsset(AssetHandle handle)
	{
		auto asset = mAssetManager->GetAsset<IAsset>(handle);
		switch (asset->GetType())
		{
		case AssetType::Texture:
			mSceneRenderer->RemoveTexture(handle);
			break;
		case AssetType::Material:
			mSceneRenderer->RemoveMaterial(handle);
			break;
		case AssetType::EnvironmentMap:
			WeakRef<EnvironmentMap> envMap = asset;
			mSceneRenderer->RemoveTexture(envMap->GetCubeMapHandle());
			mSceneRenderer->RemoveTexture(envMap->GetDiffuseIBLMap());
			mSceneRenderer->RemoveTexture(envMap->GetPreFilterMap());
			break;
		}
		mAssetManager->RemoveAsset(handle);
	}

	std::vector<Ref<IAsset>> EngineContext::GetAssetsOfType(AssetType type) const
	{
		return mAssetManager->GetAssetsOfType(type);
	}

	const std::unordered_map<AssetHandle, Ref<IAsset>>& EngineContext::GetAllAssets() const
	{
		return mAssetManager->GetAllAssets();
	}

	Ref<IAsset> EngineContext::GetAssetByFilepath(const fs::path& path)
	{
		return mAssetManager->GetAssetByFilepath(path);
	}

	void EngineContext::UpdateAssetHandle(AssetHandle oldHandle, AssetHandle newHandle)
	{
		mAssetManager->UpdateAssetHandle(oldHandle, newHandle);
	}
}