
using System;
using System.Net.Mime;

namespace Mule
{
    class Entity
    {
        private readonly uint _id;
        private readonly UIntPtr _scene;

        Entity(uint id, UIntPtr scene)
        {
            this._id = id;
            this._scene = scene;
        }

        public T GetComponent<T>()
        {
            return default(T);
        }
    }
}