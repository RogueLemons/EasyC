# EasyC (Prototype)
Ease-of-life features added in a simple proof-of-concept superset-transpiler to C.

Includes:
- keyword **typestruct**: automatically creates typedef of struct
- keyword **indef**: to write define statements inside of functions to keep local values local
- keyword **prefix**: working similar to C++ namespace to avoid long struct and function names
- keyword **mut**: working as an inverted const, where everything is const by default unless mut is used
- keyword **safe**: if type is safe then EC__CHECK__NULL(x) automatically called on next line (macro with default behavior, can be overridden)
- keyword **typenum**: a typesafe enum with a struct under hood with macro definitions for values, if type has option = 5 then access with type::option, type::count and type::get added for bonus help, uses typenum::equals instead of ==

# TODO
- keyword **cleanpop**: variable initialized with cleanpop automatically calls POPULATE__type(t) on next line and before scope exits (and before return statements) calls CLEANUP__type(t), must be defined as functions or macros manually
- invert keyword **safe** so everything is safe by default and make user use keyword **nullable** for pointers that may be null
- convert .eh and .ec file includes to .h and .c
- keyword **typenum**: add ability to assign internal type e.g. typenum(char) Status { OK = 0, Error = 1 };

# Bugs
As a prototype this mini-project will never be perfect, it is a proof of concept. But less acceptable bugs include
- cannot use * without whitespace after unless dereferencing (* some_ptr not ok) or multiplying (a * b not ok)
- cannot typedef and define struct in same statement
- unnecessary const in non-pointer return types
