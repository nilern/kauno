#ifndef OBJECT_H
#define OBJECT_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>

struct State;

struct Header {
    uintptr_t bits;
};

struct Any;

struct Type;

template<typename T>
class ORef {
    T* ptr_;

public:
    explicit ORef(T* ptr) : ptr_(ptr) {}

    Header* header() const { return (Header*)ptr_ - 1; }

    bool is_marked() const { return header()->bits & 1; }

    void set_marked() const { header()->bits |= 1; }

    ORef<Type> type() const { return ORef<Type>((Type*)(void*)(header()->bits & ~1)); }

    void set_type(ORef<Type> type) const { header()->bits = (size_t)type.data() | (header()->bits & 1); }

    T* data() const { return ptr_; }

    bool operator==(ORef<T> other) const { return ptr_ == other.ptr_; }

    template<typename U>
    ORef<U> unchecked_cast() const { return ORef<U>((U*)ptr_); }

    ORef<Any> as_any() const { return unchecked_cast<Any>(); }
};

template<typename T>
class Handle {
    ORef<T>* oref_ptr_;

public:
    explicit Handle(ORef<T>* oref_ptr) : oref_ptr_(oref_ptr) {}

    ORef<T> oref() const { return *oref_ptr_; }

    ORef<Type> type() const { return oref().type(); }

    T* data() const { return oref().data(); }

    template<typename U>
    Handle<U> unchecked_cast() const { return Handle<U>((ORef<U>*)oref_ptr_); }

    Handle<Any> as_any() const { return unchecked_cast<Any>(); }
};

struct Field {
    size_t offset;
    ORef<Type> type;
};

struct Type {
    size_t align;
    size_t min_size;
    bool inlineable;
    bool is_bits;
    bool has_indexed;
    size_t fields_count; // if is_bits then byte count else field_types count
    Field fields[];
};

#endif // OBJECT_H
