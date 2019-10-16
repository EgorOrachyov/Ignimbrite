//
// Created by Egor Orachyov on 2019-10-15.
//

#ifndef VULKANRENDERER_OPTIONAL_H
#define VULKANRENDERER_OPTIONAL_H

#include <Types.h>
#include <new>
#include <memory>

template <typename T>
class Optional {
public:
    Optional() = default;
    explicit Optional(const T& value) {
        new (mData) T(value);
        mIsPresent = true;
    }
    Optional(const Optional<T>& other) {
        if (other.hasValue()) {
            new (mData) T(other.get());
            mIsPresent = true;
        }
    }
    Optional(Optional<T>&& other) noexcept {
        if (other.hasValue()) {
            memcpy(mData, other.mData, sizeof(T));
            mIsPresent = true;
            other.mIsPresent = false;
        }
    }
    ~Optional() {
        if (mIsPresent) {
            get().~T();
        }
    }
    Optional& operator=(const Optional<T>& other) {
        this->~Optional();
        new (this) Optional<T>(other);
    }
    bool hasValue() const {
        return mIsPresent;
    }
    void setValue(const T& value) {
        this->~Optional();
        new (this) Optional<T>(value);
    }
    T& get() {
        return *((T*)mData);
    }
private:
    bool mIsPresent = false;
    char mData[sizeof(T)] = {0};
};


#endif //VULKANRENDERER_OPTIONAL_H
