using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mule.Components
{
    public class TransformComponent : Component
    {
        private struct TransformComponentRaw
        {
            public Vector3 Translation;
            public Vector3 Rotation;
            public Vector3 Scale;
        };

        public TransformComponent(nint nativePtr)
            :
            base(nativePtr)
        {
        }

        public TransformComponent()
            :
            base(IntPtr.Zero)
        {

        }

        public Vector3 Translation
        {
            get
            {
                return Marshal.PtrToStructure<TransformComponentRaw>(_nativePtr).Translation;
            }
            set
            {
                var data = Marshal.PtrToStructure<TransformComponentRaw>(_nativePtr);
                data.Translation = value;
                Marshal.StructureToPtr(data, _nativePtr, false);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                return Marshal.PtrToStructure<TransformComponentRaw>(_nativePtr).Rotation;
            }
            set
            {
                var data = Marshal.PtrToStructure<TransformComponentRaw>(_nativePtr);
                data.Rotation = value;
                Marshal.StructureToPtr(data, _nativePtr, false);
            }
        }

        public Vector3 Scale
        {
            get
            {
                return Marshal.PtrToStructure<TransformComponentRaw>(_nativePtr).Scale;
            }
            set
            {
                var data = Marshal.PtrToStructure<TransformComponentRaw>(_nativePtr);
                data.Scale = value;
                Marshal.StructureToPtr(data, _nativePtr, false);
            }
        }
    }
}
