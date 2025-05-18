#include "Engine Context/EngineContext.h"
#include "Engine Context/EngineAssets.h"

// Services
#include "Scripting/ScriptContext.h"
#include "JobSystem/JobSystem.h"
#include "Graphics/ShaderFactory.h"

#include "Graphics/Vertex.h"
#include "Graphics/Renderer/Renderer.h"

// Asset Loaders
#include "Asset/Serializer/ModelSerializer.h"
#include "Asset/Serializer/SceneSerializer.h"
#include "Asset/Serializer/TextureSerializer.h"
#include "Asset/Serializer/EnvironmentMapSerializer.h"
#include "Asset/Serializer/MaterialSerializer.h"
#include "Asset/Serializer/ScriptSerializer.h"

// Generators
#include "Asset/Generator/EnvironmentMapGenerator.h"

namespace Mule
{
	EngineContext::EngineContext(EngineContextDescription& description)
		:
		mFilePath(description.ProjectPath)
	{
		mWindow = MakeRef<Window>(description.WindowName);

		// TODO: move graphics context to Renderer
		GraphicsContext::Init(GraphicsAPI::Vulkan, mWindow);

		mServiceManager = MakeRef<ServiceManager>();

		auto assetManager = mServiceManager->Register<AssetManager>();
		mServiceManager->Register<ImGuiContext>(mWindow);
		mServiceManager->Register<ScriptContext>(this);
		mServiceManager->Register<JobSystem>();
		mServiceManager->Register<EnvironmentMapGenerator>(mServiceManager);

		// Needs to be called after imgui init
		Renderer::Init();

		assetManager->LoadRegistry(mFilePath / "Registry.mrz");
		assetManager->RegisterLoader<SceneSerializer>(mServiceManager);
		assetManager->RegisterLoader<ScriptSerializer>(mServiceManager);
		assetManager->RegisterLoader<EnvironmentMapSerializer>(mServiceManager);
		assetManager->RegisterLoader<ModelSerializer>(mServiceManager);
		assetManager->RegisterLoader<TextureSerializer>(mServiceManager);
		assetManager->RegisterLoader<MaterialSerializer>(mServiceManager);

		assetManager->RegisterLoadCallback<Material>([](WeakRef<Material> material) {
			Renderer::Get().AddMaterial(material);
			});

		assetManager->RegisterLoadCallback<Texture2D>([](WeakRef<Texture> texture) {
			Renderer::Get().AddTexture(texture);
			});

		LoadEngineAssets();
	}

	EngineContext::~EngineContext()
	{
		auto assetManager = mServiceManager->Get<AssetManager>();
		assetManager->SaveRegistry(mFilePath / "Registry.mrz");

		GraphicsContext::Get().AwaitIdle();

		mServiceManager->Unload<JobSystem>();
		mServiceManager->Unload<ScriptContext>();
		mServiceManager->Unload<AssetManager>();
		Renderer::Shutdown();
		mServiceManager->Unload<ImGuiContext>();

		mServiceManager.Release();

		ShaderFactory::Shutdown();
		GraphicsContext::Shutdown();
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

	Ref<ServiceManager> EngineContext::GetServiceManager() const
	{
		return mServiceManager;
	}

	void EngineContext::SetScene(WeakRef<Scene> scene)
	{
		if (mScene)
		{
			mScene->OnUnload();
		}

		mScene = scene;
		mScene->OnPrepare();
	}

	void EngineContext::LoadEngineAssets()
	{
		auto assetManager = mServiceManager->Get<AssetManager>();
		auto jobSystem = mServiceManager->Get<JobSystem>();
		auto& shaderFactory = ShaderFactory::Get();

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
		Buffer blackImage(blackImageData, sizeof(blackImageData));

		uint8_t whiteImageData[] = {
			255, 255, 255, 255,		255, 255, 255, 255,
			255, 255, 255, 255,		255, 255, 255, 255
		};
		Buffer whiteImage(whiteImageData, sizeof(whiteImageData));


		auto blackTexture = Texture2D::Create("Black Image", blackImage, 2, 2, TextureFormat::RGBA_8U, TextureFlags::TransferDst);
		blackTexture->SetHandle(MULE_BLACK_TEXTURE_HANDLE);
		assetManager->Insert(blackTexture);

		auto blackTextureCube = TextureCube::Create("Black Texture Cube", blackImage, 2, TextureFormat::RGBA_8U, TextureFlags::TransferDst);
		blackTextureCube->SetHandle(MULE_BLACK_TEXTURE_CUBE_HANDLE);
		assetManager->Insert(blackTextureCube);

		auto whiteTexture = Texture2D::Create("White Image", whiteImage, 2, 2, TextureFormat::RGBA_8U, TextureFlags::TransferDst);
		whiteTexture->SetHandle(MULE_WHITE_TEXTURE_HANDLE);
		assetManager->Insert(whiteTexture);
		
		// Cube
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->Load<Model>("../Assets/Meshes/Primitives/Cube.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateHandle(mesh->Handle(), MULE_CUBE_MESH_HANDLE);
			});

		// Sphere
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->Load<Model>("../Assets/Meshes/Primitives/Sphere.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateHandle(mesh->Handle(), MULE_SPHERE_MESH_HANDLE);
			});

		// Cylinder
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->Load<Model>("../Assets/Meshes/Primitives/Cylinder.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateHandle(mesh->Handle(), MULE_CYLINDER_MESH_HANDLE);
			});

		// Cone
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->Load<Model>("../Assets/Meshes/Primitives/Cone.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateHandle(mesh->Handle(), MULE_CONE_MESH_HANDLE);
			});

		// Plane
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->Load<Model>("../Assets/Meshes/Primitives/Plane.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateHandle(mesh->Handle(), MULE_PLANE_MESH_HANDLE);
			});

		// Torus
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->Load<Model>("../Assets/Meshes/Primitives/Torus.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateHandle(mesh->Handle(), MULE_TORUS_MESH_HANDLE);
			});

		// Beveled Block
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->Load<Model>("../Assets/Meshes/Primitives/Beveled Block.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateHandle(mesh->Handle(), MULE_BEVELED_BLOCK_MESH_HANDLE);
			});

		// Capsule
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->Load<Model>("../Assets/Meshes/Primitives/Capsule.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateHandle(mesh->Handle(), MULE_CAPSULE_MESH_HANDLE);
			});

		// Point Light Icon
		jobSystem->PushJob([assetManager]() {
			auto texture = assetManager->Load<Texture2D>("../Assets/Textures/point-light-icon.png");
			assetManager->UpdateHandle(texture->Handle(), MULE_POINT_LIGHT_ICON_TEXTURE_HANDLE);
			});

		// Spot Light
		jobSystem->PushJob([assetManager]() {
			auto texture = assetManager->Load<Texture2D>("../Assets/Textures/spot-light-icon.png");
			assetManager->UpdateHandle(texture->Handle(), MULE_SPOT_LIGHT_ICON_TEXTURE_HANDLE);
			});

		// BRDF LUT
		jobSystem->PushJob([assetManager]() {
			auto texture = assetManager->Load<Texture2D>("../Assets/Textures/brdf_lut.png");
			assetManager->UpdateHandle(texture->Handle(), MULE_BDRF_LUT_TEXTURE_HANDLE);
			});
	}
}