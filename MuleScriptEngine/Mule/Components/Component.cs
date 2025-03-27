using System;
using System.Collections.Generic;
using System.Linq;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mule.Components
{
    public class Component<T>
    {
        private IntPtr _NativePtr;
        private T _StructRaw;

        internal Component(IntPtr nativePtr)
        {
            _NativePtr = nativePtr;
            _StructRaw = Marshal.PtrToStructure<T>(nativePtr);
        }

        internal Component()
        {
            _NativePtr = IntPtr.Zero;
            _StructRaw = (T)Activator.CreateInstance(typeof(T));
        }

        internal ref T GetStructInt()
        {
            if (IntPtr.Zero != _NativePtr)
                _StructRaw = Marshal.PtrToStructure<T>(_NativePtr);

            return ref _StructRaw;
        }

        internal void SaveStructInt()
        {
            if (IntPtr.Zero != _NativePtr)
                Marshal.StructureToPtr(_StructRaw, _NativePtr, false);
        }

    };


}
