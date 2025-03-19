
using Mule.Components;
using System;
using System.Net.Mime;
using System.Runtime.InteropServices;

namespace Mule
{
    public class Entity
    {
        public readonly uint _id;

        protected Entity()
        {
            this._id = uint.MaxValue;
        }

        public T GetComponent<T>()
        {
            uint componentId = Component.GetComponentID<T>();

            IntPtr nativePtr = IntPtr.Zero;

            unsafe {
                nativePtr = InternalCalls.GetComponentPtr(_id, componentId);
            }

            if (nativePtr == IntPtr.Zero)
                throw new Exception("Component does not exist");

            return (T)Activator.CreateInstance(typeof(T), nativePtr);

        }

        public T AddComponent<T>()
        {
            uint componentId = Component.GetComponentID<T>();

            IntPtr nativePtr = IntPtr.Zero;

            unsafe
            {
                nativePtr = InternalCalls.AddComponentGetPtr(_id, componentId);
            }

            if (nativePtr == IntPtr.Zero)
                throw new Exception("Component does not exist");

            return (T)Activator.CreateInstance(typeof(T), nativePtr);

        }

        public bool HasComponent<T>()
        {
            uint componentId = Component.GetComponentID<T>();

            bool hasComponent = false;

            unsafe
            {
                hasComponent = InternalCalls.HasComponent(_id, componentId);
            }

            return hasComponent;
        }

        public void RemoveComponent<T>()
        {
            uint componentId = Component.GetComponentID<T>();

            unsafe
            {
                InternalCalls.RemoveComponent(_id, componentId);
            }
        }
    }
}