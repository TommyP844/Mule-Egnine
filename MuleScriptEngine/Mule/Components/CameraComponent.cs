using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mule.Components
{
    public class CameraComponent : Component<CameraComponent_Int>
    {
        private Camera CameraInternal;

        public CameraComponent(IntPtr ptr)
            :
            base(ptr)
        {
            CameraInternal = new Camera(GetStructInt().CameraPtr);
        }

        public CameraComponent()
            :
            base(IntPtr.Zero)
        {
            CameraInternal = new Camera();
        }

        public bool Active
        {
            get
            {
                return GetStructInt().Active;
            }
            set
            {
                GetStructInt().Active = value;
                SaveStructInt();
            }
        }

        public Camera GetCamera() { return CameraInternal; }
    }
}
