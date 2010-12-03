This is an import of the GCC testsuite for use in testing Clang. It was cloned
from Apple GCC 4.2 rev 155935.

Testing
-------

You can run the entire test suite with:

    make check

This is a convenience target which runs:

    make check-gcc 
    make check-g++ 
    make check-objc 
    make check-obj-c++

which test the C, C++, Objective-C, and Objective-C++ languages in that order.


You can run an individual test with, e.g.:

    make check-gcc RUNTESTFLAGS=dg.exp=local4.C

You need to supply the right `testsuite.exp` filename and test name.


TODO
-----

large-size-array.c is forcibly disabled, clang goes memory crazy on it.