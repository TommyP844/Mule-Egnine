#pragma once

#include <functional>

template<class T>
class Ref
{
public:
    explicit Ref(T* ptr)
    {
        mPtr = ptr;
        mRefCount = new size_t(1);
    }

    explicit Ref()
    {
        mPtr = nullptr;
        mRefCount = nullptr;
    }

    Ref(std::nullptr_t)
        :
        mPtr(nullptr),
        mRefCount(nullptr)
    {
    }

    Ref(const Ref& other)
        :
        mPtr(other.mPtr),
        mRefCount(other.mRefCount)
    {
        if (mRefCount)
        {
            ++(*mRefCount);
        }
    }

    template<typename Derived>
    Ref(const Ref<Derived>& other)
    {
        // Use dynamic_cast to cast the pointer to the derived type
        mPtr = (T*)other.Get();
        if (mPtr) {
            mRefCount = other.RefCountPtr();
            ++(*mRefCount);
        }
        else {
            // Handle dynamic cast failure
            // For example, set mRefCount to nullptr to indicate an invalid reference
            mRefCount = nullptr;
        }
    }

    template<typename Derived>
    Ref& operator=(const Ref<Derived>& other) {
        if (mPtr != other.Get()) {
            Release();
            mPtr = (T*)other.Get();
            mRefCount = other.RefCountPtr();
            ++(*mRefCount);
        }
        return *this;
    }

    ~Ref() {
        Release();
    }

    Ref& operator=(const Ref& other) {
        if (this != &other && other.mPtr != nullptr) {
            Release();
            mPtr = other.mPtr;
            mRefCount = other.mRefCount;
            ++(*mRefCount);
        }
        return *this;
    }

    Ref& operator=(T* ptr) {
        if (mPtr != ptr) {
            Release();
            mPtr = ptr;
            mRefCount = new size_t(ptr ? 1 : 0);
        }
        return *this;
    }

    bool operator==(std::nullptr_t) const {
        return mPtr == nullptr;
    }

    bool operator==(const Ref<T>& other) const {
        return mPtr == other.mPtr;
    }

    bool operator!=(std::nullptr_t) const {
        return mPtr != nullptr;
    }

    operator bool() const {
        return mPtr != nullptr;
    }

    T& operator*() const {
        return *mPtr;
    }

    // Member access operator
    T* operator->() const {
        return mPtr;
    }

    void Release() {
        if (mPtr != nullptr) {
            if (--(*mRefCount) == 0) {
                delete mPtr;
                delete mRefCount;
            }
            mPtr = nullptr;
            mRefCount = nullptr;
        }
    }

    T* Get() const {
        return mPtr;
    }

    T& GetRef() {
        return *mPtr;
    }

    size_t UseCount() const {
        return (mRefCount != nullptr) ? *mRefCount : 0;
    }

    size_t* RefCountPtr() const { return mRefCount; }

private:
    Ref(T* ptr, size_t* refCount)
        :
        mPtr(ptr),
        mRefCount(refCount)
    {
    }

    T* mPtr;
    size_t* mRefCount;
};

template <class T, typename... Args>
Ref<T> MakeRef(Args&&... args)
{
    return Ref<T>(new T(std::forward<Args>(args)...));
}
