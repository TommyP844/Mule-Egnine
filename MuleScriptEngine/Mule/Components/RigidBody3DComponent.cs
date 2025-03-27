using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace Mule.Components
{
    // Physics Components in the editor are just layouts for the objects to be created at runtime
    // In the scripts they reference actual objects so the data layout is different in the sence that we wont modify the component
    // But the underlying rigid body instead

    public class RigidBody3DComponent : Component<RigidBody3DComponent_Int>
    {
        private ulong PhysicsHandle;
        public RigidBody3DComponent(IntPtr ptr)
            :
            base(ptr)
        {
            PhysicsHandle = GetStructInt().Handle;
        }

        public RigidBody3DComponent()
            :
            base(IntPtr.Zero)
        { }

        public float Mass
        {
            get
            {
                unsafe
                {
                    return InternalCalls.GetRigidBody3DMass(PhysicsHandle);
                }
            }
            set
            {
                unsafe
                {
                    InternalCalls.SetRigidBody3DMass(PhysicsHandle, value);
                    GetStructInt().Mass = value;
                    SaveStructInt();
                }
            }
        }

        public void MoveKinematic(Vector3 targetPosition, Quaternion targetRotation, float timeToMove)
        {
            unsafe
            {
                InternalCalls.MoveRigidBody3DKinematic(PhysicsHandle, targetPosition, targetRotation, timeToMove);
            }
        }
    }
}
