
using Mule.Components;
using System;
using System.Net.Mime;
using System.Runtime.InteropServices;

namespace Mule
{
    public class Entity
    {
        public ulong _guid = 0;

        protected Entity()
        {
            this._guid = 0;
        }

        public ulong GetGuid()
        {
            return _guid;
        }

        public T GetComponent<T>()
        {
            uint componentId = ComponentType.GetID<T>();
            Console.WriteLine($"Component ID: {componentId}");
            IntPtr nativePtr = IntPtr.Zero;

            unsafe {
                nativePtr = InternalCalls.GetComponentPtr(_guid, componentId);
            }

            if (nativePtr == IntPtr.Zero)
                throw new Exception("Component does not exist");

            if(typeof(T) == typeof(RigidBody))
                return (T)Activator.CreateInstance(typeof(T), nativePtr, _guid);

            return (T)Activator.CreateInstance(typeof(T), nativePtr);

        }

        public T AddComponent<T>()
        {
            uint componentId = ComponentType.GetID<T>();

            IntPtr nativePtr = IntPtr.Zero;

            unsafe
            {
                nativePtr = InternalCalls.AddComponentGetPtr(_guid, componentId);
            }

            if (nativePtr == IntPtr.Zero)
                throw new Exception("Component does not exist");

            return (T)Activator.CreateInstance(typeof(T), nativePtr);

        }

        public bool HasComponent<T>()
        {
            uint componentId = ComponentType.GetID<T>();

            bool hasComponent = false;

            unsafe
            {
                hasComponent = InternalCalls.HasComponent(_guid, componentId);
            }

            return hasComponent;
        }

        public void RemoveComponent<T>()
        {
            uint componentId = ComponentType.GetID<T>();

            unsafe
            {
                InternalCalls.RemoveComponent(_guid, componentId);
            }
        }
    }
}