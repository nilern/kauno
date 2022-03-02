# Kauno Design

## Language Overview

Dynamically typed functional-first language with

* Modules
    - Recursive modules
* First-class functions with
    - Arity and parameter type checking (mostly to facilitate use as multimethod methods)
* Multimethods with
    - Type parameters (`pure{Option}()`) (CLOS (eql specializers), Julia (`Type{Int}`))
    - Templated methods (`forall T . get(v : Array{T}, i : USize)`) (Julia)
* Dynamic type classes (built on top of multimethods)
* Types with
    - Both composite and bits types (Julia)
    - Access-controllable (via modules) constructors and field accessors (Scheme)
    - Data inlining (Project Valhalla)
    - Optional indexed final field (C flexible struct member)
* Multiple (return) values (Scheme, Lua)
* Extensible effects / delimited continuations (Racket)
* Hygienic macros (Scheme)

no

* Classical or prototypical OO (complecting)
* Pointer equality (low-level, prevents data inlining)
* Implicit conversions (bug-inducing)
* Accepting more/less arguments than arity (bug-inducing)
* Arity overloading or vararg functions (bug-inducing, possibly slow)
* Method or field access by name from anywhere (abstraction-breaking, slow)
* Inheritance from concrete types (abstraction-breaking, prevents data inlining)
* Forward declarations (archaic and annoying)
* Importing everything in a module (hard to follow for both programmers and compilers)
* Subtyping and overriding

## Implementation Overview

* Bytecode VM
    - Bytecode more like Self than Lua 5
    - Self-hosted bytecode compiler
* Simple self-hosted method JIT
* AST interpreter for bootstrapping

## Grammar

    expr ::= ('forall' '(' VAR (',' VAR)*) ')')? 'fn' typeArgs? '(' (param (',' param)*)? ')' '->' expr
           | call

    call ::= callee typeValueArgs? args
           | callee typeValueArgs
           | callee

    callee ::= '(' expr ')'
             | VAR
             | const

    const ::= INT

    typeValueArgs ::= '{' expr (',' expr)* '}'

    param ::= VAR (':' type)

    args ::= '(' (expr (',' expr)*)? ')'

    type ::= type typeArgs
           | typeCallee

    typeCallee ::= typeCallee '::' VAR
                 | VAR

    typeArgs ::= '{' type (',' type)* '}'

### LL(1) Grammar

    expr ::= ('forall' '(' VAR (',' VAR)*) ')')? 'fn' typeArgs? '(' (param (',' param)*)? ')' '->' expr
           | call

    call ::= callee typeValueArgs? args?

    callee ::= '(' expr ')'
             | VAR
             | const

    const ::= INT

    typeValueArgs ::= '{' expr (',' expr)* '}'

    param ::= VAR (':' type)

    args ::= '(' (expr (',' expr)*)? ')'

    type ::= VAR ('::' VAR)* ('{' type (',' type)* '}')*

## Templates

A **template** of arity *n* is a pure function from type sequences of length *n* to values.

A template is **instantiable** to a value if its parameters may be replaced with concrete types
in the template body so that the body becomes equal to that value. This test may be implemented
by replacing the parameters with unification variables and then unifying the resulting body
and the value.

Two templates **overlap** if their parameters may be replaced with concrete types so that their bodies
become equal. This test may be implemented by replacing the parameters with unification variables and
then unifying the resulting bodies.

## Dispatchers

A **dispatcher** of arity *n* is a mapping from type sequences of length *n* to values. The dispatcher
entries may also be templates (whose domains can be distinct from the dispatcher domain).

Dispatcher keys must be distinct; no nontemplated key sequence shall be equal to another, no templated
entry shall be instantiable to a nontemplated key sequence and no templated entry shall overlap another.
This makes method selection unambiguous and fast but dispatcher extension slow, which is the correct
tradeoff.

## Signatures

A **signature** consists of two type sequences: one for type parameters/arguments and one for value
parameter/argument types. The lengths of those sequences are called the **arities** of the signature.

## Functions

    ('forall' '(' VAR (',' VAR)*) ')')? 'fn' typeArgs? '(' (param (',' param)*)? ')' '->' expr

    callee typeArgs? args

A function has a number of parameters, each of which may have a type that the argument is checked against
at call time. There are no varargs or keyword parameters; instead, arrays or maps must be passed
explicitly.

`typeArgs` only exist to make functions usable as methods of multimethods with type parameters; clearly
it is not really useful to specify that a function takes type arguments that must be equal to given types
and are not even bound to any variable in the value parameters or body.

`forall` makes the function a memoized template from `universals` to monomorphic functions. The arguments
for `universal` need not and in fact cannot be passed explicitly, but are determined via template
instantiation.

### CheckArities

(After the callee and arguments have been evaluated:)

1. Compute the argument signature.
2. Check that the parameter and argument signature arities match. If not, signal an error.

### DoCallFunction

(After the callee and arguments have been evaluated and the arguments checked:)

1. Extend the closure environment with the parameters bound to the arguments.
2. Evaluate the function body in that environment.

### CallFunction

(After the callee and arguments have been evaluated:)

1. **CheckArities**
2. Check that the argument signature types are equal to the parameter signature types. If not, signal an
   error.
3. **DoCallFunction**

### CallTemplatedFunction

(After the callee and arguments have been evaluated:)

1. **CheckArities**
2. Make a cache lookup with the argument signature.
3. If successful, **DoCallFunction** the found function and return the result.
4. Else **InstantiateTemplatedFunction** and **DoCallFunction** the resulting function.

### InstantiateTemplatedFunction

(Assuming that the parameter and argument signature arities (numbers of type and value parameters/arguments)
match:)

1. Extend the environment of the functions definition site with the universal variables bound to unification
   variables.
2. Symbolically evaluate (i.e. don't compute sizes etc.) the function signature in that environment.
3. Unify the resulting symbolic types with the call signature. If unification fails, signal an error.
4. Extract the unification variable values. If a unification variable was left undefined (e.g. not mentioned
   in the function signature), signal an error.
5. Rebind the universals to the extracted unification variable values.
6. Evaluate the `fn` expression in that environment, add the result to the cache and return it.

## Multimethods

### CallMultiMethod

(After the callee and arguments have been evaluated:)

1. **CheckArities**
2. Make a cache lookup with the argument signature.
3. If successful, **DoCallFunction** the found function and return the result.
4. Else for all template methods:
    1. Try **InstantiateTemplatedFunction**
    2. If successful **DoCallFunction** the resulting function and return the result.
    3. Else continue loop.
6. If we get here, no suitable method exists; signal an error.

## Types

    record Type {
        align : USize;
        minSize : USize;
        isBits : Bool;
        mutable : Bool;
        inlineable : Bool;
        indexed fields : Field;
    }

`mutable` is true if there are mutable fields or `isBits` and the bits are mutable
(e.g. an opaque FFI struct in disguise).

`inlineable` is true if `mutable` is false (updates could not be propagated to all copies efficiently)
and there are no indexed (instances differ in size) fields.

Any single field can be indexed, but the runtime lays it out last. Fields will be otherwise reordered
for efficiency too (minimizing alignment padding), except for FFI where the foreign layout obviously
has to be matched exactly. For parameterized types the layout order may differ by instance type but
the order of `fields` will not.

    record Field {
        type : Type;
        offset : USize;
        size : USize;
        inlined : Bool;
        mutable : Bool;
        indexed : Bool;
    };

`type` is null (and `inlined` false) for polymorphic fields.

Otherwise `size` and `inlined` are just copied from `type` to reduce indirection.

While mutable types may not be inlined, types may be inlined into mutable fields.

## Constructors

Constructors are autogenerated normal functions whose parameters correspond to the fields and whose
domain is the sequence of field types.

## Accessors

Instead of by name, fields are accessed via accessor values, which can be module-public or private
like any value.

    record FieldAccessor {
        type : Set{Type}
        index : USize;
    }

Immutable fields may be read and updated (allocating a new updated value). Mutable fields may
also be set in place. Indexed fields require the index in addition to the accessor and accessee.

Like calls, field accesses will be inline cached. This is simple since types, fields and accessors are
all immutable.

## Parameterized Types

A parameterized type (= type constructor) is a memoized template to concrete types.

Its constructor is a memoized template from the the type constructor parameters to constructor functions
of concrete types.

Field accessors for parameterized types are normal acessors; the accessee is of an instantiated type
and that type contains the specialized `Field`s.
