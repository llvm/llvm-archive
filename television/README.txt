LLVM Visualization Tool
CS497rej Spring 2004 term project
Team members: Misha Brukman, Tanya Brethour, Brian Gaeke

The Low Level Virtual Machine (LLVM) is a new compiler infrastructure that
uses a typed, three-address low-level intermediate code representation for
programs written in higher-level languages, making optimizations possible
across the entire lifetime of a program: at compile time, link time,
install time, runtime, and idle time.

Our main goal in this project is to build a tool that can be used
to visualize the effects of transformations written in the LLVM
framework. Compilation units in LLVM have a simple, hierarchical structure:
a Module contains Functions, which contain BasicBlocks, which contain
Instructions. At the lowest level of this hierarchy, Instructions may
reference other BasicBlocks (with branches) or Functions (with calls),
making the control-flow graphs and call graphs explicit. Our first task
is to develop an interactive browser for these graphs and hierarchies.

Our target audience is compiler developers working within the LLVM framework,
who are trying to understand and debug transformations.

For this semester, our visualization tool will not reflect dynamic updates
to the code; rather, it will reflect the state of a compilation unit at
a single instant in time, between transformations. Future work should
extend this tool to reflect a dynamically updating view of the program as
a transformation is modifying the code.

We intend to implement this tool in C++, because it will dramatically
simplify the task of integrating it with the LLVM compiler framework
(which itself is 150,000 lines of C++). We intend to use wxWidgets as our
GUI toolkit.

How to compile:

% ./configure --with-llvmsrc=[path] --with-llvmobj=[path]
   If you're building in llvm/projects/llvm-tv, then you don't need
   to specify these --with options.
% cd lib/wxwindows
% ./configure --enable-debug --prefix=`pwd`
   Make absolutely sure that wxwindows's configure picks up the same
   C++ compiler that you're using for llvm. Otherwise, you may get
   weird link errors when trying to link the llvm-tv tool.
% cd ../../
% gmake

Example of usage:
 
% llvm-tv.exe &
   The .exe is not a typo; this command starts up the visualizer in
   the background using its wrapper script.
% opt-snap -debug -licm -snapshot -gcse -snapshot < bytecode-file.bc > /dev/null
   This runs the llvm optimizer driver with the snapshot pass loaded, using
   another wrapper script, and makes two snapshots, which should appear
   in your visualizer.

