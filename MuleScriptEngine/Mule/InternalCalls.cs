using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mule
{
    public static class InternalCalls
    {
        #region Entity

        internal static unsafe delegate*<uint, uint, IntPtr> GetComponentPtr;
        internal static unsafe delegate*<uint, uint, IntPtr> AddComponentGetPtr;
        internal static unsafe delegate*<uint, uint, bool> HasComponent;
        internal static unsafe delegate*<uint, uint, void> RemoveComponent;

        #endregion

        #region Input

        internal static unsafe delegate*<Vector2, void> SetMousePos;
        internal static unsafe delegate*<Vector2> GetMousePos;
        internal static unsafe delegate*<uint, bool> IsMouseButtonPressed;
        internal static unsafe delegate*<uint, bool> IsKeyDown;

        #endregion
    }
}
