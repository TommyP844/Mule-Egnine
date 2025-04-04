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

    public class RigidBody : Component<RigidBody3DComponent_Int>
    {
        private ulong EntityGuid;
        public RigidBody(IntPtr ptr, ulong entityGuid)
            :
            base(ptr)
        {
            EntityGuid = entityGuid;
        }

        public RigidBody()
            :
            base(IntPtr.Zero)
        {
            EntityGuid = 0;
        }

        public float Mass
        {
            get
            {
                unsafe
                {
                    return InternalCalls.GetRigidBodyMass(EntityGuid);
                }
            }
            set
            {
                unsafe
                {
                    InternalCalls.SetRigidBodyMass(EntityGuid, value);
                    GetStructInt().Mass = value;
                    SaveStructInt();
                }
            }
        }

        public void MoveKinematic(Vector3 targetPosition, Quaternion targetRotation, float timeToMove)
        {
            unsafe
            {
                InternalCalls.MoveRigidBodyKinematic(EntityGuid, targetPosition, targetRotation, timeToMove);
            }
        }

        public void AddForce(Vector3 force)
        {
            unsafe
            {
                InternalCalls.AddRigidBodyForce(EntityGuid, force);
            }
        }

        public void AddTorque(Vector3 torque)
        {
            unsafe
            {
                InternalCalls.AddRigidBodyTorque(EntityGuid, torque);
            }
        }

        public void AddImpulse(Vector3 impulse)
        {
            unsafe
            {
                InternalCalls.AddRigidBodyImpulse(EntityGuid, impulse);
            }
        }

        public void AddAngularImpulse(Vector3 angularImpulse)
        {
            unsafe
            {
                InternalCalls.AddRigidBodyAngularImpulse(EntityGuid, angularImpulse);
            }
        }

        public void SetLinearVelocity(Vector3 linearVelocity)
        {
            unsafe
            {
                InternalCalls.SetRigidBodyLinearVelocity(EntityGuid, linearVelocity);
            }
        }

        public void SetAngularVelocity(Vector3 angularVelocity)
        {
            unsafe
            {
                InternalCalls.SetRigidBodyAngularVelocity(EntityGuid, angularVelocity);
            }
        }

        public Vector3 GetLinearVelocity()
        {
            unsafe
            {
                return InternalCalls.GetRigidBodyLinearVelocity(EntityGuid);
            }
        }
    }
}
