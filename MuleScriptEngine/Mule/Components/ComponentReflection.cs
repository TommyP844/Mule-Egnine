using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Mule.Components
{
    public enum RigidBodyType : uint
    {
        Dynamic,
        Static,
        Kinematic
    }

    public struct TransformComponent_Int
    {
        public Vector3 Translation;
        public Vector3 Rotation;
        public Vector3 Scale;
    };

    public struct CameraComponent_Int
    {
        public bool Active;
        public IntPtr CameraPtr;
    }

    public struct RigidBody3DComponent_Int
    {
        public ulong Handle;
        public float Mass;
        public RigidBodyType RigidBodyType;
    }
}
