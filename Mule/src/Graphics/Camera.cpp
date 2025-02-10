#include "Graphics/Camera.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

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
		UpdateProj();
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
		UpdateProj();
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
		UpdateProj();
	}

	void Camera::SetFarPlane(float farPlane)
	{
		mFarPlane = farPlane;
		UpdateProj();
	}

	void Camera::SetFOVDegrees(float degrees)
	{
		mFOVDeg = degrees;
		UpdateProj();
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		mAspectRatio = aspectRatio;
		UpdateProj();
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
		UpdateViewProj();
	}

	void Camera::UpdateProj()
	{
		mProj = glm::perspective(glm::radians(mFOVDeg), mAspectRatio, mNearPlane, mFarPlane);
		UpdateViewProj();
	}

	void Camera::UpdateViewProj()
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
}