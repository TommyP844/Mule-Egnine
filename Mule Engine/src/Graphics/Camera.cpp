#include "Graphics/Camera.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <algorithm>

#include <array>

namespace Mule
{
	Camera::Camera()
		:
		mPosition({0.f, 0.f, 0.f}),
		mFOVDeg(70.f),
		mNearPlane(1.f),
		mFarPlane(1000.f),
		mAspectRatio(1.f),
		mViewDir({ 0.f, 0.f, -1.f }),
		mRight({ 1.f, 0.f, 0.f }),
		mWorldUp({ 0.f, 1.f, 0.f }),
		mUp({ 0.f, 1.f, 0.f }),
		mYaw(0.f),
		mPitch(0.f)
	{
		UpdateView();
		UpdateProjection();
	}
	 
	Camera::Camera(const glm::vec3& position, float fovDegrees, float nearPlane, float farPlane)
		:
		mPosition(position),
		mFOVDeg(fovDegrees),
		mNearPlane(nearPlane),
		mFarPlane(farPlane),
		mAspectRatio(1.f),
		mViewDir({0.f, 0.f, -1.f}),
		mRight({1.f, 0.f, 0.f}),
		mWorldUp({0.f, 1.f, 0.f}),
		mUp({0.f, 1.f, 0.f}),
		mYaw(0.f),
		mPitch(0.f)
	{
		UpdateView();
		UpdateProjection();
	}

	void Camera::SetYaw(float yaw)
	{
		mYaw = fmod(yaw, 360.f);
		UpdateLocalVectors();
	}

	void Camera::SetPitch(float pitch)
	{
		mPitch = glm::clamp(pitch, -89.f, 89.f);
		UpdateLocalVectors();
	}

	void Camera::SetPosition(const glm::vec3& position)
	{
		mPosition = position;
		UpdateView();
	}

	void Camera::SetWorldUp(const glm::vec3& worldUp)
	{
		mWorldUp = worldUp;
		UpdateView();
	}

	void Camera::SetViewDir(const glm::vec3& viewDir)
	{
		mViewDir = glm::normalize(viewDir);
		mYaw = glm::degrees(atan2(viewDir.z, viewDir.x));
		mPitch = glm::degrees(asin(viewDir.y));
		mRight = glm::normalize(glm::cross(mWorldUp, mViewDir));
		mUp = glm::normalize(glm::cross(mRight, mViewDir));
		UpdateView();
	}

	void Camera::SetNearPlane(float nearPlane)
	{
		mNearPlane = nearPlane;
		UpdateProjection();
	}

	void Camera::SetFarPlane(float farPlane)
	{
		mFarPlane = farPlane;
		UpdateProjection();
	}

	void Camera::SetFOVDegrees(float degrees)
	{
		mFOVDeg = degrees;
		UpdateProjection();
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		mAspectRatio = aspectRatio;
		UpdateProjection();
	}

	void Camera::Rotate(float yawDelta, float pitchDelta)
	{
		mPitch = glm::clamp(mPitch + pitchDelta, -89.f, 89.f);
		mYaw = fmod(mYaw + yawDelta, 360.f);

		UpdateLocalVectors();
	}

	void Camera::UpdateView()
	{
		mView = glm::lookAt(mPosition, mPosition + mViewDir, mWorldUp);
		UpdateViewProjection();
	}

	void Camera::UpdateProjection()
	{
		mProj = glm::perspective(glm::radians(mFOVDeg), mAspectRatio, mNearPlane, mFarPlane);
		UpdateViewProjection();
	}

	void Camera::UpdateViewProjection()
	{
		mVP = mProj * mView;
	}

	void Camera::UpdateLocalVectors()
	{
		mViewDir.x = cos(glm::radians(mPitch)) * cos(glm::radians(mYaw));
		mViewDir.y = sin(glm::radians(mPitch));
		mViewDir.z = cos(glm::radians(mPitch)) * sin(glm::radians(mYaw));
		mViewDir = glm::normalize(mViewDir);

		// Recalculate the right vector
		mRight = glm::normalize(glm::cross(mViewDir, mWorldUp));

		// Recalculate the up vector
		mUp = glm::normalize(glm::cross(mRight, mViewDir));

		UpdateView();
	}

	Camera::CascadeSplits Camera::GenerateLightSpaceCascades(uint32_t count, const glm::vec3& direction) const
	{
		const float cascadeSplitLambda = 0.95;

		CascadeSplits cascadeData;
		cascadeData.LightSpaceMatrices.resize(count);
		cascadeData.SplitDistances.resize(count);
		cascadeData.Count = count;

		std::vector<float> cascadeSplits(count);
		
		float nearClip = mNearPlane;
		float farClip = mFarPlane;
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		// Calculate split depths based on view camera frustum
		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < count; i++) {
			float p = (i + 1) / static_cast<float>(count);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = cascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
		}

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < count; i++) {
			float splitDist = cascadeSplits[i];

			glm::vec3 frustumCorners[8] = {
				glm::vec3(-1.0f,  1.0f, 0.0f),
				glm::vec3(1.0f,  1.0f, 0.0f),
				glm::vec3(1.0f, -1.0f, 0.0f),
				glm::vec3(-1.0f, -1.0f, 0.0f),
				glm::vec3(-1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f, -1.0f,  1.0f),
				glm::vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			glm::mat4 invCam = glm::inverse(mProj * mView);
			for (uint32_t j = 0; j < 8; j++) {
				glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[j], 1.0f);
				frustumCorners[j] = invCorner / invCorner.w;
			}

			for (uint32_t j = 0; j < 4; j++) {
				glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
				frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
				frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t j = 0; j < 8; j++) {
				frustumCenter += frustumCorners[j];
			}
			frustumCenter /= 8.0f;

			float radius = 0.0f;
			for (uint32_t j = 0; j < 8; j++) {
				float distance = glm::length(frustumCorners[j] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			glm::vec3 lightDir = glm::normalize(direction);
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

			glm::mat4 lightViewProj = lightOrthoMatrix * lightViewMatrix;

			// 2. Transform origin (0, 0, 0) to light space to find texel alignment
			glm::vec4 shadowOrigin = lightViewProj * glm::vec4(0.0f);
			shadowOrigin *= (float)2048.f / 2.0f;
			
			// 3. Round to nearest texel
			glm::vec4 roundedOrigin = glm::round(shadowOrigin);
			
			// 4. Compute offset and apply it back
			glm::vec4 offset = (roundedOrigin - shadowOrigin) * 2.0f / (float)2048.f;
			offset.z = 0.0f;
			offset.w = 0.0f;
			
			glm::mat4 texelSnapOffset = glm::translate(glm::mat4(1.0f), glm::vec3(offset));
			
			// 5. Apply to final matrix
			lightViewProj = texelSnapOffset * lightViewProj;

			// Store the final matrix
			cascadeData.LightSpaceMatrices[i] = lightViewProj;
			cascadeData.SplitDistances[i] = (mNearPlane + splitDist * clipRange) * -1.0f;
			
			lastSplitDist = cascadeSplits[i];
		}

		return cascadeData;
	}

	WeakRef<TextureView> Camera::GetColorOutput() const
	{
		return mResourceRegistry->GetColorOutput();
	}
}