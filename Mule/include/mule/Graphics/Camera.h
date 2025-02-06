#pragma once

#include <glm/glm.hpp>

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

		void Rotate(float yawDelata, float pitchDelata);

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
	private:
		glm::mat4 mView, mProj, mVP;

		glm::vec3 mPosition, mWorldUp, mViewDir, mRight, mUp;
		float mFOVDeg, mNearPlane, mFarPlane, mAspectRatio, mYaw, mPitch;

		void UpdateView();
		void UpdateProj();
		void UpdateViewProj();
		void UpdateLocalVectors();
	};
}