HLVM Runtime Library
====================

This directory contains the HLVM Runtime Library. In order to avoid many linking
and other problems, it has a number of rules that must be followed.

1. No exposed C++
-----------------
Although the source files are C++, the library cannot expose any C++ to its 
users. Its interface is C language. Header files must be compilable with a plain
C compiler. Implementation can use C++ features, but with caveats.

2. C++ Implementation In Anonymous Namespace
--------------------------------------------
Any C++ used in a .cpp file must be in the anonymous namespace. This makes it
local (static) to the compilation unit and completely hidden from the users of
the runtime.

3. extern "C" usage
-------------------
The C++ extern "C" facility may not be used in Runtime headers. Instead, 
implementation files should wrap the #include statements like this:

extern "C" {
#include <hlvm/Runtime/Program.h>
}

4. No Static Construction
-------------------------
The implementation files must avoid static construction. This helps to ensure
fast startup times and avoids linking errors when llvm-g++ optimizes out things

5. No libraries but APR
-----------------------
The Runtime library is intended to link against APR and APRUTIL only. No other
linkage requirements are permitted.

6. No System Headers
--------------------
The Runtime library must not #include any system headers. Instead it should use
the facilities of the APR or APRUTIL libraries.

7. Three Layers
------------------------
There are three layers of abstraction in the runtime. The first layer is what
user programs can see. At this level, most everything is a void*. This is so
that the implementation choices are not restricted and so that runtime
implementation can change without affecting its users. 

The second layer is that which is common to the runtime. That is, between
compilation units of the runtime itself. At this level, most of the types are
defined as simple structs which allow access to data members between modules.

The third layer is that which is internal to the defining module.  Each module
might decide to embellish the inter-module definition of a given type,
including by making it a C++ class. None of this can be exposed outside the
module.
