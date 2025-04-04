#pragma once

namespace Mule
{
	struct RotationConstraint
	{
		RotationConstraint() = default;
		RotationConstraint(const RotationConstraint&) = default;

		bool LockX = false;
		bool LockY = false;
		bool LockZ = false;
	};
}