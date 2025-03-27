using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mule
{
    public class Camera
    {
        [StructLayout(LayoutKind.Sequential)]
        internal struct CameraRaw
        {
            public Matrix4x4 View, Proj, ViewProjection;
            public Vector3 Position, WorldUp, ViewDir, RightDir, UpDir;
            public float FOVDegrees, NearPlane, FarPlane, AspectRatio, Yaw, Pitch;
        };

        private CameraRaw _CameraRaw;
        private IntPtr _Ptr;

        public Camera(IntPtr ptr)
        {
            _CameraRaw = Marshal.PtrToStructure<CameraRaw>(ptr);
            _Ptr = ptr;
        }

        public Camera()
        {
            _CameraRaw = new CameraRaw();
            _Ptr = IntPtr.Zero;
        }

        private void UpdateVectors()
        {
            if(_Ptr != IntPtr.Zero)
            {
                unsafe
                {
                    InternalCalls.UpdateCameraVectors(_Ptr);
                }
            }
        }

        private void UpdateView()
        {
            if (_Ptr != IntPtr.Zero)
            {
                unsafe
                {
                    InternalCalls.UpdateCameraViewMatrix(_Ptr);
                }
            }
        }

        private void UpdateProjection()
        {
            if(_Ptr != IntPtr.Zero)
            {
                unsafe
                {
                    InternalCalls.UpdateCameraProjectionMatrix(_Ptr);
                }
            }
        }

        public float Pitch
        {
            get
            {
                return MarshalLoad().Pitch;
            }
            set
            {
                _CameraRaw.Pitch = value;
                MarshalSave();
                UpdateVectors();
            }
        }

        public float Yaw
        {
            get
            {
                return MarshalLoad().Yaw;
            }
            set
            {
                _CameraRaw.Yaw = value;
                MarshalSave();
                UpdateVectors();
            }
        }

        public float FieldOfView
        {
            get
            {
                return MarshalLoad().FOVDegrees;
            }
            set
            {
                _CameraRaw.FOVDegrees = value;
                MarshalSave();
                UpdateView();
            }
        }

        public float NearClip
        {
            get
            {
                return MarshalLoad().NearPlane;
            }
            set
            {
                _CameraRaw.NearPlane = value;
                MarshalSave();
                UpdateProjection();
            }
        }

        public float FarClip
        {
            get
            {
                return MarshalLoad().FarPlane;
            }
            set
            {
                _CameraRaw.FarPlane = value;
                MarshalSave();
                UpdateProjection();
            }
        }

        public Vector3 GetViewDirection()
        {
            return MarshalLoad().ViewDir;
        }

        public Vector3 GetRightDirection()
        {
            return MarshalLoad().RightDir;
        }

        public Vector3 GetLocalUpDirection()
        {
            return MarshalLoad().UpDir;
        }

        public Vector3 GetWorldUpDirection()
        {
            return MarshalLoad().WorldUp;
        }

        private void MarshalSave()
        {
            if(_Ptr != IntPtr.Zero)
                Marshal.StructureToPtr(_CameraRaw, _Ptr, false);
        }

        private CameraRaw MarshalLoad()
        {
            if (_Ptr == IntPtr.Zero)
                return _CameraRaw;

            return Marshal.PtrToStructure<CameraRaw>(_Ptr);
        }

    }
}
