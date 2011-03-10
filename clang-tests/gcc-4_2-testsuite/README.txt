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

Note that by default these also use the CCC_ADD_ARGS environment variable to
override a variety of Clang defaults which otherwise tend to confuse the GCC
test suite's expectations. See the Makefile for more details.


You can run an individual test with, e.g.:

    make check-gcc RUNTESTFLAGS=dg.exp=local4.C

You need to supply the right `testsuite.exp` filename and test name. Generally,
you want the .exp file which is in the same directory as the individual test you
are trying to run.

You can override the compiler under test using, e.g.:

    make check-gcc RUNTESTFLAGS=dg.exp=local4.C CC_UNDER_TEST=$(which clang) CXX_UNDER_TEST=$(which clang++)

