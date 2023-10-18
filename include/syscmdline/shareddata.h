#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include <atomic>
#include <utility>

namespace SysCmdLine {

    template <class T>
    class SharedDataPointer;

    class SharedData {
    public:
        mutable std::atomic_int ref;

        inline SharedData() noexcept : ref(0) {
        }
        SharedData(const SharedData &) = delete;

        // using the assignment operator would lead to corruption in the ref-counting
        SharedData &operator=(const SharedData &) = delete;
        virtual ~SharedData() = default;
    };

    template <class T>
    class SharedDataPointer {
    public:
        typedef T Type;
        typedef T *pointer;

        inline void detach() {
            if (d && d->ref.load() != 1)
                detach_helper();
        }
        inline T &operator*() {
            detach();
            return *d;
        }
        inline const T &operator*() const {
            return *d;
        }
        inline T *operator->() {
            detach();
            return d;
        }
        inline const T *operator->() const {
            return d;
        }
        inline operator T *() {
            detach();
            return d;
        }
        inline operator const T *() const {
            return d;
        }
        inline T *data() {
            detach();
            return d;
        }
        inline const T *data() const {
            return d;
        }
        inline const T *constData() const {
            return d;
        }

        inline bool operator==(const SharedDataPointer<T> &other) const {
            return d == other.d;
        }
        inline bool operator!=(const SharedDataPointer<T> &other) const {
            return d != other.d;
        }

        inline SharedDataPointer() {
            d = nullptr;
        }
        inline ~SharedDataPointer() {
            if (d && d->ref.fetch_sub(1) == 1)
                delete d;
        }

        explicit SharedDataPointer(T *data) noexcept;
        inline SharedDataPointer(const SharedDataPointer<T> &o) : d(o.d) {
            if (d)
                d->ref.fetch_add(1);
        }
        inline SharedDataPointer<T> &operator=(const SharedDataPointer<T> &o) {
            if (o.d != d) {
                if (o.d)
                    o.d->ref.fetch_add(1);
                T *old = d;
                d = o.d;
                if (old && old->ref.fetch_sub(1) == 1)
                    delete old;
            }
            return *this;
        }
        inline SharedDataPointer &operator=(T *o) {
            if (o != d) {
                if (o)
                    o->ref.fetch_add(1);
                T *old = d;
                d = o;
                if (old && old->ref.fetch_sub(1) == 1)
                    delete old;
            }
            return *this;
        }
        SharedDataPointer(SharedDataPointer &&o) noexcept : d(o.d) {
            o.d = nullptr;
        }
        inline SharedDataPointer<T> &operator=(SharedDataPointer<T> &&other) noexcept {
            SharedDataPointer moved(std::move(other));
            swap(moved);
            return *this;
        }

        inline bool operator!() const {
            return !d;
        }

        inline void swap(SharedDataPointer &other) noexcept {
            std::swap(d, other.d);
        }

    protected:
        T *clone();

    private:
        void detach_helper();

        T *d;
    };

    template <class T>
    inline bool operator==(std::nullptr_t p1, const SharedDataPointer<T> &p2) {
        (void) p1;
        return !p2;
    }

    template <class T>
    inline bool operator==(const SharedDataPointer<T> &p1, std::nullptr_t p2) {
        (void) p2;
        return !p1;
    }

    template <class T>
    inline SharedDataPointer<T>::SharedDataPointer(T *adata) noexcept : d(adata) {
        if (d)
            d->ref.fetch_add(1);
    }

    template <class T>
    inline T *SharedDataPointer<T>::clone() {
        return d->clone();
    }

    template <class T>
    inline void SharedDataPointer<T>::detach_helper() {
        T *x = clone();
        x->ref.fetch_add(1);
        if (d->ref.fetch_sub(1) == 1)
            delete d;
        d = x;
    }

#define SYSCMDLINE_GET_DATA(T)                                                                     \
    T##Data *const d = reinterpret_cast<T##Data *>(d_ptr.data())

#define SYSCMDLINE_GET_CONST_DATA(T)                                                               \
    const T##Data *const d = reinterpret_cast<const T##Data *>(d_ptr.constData());

}

#endif // SHAREDDATA_H
