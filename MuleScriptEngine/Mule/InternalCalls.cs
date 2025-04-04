﻿using System;
using System.Numerics;

namespace Mule
{
    public static class InternalCalls
    {
        #region Entity

        internal static unsafe delegate*<ulong, uint, IntPtr> GetComponentPtr;
        internal static unsafe delegate*<ulong, uint, IntPtr> AddComponentGetPtr;
        internal static unsafe delegate*<ulong, uint, bool> HasComponent;
        internal static unsafe delegate*<ulong, uint, void> RemoveComponent;

        #endregion

        #region Input

        internal static unsafe delegate*<Vector2, void> SetMousePos;
        internal static unsafe delegate*<float> GetMousePosX;
        internal static unsafe delegate*<float> GetMousePosY;
        internal static unsafe delegate*<uint, bool> IsMouseButtonPressed;
        internal static unsafe delegate*<uint, bool> IsKeyDown;

        #endregion

        #region Camera

        internal static unsafe delegate*<IntPtr, void> UpdateCameraVectors;
        internal static unsafe delegate*<IntPtr, void> UpdateCameraViewMatrix;
        internal static unsafe delegate*<IntPtr, void> UpdateCameraProjectionMatrix;
        internal static unsafe delegate*<IntPtr, void> UpdateCameraVPMatrix;

        #endregion

        #region Physics

        internal static unsafe delegate*<ulong, float> GetRigidBodyMass;
        internal static unsafe delegate*<ulong, float, void> SetRigidBodyMass;
        internal static unsafe delegate*<ulong, Vector3, Quaternion, float, void> MoveRigidBodyKinematic;
        internal static unsafe delegate*<ulong, Vector3, void> AddRigidBodyForce;
        internal static unsafe delegate*<ulong, Vector3, void> AddRigidBodyTorque;
        internal static unsafe delegate*<ulong, Vector3, void> AddRigidBodyImpulse;
        internal static unsafe delegate*<ulong, Vector3, void> AddRigidBodyAngularImpulse;
        internal static unsafe delegate*<ulong, Vector3, void> SetRigidBodyLinearVelocity;
        internal static unsafe delegate*<ulong, Vector3, void> SetRigidBodyAngularVelocity;
        internal static unsafe delegate*<ulong, Vector3> GetRigidBodyLinearVelocity;

        #endregion
    }
}
