using Mule.Components;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mule.Components
{
    internal class ComponentType
    {
        public static uint GetID<T>()
        {
            if (typeof(T) == typeof(TransformComponent))
                return 3;
            if (typeof(T) == typeof(CameraComponent))
                return 4;
            if (typeof(T) == typeof(RigidBody))
                return 11;

            return uint.MaxValue;
        }
    }
}
