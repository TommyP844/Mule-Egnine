#include "Engine Context/EngineContext.h"
#include "Engine Context/EngineAssets.h"

// Services
#include "Scripting/ScriptContext.h"
#include "JobSystem/JobSystem.h"
#include "Graphics/ShaderFactory.h"

#include "Graphics/Vertex.h"

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

		GraphicsContext::Init(GraphicsAPI::Vulkan, mWindow);

		mServiceManager = MakeRef<ServiceManager>();

		auto assetManager = mServiceManager->Register<AssetManager>();
		mServiceManager->Register<ImGuiContext>(mWindow);
		mServiceManager->Register<ScriptContext>(this);
		mServiceManager->Register<JobSystem>();
		mServiceManager->Register<ShaderFactory>();
		mServiceManager->Register<EnvironmentMapGenerator>(mServiceManager);


		assetManager->LoadRegistry(mFilePath / "Registry.mrz");
		assetManager->RegisterLoader<SceneSerializer>(mServiceManager);
		assetManager->RegisterLoader<ScriptSerializer>(mServiceManager);
		assetManager->RegisterLoader<EnvironmentMapSerializer>(mServiceManager);
		assetManager->RegisterLoader<ModelSerializer>(mServiceManager);
		assetManager->RegisterLoader<TextureSerializer>(mServiceManager);
		assetManager->RegisterLoader<MaterialSerializer>(mServiceManager);

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
		mServiceManager->Unload<ImGuiContext>();

		mServiceManager.Release();

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
		auto shaderFactory = mServiceManager->Get<ShaderFactory>();

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
		assetManager->InsertAsset(blackTexture);

		auto blackTextureCube = TextureCube::Create("Black Texture Cube", blackImage, 2, TextureFormat::RGBA_8U, TextureFlags::TransferDst);
		blackTextureCube->SetHandle(MULE_BLACK_TEXTURE_CUBE_HANDLE);
		assetManager->InsertAsset(blackTextureCube);

		auto whiteTexture = Texture2D::Create("White Image", whiteImage, 2, 2, TextureFormat::RGBA_8U, TextureFlags::TransferDst);
		whiteTexture->SetHandle(MULE_WHITE_TEXTURE_HANDLE);
		assetManager->InsertAsset(whiteTexture);
		
		// Cube
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Cube.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateAssetHandle(mesh->Handle(), MULE_CUBE_MESH_HANDLE);
			});

		// Sphere
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Sphere.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateAssetHandle(mesh->Handle(), MULE_SPHERE_MESH_HANDLE);
			});

		// Cylinder
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Cylinder.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateAssetHandle(mesh->Handle(), MULE_CYLINDER_MESH_HANDLE);
			});

		// Cone
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Cone.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateAssetHandle(mesh->Handle(), MULE_CONE_MESH_HANDLE);
			});

		// Plane
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Plane.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateAssetHandle(mesh->Handle(), MULE_PLANE_MESH_HANDLE);
			});

		// Torus
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Torus.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateAssetHandle(mesh->Handle(), MULE_TORUS_MESH_HANDLE);
			});

		// Beveled Block
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Beveled Block.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateAssetHandle(mesh->Handle(), MULE_BEVELED_BLOCK_MESH_HANDLE);
			});

		// Capsule
		jobSystem->PushJob([assetManager]() {
			auto model = assetManager->LoadAsset<Model>("../Assets/Meshes/Primitives/Capsule.obj");
			auto mesh = model->GetRootNode().GetChildren()[0].GetMeshes()[0];
			assetManager->UpdateAssetHandle(mesh->Handle(), MULE_CAPSULE_MESH_HANDLE);
			});

		// Point Light Icon
		jobSystem->PushJob([assetManager]() {
			auto texture = assetManager->LoadAsset<Texture2D>("../Assets/Textures/point-light-icon.png");
			assetManager->UpdateAssetHandle(texture->Handle(), MULE_POINT_LIGHT_ICON_TEXTURE_HANDLE);
			});

		// Spot Light
		jobSystem->PushJob([assetManager]() {
			auto texture = assetManager->LoadAsset<Texture2D>("../Assets/Textures/spot-light-icon.png");
			assetManager->UpdateAssetHandle(texture->Handle(), MULE_SPOT_LIGHT_ICON_TEXTURE_HANDLE);
			});

		// BRDF LUT
		jobSystem->PushJob([assetManager]() {
			auto texture = assetManager->LoadAsset<Texture2D>("../Assets/Textures/brdf_lut.png");
			assetManager->UpdateAssetHandle(texture->Handle(), MULE_BDRF_LUT_TEXTURE_HANDLE);
			});

		// Graphics Pipelines
		{
			VertexLayout defaultVertexLayout;
			defaultVertexLayout.AddAttribute(AttributeType::Vec3)
				.AddAttribute(AttributeType::Vec3)
				.AddAttribute(AttributeType::Vec3)
				.AddAttribute(AttributeType::Vec2)
				.AddAttribute(AttributeType::Vec4);


			GraphicsPipelineDescription geometryPipeline{};
			geometryPipeline.Filepath = "../Assets/Shaders/Graphics/DefaultGeometryShader.glsl";
			geometryPipeline.FilleMode = FillMode::Solid;
			geometryPipeline.CullMode = CullMode::Back;
			geometryPipeline.VertexLayout = defaultVertexLayout;
			geometryPipeline.DepthFormat = TextureFormat::D_32F;
			geometryPipeline.EnableDepthTest = true;
			geometryPipeline.EnableDepthWrite = true;
			shaderFactory->RegisterGraphicsPipeline("Geometry", geometryPipeline);


			GraphicsPipelineDescription environmentMapPipeline{};
			environmentMapPipeline.Filepath = "../Assets/Shaders/Graphics/EnvironmentMapShader.glsl";
			environmentMapPipeline.FilleMode = FillMode::Solid;
			environmentMapPipeline.CullMode = CullMode::Front;
			environmentMapPipeline.VertexLayout = defaultVertexLayout;
			environmentMapPipeline.DepthFormat = TextureFormat::D_32F;
			environmentMapPipeline.EnableDepthTest = false;
			environmentMapPipeline.EnableDepthWrite = false;
			shaderFactory->RegisterGraphicsPipeline("EnvironmentMap", environmentMapPipeline);
		}

		// Compute Pipelines
		{
			ComputePipelineDescription cubeMapCompute{};
			cubeMapCompute.Filepath = "../Assets/Shaders/Compute/HDRToCubeMapCompute.glsl";
			shaderFactory->RegisterComputePipeline("HDRToCubemap", cubeMapCompute);

			ComputePipelineDescription diffuseIBLCompute{};
			diffuseIBLCompute.Filepath = "../Assets/Shaders/Compute/DiffuseIBLCompute.glsl";
			shaderFactory->RegisterComputePipeline("DiffuseIBL", diffuseIBLCompute);

			ComputePipelineDescription prefilterIBLCompute{};
			prefilterIBLCompute.Filepath = "../Assets/Shaders/Compute/PrefilterIBLCompute.glsl";
			shaderFactory->RegisterComputePipeline("PrefilterIBL", prefilterIBLCompute);
		}
	}
}