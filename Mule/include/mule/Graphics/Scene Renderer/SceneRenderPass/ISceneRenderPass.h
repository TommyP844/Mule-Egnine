#pragma once

#include <typeinfo> // uint32_t
#include "ECS/Scene.h"
#include "Graphics/Execution/Semaphore.h"

namespace Mule
{
	class ISceneRenderPass
	{
	public:
		ISceneRenderPass(uint32_t frameCount)
			:
			mFrameCount(frameCount),
			mFrameIndex(0)
		{ }

		virtual ~ISceneRenderPass() {}

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		void NextFrame()
		{
			mFrameIndex = (mFrameIndex + 1) % mFrameCount;
		}

	protected:
		uint32_t GetFrameIndex() const { return mFrameIndex; }
		const uint32_t mFrameCount;

	private:
		uint32_t mFrameIndex;
	};
}