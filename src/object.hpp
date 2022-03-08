#ifndef OBJECT_H
#define OBJECT_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>

using std::optional;

namespace kauno {

class State;

struct Header {
    uintptr_t bits;
};

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

    bool operator!=(ORef<T> other) const { return ptr_ != other.ptr_; }

    bool is_instance_dyn(ORef<Type> super) const { return type() == super; }

    template<typename S>
    bool is_instance(State const& state) const { return is_instance_dyn(S::reify(state)); }

    template<typename U>
    ORef<U> unchecked_cast() const { return ORef<U>((U*)ptr_); }

    template<typename U>
    optional<ORef<U>> try_cast(State const& state) const {
        return is_instance<U>(state) ? optional(unchecked_cast<U>()) : optional<ORef<U>>();
    }

    ORef<void> as_void() const { return unchecked_cast<void>(); }
};

template<typename T>
class Handle {
    ORef<T>* oref_ptr_;

public:
    explicit Handle(ORef<T>* oref_ptr) : oref_ptr_(oref_ptr) {}

    ORef<T> oref() const { return *oref_ptr_; }

    ORef<T>* oref_ptr() const { return oref_ptr_; }

    ORef<Type> type() const { return oref().type(); }

    T* data() const { return oref().data(); }

    bool is_instance_dyn(ORef<Type> super) const { return oref().is_instance_dyn(super); }

    template<typename S>
    bool is_instance(State const& state) const { return oref().template is_instance<S>(state); }

    template<typename U>
    Handle<U> unchecked_cast() const { return Handle<U>((ORef<U>*)oref_ptr_); }

    template<typename U>
    optional<Handle<U>> try_cast(State const& state) const {
        return is_instance<U>(state) ? optional(unchecked_cast<U>()) : optional<Handle<U>>();
    }

    Handle<void> as_void() const { return unchecked_cast<void>(); }
};

struct Field {
    ORef<Type> type;
    size_t offset;
    size_t size;
    bool inlined;

    Field(ORef<Type> type_, size_t offset_);
};

struct Type {
    size_t hash;
    size_t align;
    size_t min_size;
    bool inlineable;
    bool is_bits;
    bool has_indexed;
    size_t fields_count; // if is_bits then byte count else field_types count
    Field fields[0];

    static ORef<Type> reify(State const& state);

private:
    Type(State& state,
         size_t align_, size_t min_size_, bool inlineable_, bool is_bits_, bool has_indexed_, size_t fields_count_);

public:
    static Type create_bits(State& state, size_t align, size_t size, bool inlineable) {
        return Type(state, align, size, inlineable, true, false, 0);
    }

    static Type create_record(State& state, size_t align, size_t size, bool inlineable, size_t fields_count) {
        return Type(state, align, size, inlineable, false, false, fields_count);
    }

    static Type create_indexed(State& state, size_t align, size_t min_size, size_t fields_count) {
        return Type(state, align, min_size, false, false, true, fields_count);
    }
};

Field::Field(ORef<Type> type_, size_t offset_)
    : type(type_), offset(offset_),
      size(0), inlined(false)
{
    Type* const t = type_.data();
    if (t) {
        size = t->min_size;
        inlined = t->inlineable;
    }
}

struct None {};

}

#endif // OBJECT_H
