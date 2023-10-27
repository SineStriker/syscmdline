#ifndef SHAREDBASE_H
#define SHAREDBASE_H

#include <utility>

#include <syscmdline/global.h>

namespace SysCmdLine {

    class SharedBasePrivate;

    class SYSCMDLINE_EXPORT SharedBase {
        SYSCMDLINE_DECL_PRIVATE(SharedBase)
    public:
        virtual ~SharedBase();

        SharedBase(const SharedBase &other);
        inline SharedBase(SharedBase &&other) noexcept;
        SharedBase &operator=(const SharedBase &other);
        inline SharedBase &operator=(SharedBase &&other) noexcept;

    public:
        inline void swap(SharedBase &other) noexcept;
        bool isDetached() const;
        void detach();

    protected:
        SharedBase(SharedBasePrivate *d);
        SharedBasePrivate *d_ptr;
    };

    inline SharedBase::SharedBase(SharedBase &&other) noexcept : d_ptr(nullptr) {
        swap(other);
    }

    inline SharedBase &SharedBase::operator=(SharedBase &&other) noexcept {
        swap(other);
        return *this;
    }

    void SharedBase::swap(SharedBase &other) noexcept {
        std::swap(d_ptr, other.d_ptr);
    }

}

#endif // SHAREDBASE_H
