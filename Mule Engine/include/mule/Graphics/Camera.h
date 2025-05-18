#pragma once

#include <glm/glm.hpp>

#include "Ref.h"
#include "Graphics/Renderer/RenderGraph/ResourceRegistry.h"

namespace Mule
{
	class Camera
	{
	public:
		Camera(const glm::vec3& position, float fovDegrees, float nearPlane, float farPlane);
		Camera();
		~Camera(){}

		void SetYaw(float yaw);
		void SetPitch(float pitch);
		void SetPosition(const glm::vec3& position);
		void SetWorldUp(const glm::vec3& worldUp);
		void SetViewDir(const glm::vec3& viewDir);
		void SetNearPlane(float nearPlane);
		void SetFarPlane(float farPlane);
		void SetFOVDegrees(float degrees);
		void SetAspectRatio(float aspectRatio);

		// Getters
		const glm::mat4& GetView() const { return mView; }
		const glm::mat4& GetProj() const { return mProj; }
		const glm::mat4& GetViewProj() const { return mVP; }
		const glm::vec3& GetPosition() const { return mPosition; }
		const glm::vec3& GetWorldUp() const { return mWorldUp; }
		const glm::vec3& GetLocalUp() const { return mUp; }
		const glm::vec3& GetForwardDir() const { return mViewDir; }
		const glm::vec3& GetRightDir() const { return mRight; }
		float GetNearPlane() const { return mNearPlane; }
		float GetFarPlane() const { return mFarPlane; }
		float GetFOVDegrees() const { return mFOVDeg; }
		float GetAspectRatio() const { return mAspectRatio; }
		float GetYaw() const { return mYaw; }
		float GetPitch() const { return mPitch; }


		void Rotate(float yawDelata, float pitchDelata);
		void UpdateView();
		void UpdateProjection();
		void UpdateViewProjection();
		void UpdateLocalVectors();

		void SetResourceRegistry(Ref<ResourceRegistry> registry) { mResourceRegistry = registry; }
		Ref<ResourceRegistry> GetRegistry() const { return mResourceRegistry; }

		struct CascadeSplits
		{
			std::vector<glm::mat4> LightSpaceMatrices;
			std::vector<float> SplitDistances;
			uint32_t Count;
		};

		CascadeSplits GenerateLightSpaceCascades(uint32_t count, const glm::vec3& direction) const;

		WeakRef<TextureView> GetColorOutput() const;

	private:
		glm::mat4 mView, mProj, mVP;

		glm::vec3 mPosition, mWorldUp, mViewDir, mRight, mUp;
		float mFOVDeg, mNearPlane, mFarPlane, mAspectRatio, mYaw, mPitch;

		Ref<ResourceRegistry> mResourceRegistry;
	};
}