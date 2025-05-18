#pragma once

#include "Ref.h"

template<typename T>
class WeakRef
{
public:
    WeakRef(T* ptr = nullptr)
        :
        mPtr(ptr)
    {
    }

    WeakRef(const WeakRef& other)
        :
        mPtr(other.mPtr)
    {
    }

    WeakRef(const Ref<T>& other)
        :
        mPtr(other.Get())
    {
    }

    WeakRef(std::nullptr_t) 
        :
        mPtr(nullptr)
    {
    }

    template<typename Derived>
    WeakRef(const Ref<Derived>& other)
    {
		static_assert(std::is_base_of<T, Derived>::value || std::is_base_of<Derived, T>::value, "WeakRef can only be constructed from Ref of a derived type");
        mPtr = (T*)other.Get();
    }

    template<typename Derived>
    WeakRef(const WeakRef<Derived>& other)
    {
        static_assert(std::is_base_of<T, Derived>::value || std::is_base_of<Derived, T>::value, "WeakRef can only be constructed from WeakRef of a derived type");
        mPtr = (T*)other.Get();
    }

    ~WeakRef() {}


    WeakRef& operator=(T* ptr) {
        if (mPtr != ptr) {
            mPtr = ptr;
        }
        return *this;
    }

    bool operator==(std::nullptr_t) const {
        return mPtr == nullptr;
    }

    bool operator==(const WeakRef<T>& other) const {
        return mPtr == other.mPtr;
    }

    bool operator!=(std::nullptr_t) const {
        return mPtr != nullptr;
    }

    operator bool() const {
        return mPtr != nullptr;
    }

    WeakRef& operator=(const WeakRef& other) {
        if (this != &other) {
            mPtr = other.mPtr;
        }
        return *this;
    }

    WeakRef& operator=(const Ref<T>& other)
    {
        mPtr = other.Get();
        return *this;
    }

    template<typename Derived>
    WeakRef& operator=(const Ref<Derived>& other)
    {
        mPtr = (T*)other.Get();
        return *this;
    }

    template<typename Derived>
    WeakRef& operator=(const WeakRef<Derived>& other)
    {
        mPtr = (T*)other.Get();
        return *this;
    }

    T& operator*() const {
        return *mPtr;
    }

    // Member access operator
    T* operator->() const {
        return mPtr;
    }

    T* Get() const {
        return mPtr;
    }

private:
    T* mPtr;
};