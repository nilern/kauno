# Kauno Design

## Language Overview

Dynamically typed functional-first language with

* Modules
    - Recursive modules
* First-class functions with
    - Arity and parameter type checking
    - Type parameters (`pure<T = Option>()`) (Common Lisp (eql specializers), Julia (`Type{Int}`))
    - Both mostly to facilitate use as multimethod methods
* Multimethods with
    - Templated methods (`forall T . get(v : Array<T>, i : USize)`) (Julia)
* Types with
    - Abstract, bits and composite types (Julia)
    - Access-controllable (via modules) constructors and field accessors (Scheme)
    - Data inlining (Project Valhalla)
    - Optional indexed final field (C flexible struct member)
* Multiple (return) values (Scheme, Lua)
* Extensible effects / delimited continuations (Racket)
* Hygienic macros (Scheme)

no

* Implicit conversions (bug-inducing)
* Accepting more/less arguments than arity (bug-inducing)
* Arity overloading or vararg functions (bug-inducing, possibly slow)
* Method or field access by name from anywhere (abstraction-breaking, slow)
* Inheritance from concrete types (abstraction-breaking, prevents data inlining)
* Forward declarations (archaic and annoying)

## Implementation Overview

* Bytecode VM
    - Bytecode more like Self than Lua 5
    - Self-hosted bytecode compiler
* Simple self-hosted method JIT
* AST interpreter for bootstrapping

## Grammar

    expr ::= 'fn' '(' (param (',' param)*)? '->' expr
           | call

    call ::= callee '(' (expr (',' expr)*)? ')'
           | callee

    callee ::= '(' expr ')'
             | VAR
             | const

    const ::= INT

    param ::= VAR (':' expr)

### LL(1) Grammar

    expr ::= 'fn' '(' (param (',' param)*)? '->' expr
           | call

    call ::= callee ('(' (expr (',' expr)*)? ')')?

    callee ::= '(' expr ')'
             | VAR
             | const

    const ::= INT

    param ::= VAR (':' expr)

## Types

    abstype Type {
        super : Option<Type>;
        align : USize;
    };

    abstype AbstractType <: Type {
        super : Option<Type>;
        align : USize;
        indexed fields : Field;
    };

Abstract types cannot be instantiated and are never inlineable because their subtypes can have
differing sizes.

Abstract types can have fields and an alignment requirement. If an abstract type has fields, it
cannot be inherited by a `BitsType`. Fielded abstract types cannot be instantiated or inlined
any more than fieldless ones.

    abstype ConcreteType <: Type {
        super : Option<Type>;
        align : USize;
    };

    record BitsType <: ConcreteType {
        super : Option<Type>;
        align : USize;
        size : USize;
        inlineable : Bool;
    };

`inlineable` is true if the bits are immutable (and not e.g. an opaque FFI struct in disguise).

    abstype CompositeType <: ConcreteType {
        super : Option<Type>;
        align : USize;
    };

    record RecordType <: CompositeType {
        super : Option<Type>;
        align : USize;
        size : USize;
        inlineable : Bool;
        indexed fields : Field;
    };

`inlineable` is true if there are no mutable fields.

    record IndexedRecordType <: CompositeType {
        super : Option<Type>;
        align : USize;
        minSize : USize;
        indexed fields : Field;
    };

Indexed record types are never inlineable because their size depends on the count of indexed elements.

The last field is the indexed one. Surface syntax will make it possible to have any single field be
indexed, but the runtime lays it out last. Fields will be otherwise reordered for efficiency too,
except for FFI where the foreign layout obviously has to be matched exactly.

    record Field {
        type : Type;
        offset : USize;
        size : USize;
        inlined : Bool;
        mutable : Bool;
    };

`size` is just copied from `type` to reduce indirection.

`inlined` is true if `type` is inlineable.

While mutable types may not be inlined, types may be inlined into mutable fields.

## Accessors

Instead of by name, fields are accessed via accessor values, which can be module-public or private
like any value.

    record FieldAccessor {
        type : CompositeType
        index : USize;
    }

Immutable fields may be read and updated (allocating a new updated value). Mutable fields may
also be set in place. Indexed fields require the index in addition to the accessor and accessee.

Like calls, field accesses will be inline cached. This is simple since types, fields and accessors are
all immutable.

Field accessors for parameterized types don't need any extra logic; the accessee is of an instantiated type
and that type contains the specialized `Field`s.
