using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mule.Components
{
    public class TransformComponent : Component<TransformComponent_Int>
    {      
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
                return GetStructInt().Translation;
            }
            set
            {
                GetStructInt().Translation = value;
                SaveStructInt();
            }
        }

        public Vector3 Rotation
        {
            get
            {
                return GetStructInt().Rotation;
            }
            set
            {
                GetStructInt().Rotation = value;
                SaveStructInt();
            }
        }

        public Vector3 Scale
        {
            get
            {
                return GetStructInt().Scale;
            }
            set
            {
                GetStructInt().Scale = value;
                SaveStructInt();
            }
        }
    }
}
