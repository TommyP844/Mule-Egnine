using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mule.Components
{
    public class Component
    {
        protected IntPtr _nativePtr;

        protected Component(nint nativePtr)
        {
            _nativePtr = nativePtr;
        }

        // See ECS/Components for Component ID's
        static internal uint GetComponentID<T>()
        {
            if (typeof(T) == typeof(TransformComponent))
                return 3;

            return 0;
        }
    }
}
