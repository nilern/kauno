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
class NRef {
    T* ptr_;

public:
    NRef() : ptr_(nullptr) {}

    explicit NRef(T* ptr) : ptr_(ptr) {}

    explicit NRef(ORef<T> oref) : ptr_(oref.data()) {}

    bool has_value() const { return ptr_ != nullptr; }

    optional<ORef<T>> data() const { return ptr_ ? optional(ORef(ptr_)) : optional<ORef<T>>(); }

    T* ptr() const { return ptr_; }
};

/** Pointer to data stack entry */
template<typename T>
class SRef {
    T* ptr_;

public:
    explicit SRef(T* ptr) : ptr_(ptr) {}

    T* data() const { return ptr_; }

    template<typename U>
    SRef<U> unchecked_cast() const { return SRef<U>((U*)ptr_); }
};

template<typename T>
class Handle;

/** Type stack entry */
class DynRef {
    void* sptr_;
    NRef<Type> type_;

public:
    DynRef(ORef<Type> type, void* data) : sptr_(data), type_(NRef(type)) {}

    template<typename T>
    explicit DynRef(ORef<T>* ptr) : sptr_((void*)ptr), type_() {}

    ORef<Type> type() const {
        optional<ORef<Type>> const opt_type = type_.data();
        if (opt_type.has_value()) {
            return *opt_type;
        } else {
            return unchecked_oref().type();
        }
    }

    void* data() const {
        optional<ORef<Type>> const opt_type = type_.data();
        if (opt_type.has_value()) {
            return sptr_;
        } else {
            return unchecked_oref().data();
        }
    }

    void* stack_ptr() const { return sptr_; }

    SRef<void> unchecked_sref() const;

    Handle<void> unchecked_handle() const;

    template<typename T>
    optional<SRef<T>> try_sref() const;

    template<typename T>
    optional<Handle<T>> try_handle() const;

    ORef<void> unchecked_oref() const { return *(ORef<void>*)sptr_; }

    bool is_instance_dyn(ORef<Type> super) const {
        optional<ORef<Type>> const opt_type = type_.data();
        if (opt_type.has_value()) {
            return *opt_type == super;
        } else {
            return unchecked_oref().is_instance_dyn(super);
        }
    }

    void repush(State& state) const;

    ORef<void> to_heap(State& state) const;
};

/** Pointer to type stack entry */
class AnySRef {
    DynRef* dyn_ref_ptr_;

public:
    explicit AnySRef(DynRef* dyn_ref_ptr) : dyn_ref_ptr_(dyn_ref_ptr) {}

    ORef<Type> type() const { return dyn_ref_ptr_->type(); }

    void* data() const { return dyn_ref_ptr_->data(); }

    bool is_instance_dyn(ORef<Type> super) const { return dyn_ref_ptr_->is_instance_dyn(super); }

    template<typename T>
    Handle<T> unchecked_handle() const;

    template<typename T>
    optional<SRef<T>> try_sref() const;

    template<typename T>
    optional<Handle<T>> try_handle() const;

    ORef<void> to_heap(State& state) const { return dyn_ref_ptr_->to_heap(state); }
};

/** Pointer to object reference on the data stack */
template<typename T>
class Handle {
    ORef<T>* oref_ptr_;

public:
    explicit Handle(ORef<T>* oref_ptr) : oref_ptr_(oref_ptr) {}

    ORef<T> oref() const { return oref_ptr_->template unchecked_cast<T>(); }

    T* data() const { return oref().data(); }

    template<typename U>
    Handle<U> unchecked_cast() const { return Handle<U>((ORef<U>*)oref_ptr_); }

    Handle<void> as_void() const { return unchecked_cast<void>(); }
};

SRef<void> DynRef::unchecked_sref() const { return SRef(sptr_); }

Handle<void> DynRef::unchecked_handle() const { return Handle((ORef<void>*)sptr_); }

template<typename T>
optional<SRef<T>> DynRef::try_sref() const {
    return type_.has_value() ?
                optional(unchecked_sref().unchecked_cast<T>())
              : optional<SRef<T>>();
}

template<typename T>
optional<Handle<T>> DynRef::try_handle() const {
    return type_.has_value() ?
                optional<Handle<T>>()
              : optional(unchecked_handle().unchecked_cast<T>());
}

template<typename T>
Handle<T> AnySRef::unchecked_handle() const {
    return dyn_ref_ptr_->unchecked_handle().unchecked_cast<T>();
}

template<typename T>
optional<SRef<T>> AnySRef::try_sref() const { return dyn_ref_ptr_->try_sref<T>(); }

template<typename T>
optional<Handle<T>> AnySRef::try_handle() const { return dyn_ref_ptr_->try_handle<T>(); }

struct Field {
    NRef<Type> type;
    size_t offset;
    size_t size;
    bool inlined;

    Field(NRef<Type> type_, size_t offset_);
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

Field::Field(NRef<Type> type_, size_t offset_)
    : type(type_), offset(offset_),
      size(0), inlined(false)
{
    optional<ORef<Type>> const t = type_.data();
    if (t.has_value()) {
        size = t->data()->min_size;
        inlined = t->data()->inlineable;
    }
}

struct None {};

}

#endif // OBJECT_H
