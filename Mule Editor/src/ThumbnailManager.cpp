#include "ThumbnailManager.h"

ThumbnailManager::ThumbnailManager(WeakRef<Mule::EngineContext> context, WeakRef<EditorContext> editorContext)
	:
	mEngineContext(context),
	mEditorContext(editorContext),
	mRunning(true)
{
	mLoadingImage = mEngineContext->GetAssetManager()->LoadAsset<Mule::Texture2D>("../Assets/Textures/Loading.png");
	mRenderThread = std::thread(&ThumbnailManager::Renderthread, this);
	mSceneRenderer = MakeRef<Mule::SceneRenderer>(mEngineContext->GetGraphicsContext(), mEngineContext->GetAssetManager());
	mSceneRenderer->Resize(256, 256);
	mCamera.SetAspectRatio(1.f);
}

ThumbnailManager::~ThumbnailManager()
{
	mRunning = false;
	if (mRenderThread.joinable())
		mRenderThread.join();
}

WeakRef<Mule::Texture2D> ThumbnailManager::GetThumbnail(Mule::AssetHandle handle)
{
	std::lock_guard<std::mutex> lock();
	auto iter = mThumbnails.find(handle);
	if (iter == mThumbnails.end())
	{
		mAssetsToRender.insert(handle);
		return mLoadingImage;
	}
	return iter->second;
}

void ThumbnailManager::Renderthread()
{
	float stacks = 10;
	float slices = 10;
	float radius = 0.5f;

	while (mRunning)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		Mule::AssetHandle handle = Mule::NullAssetHandle;
		{
			std::lock_guard<std::mutex> lock(mMutex);
			if (mAssetsToRender.begin() != mAssetsToRender.end())
			{
				handle = *mAssetsToRender.begin();
			}
			else
			{
				continue;
			}
		}

		WeakRef<Mule::IAsset> asset = mEngineContext->GetAssetManager()->GetAsset<Mule::IAsset>(handle);
		if (!asset)
		{
			mAssetsToRender.erase(handle);
			continue;
		}

		Ref<Mule::Texture2D> texture;
		switch (asset->GetType())
		{
		case Mule::AssetType::Model:
			texture = RenderModel(asset);
			break;
		case Mule::AssetType::Scene:
			texture = RenderScene(asset);
			break;

		case Mule::AssetType::Texture:
		default:
			mAssetsToRender.erase(handle);
			continue;
			break;
		}

		{
			std::lock_guard<std::mutex> lock(mMutex);
			mThumbnails[handle] = texture;
			mAssetsToRender.erase(handle);
		}
	}
}

Ref<Mule::Texture2D> ThumbnailManager::RenderModel(WeakRef<Mule::Model> model)
{
	SPDLOG_INFO("Rendering model: {}", model->Name());
	mScene = MakeRef<Mule::Scene>();

	auto entity = mScene->CreateEntity();
	entity.AddModel(model);
	
	auto lightEntity = mScene->CreateEntity();
	auto& light = lightEntity.AddComponent<Mule::DirectionalLightComponent>();
	light.Intensity = 10.f;
	lightEntity.GetTransformComponent().Rotation.x = -90.f;

	Mule::EditorRenderSettings settings;

	glm::vec3 modelCenter = (model->GetMin() + model->GetMax()) * 0.5f;
	glm::vec3 modelSize = model->GetMax() - model->GetMin();
	float maxAxis = glm::max(modelSize.x, glm::max(modelSize.y, modelSize.z));
	float scale = 200.f / maxAxis;

	ScaleModel(entity, scale);

	mCamera.SetPosition({ 80.f, 90.f, 100.f });
	mCamera.SetViewDir(-mCamera.GetPosition());
	mCamera.SetFOVDegrees(90.f);
	mCamera.SetNearPlane(1.f);

	settings.Scene = mScene;
	settings.EditorCamera = mCamera;

	mSceneRenderer->OnEditorRender(settings);
	auto framebuffer = mSceneRenderer->GetCurrentFrameBuffer();
	auto texture = framebuffer->GetColorAttachment(0);

	Ref<Mule::Texture2D> copyTexture = MakeRef<Mule::Texture2D>(mEngineContext->GetGraphicsContext(), nullptr, 256, 256, texture->GetFormat());

	auto queue = mEngineContext->GetGraphicsContext()->GetGraphicsQueue();
	auto commandPool = queue->CreateCommandPool();
	auto  commandBuffer = commandPool->CreateCommandBuffer();
	auto fence = mEngineContext->GetGraphicsContext()->CreateFence();
	commandBuffer->Begin();
	commandBuffer->TranistionImageLayout(texture, Mule::ImageLayout::TransferSrc);
	commandBuffer->TranistionImageLayout(copyTexture, Mule::ImageLayout::TransferDst);
	Mule::CommandBuffer::TextureCopyInfo info;
	info.Extent = glm::ivec3(256, 256, 1);
	commandBuffer->CopyTexture(texture, copyTexture, info);
	commandBuffer->TranistionImageLayout(texture, Mule::ImageLayout::ShaderReadOnly);
	commandBuffer->TranistionImageLayout(copyTexture, Mule::ImageLayout::ShaderReadOnly);
	commandBuffer->End();
	fence->Reset();
	queue->Submit(commandBuffer, { mSceneRenderer->GetCurrentFrameRenderFinishedSemaphore() }, {}, fence);
	fence->Wait();

	return copyTexture;
}

Ref<Mule::Texture2D> ThumbnailManager::RenderMaterial(WeakRef<Mule::Material> material)
{
	return Ref<Mule::Texture2D>();
}

Ref<Mule::Texture2D> ThumbnailManager::RenderScene(WeakRef<Mule::Scene> scene)
{
	SPDLOG_INFO("Rendering model: {}", scene->Name());

	Mule::EditorRenderSettings settings;

	mCamera.SetPosition({ 50.f, 50.f, 50.f });
	mCamera.SetViewDir(-mCamera.GetPosition());
	mCamera.SetFOVDegrees(90.f);
	mCamera.SetNearPlane(1.f);

	settings.Scene = scene;
	settings.EditorCamera = mCamera;

	mSceneRenderer->OnEditorRender(settings);
	auto framebuffer = mSceneRenderer->GetCurrentFrameBuffer();
	auto texture = framebuffer->GetColorAttachment(0);

	Ref<Mule::Texture2D> copyTexture = MakeRef<Mule::Texture2D>(mEngineContext->GetGraphicsContext(), nullptr, 256, 256, texture->GetFormat());

	auto queue = mEngineContext->GetGraphicsContext()->GetGraphicsQueue();
	auto commandPool = queue->CreateCommandPool();
	auto  commandBuffer = commandPool->CreateCommandBuffer();
	auto fence = mEngineContext->GetGraphicsContext()->CreateFence();
	commandBuffer->Begin();
	commandBuffer->TranistionImageLayout(texture, Mule::ImageLayout::TransferSrc);
	commandBuffer->TranistionImageLayout(copyTexture, Mule::ImageLayout::TransferDst);
	Mule::CommandBuffer::TextureCopyInfo info;
	info.Extent = glm::ivec3(256, 256, 1);
	commandBuffer->CopyTexture(texture, copyTexture, info);
	commandBuffer->TranistionImageLayout(texture, Mule::ImageLayout::ShaderReadOnly);
	commandBuffer->TranistionImageLayout(copyTexture, Mule::ImageLayout::ShaderReadOnly);
	commandBuffer->End();
	fence->Reset();
	queue->Submit(commandBuffer, { mSceneRenderer->GetCurrentFrameRenderFinishedSemaphore() }, {}, fence);
	fence->Wait();

	return copyTexture;
}

void ThumbnailManager::ScaleModel(Mule::Entity e, float scale)
{
	e.GetTransformComponent().Scale = glm::vec3(scale);
	for (auto child : e.Children())
	{
		ScaleModel(child, scale);
	}
}
