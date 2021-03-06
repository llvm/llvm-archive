LLVM-TV: LLVM Transformation Visualizer
Written by: Misha Brukman, Tanya Brethour, and Brian Gaeke

LLVM-TV is a tool that can be used to visualize the effects of transformations
written in the LLVM framework. Compilation units in LLVM have a simple,
hierarchical structure: a Module contains Functions, which contain BasicBlocks,
which contain Instructions. At the lowest level of this hierarchy, Instructions
may reference other BasicBlocks (with branches) or Functions (with calls),
making the control-flow graphs and call graphs explicit. Our first task is to
develop an interactive browser for these graphs and hierarchies.

Our target audience is compiler developers working within the LLVM framework,
who are trying to understand and debug transformations.

Currently, the visualization tool will not reflect dynamic updates
to the code; rather, it reflects the state of a compilation unit at
a single instant in time, between transformations. Future work should
extend this tool to reflect a dynamically updating view of the program as
a transformation is modifying the code.

How to compile:

1. You must have wxWidgets installed on your system, and wx-config has to be
   in your path.

   Make absolutely sure that wxWidgets' configure picks up the same
   C++ compiler that you're using for llvm. Otherwise, you may get
   weird link errors when trying to link the llvm-tv tool.

2. Check out LLVM:

# We're checking out this version and not trunk since poolalloc doesn't build
# with top of trunk LLVM.
% svn -q co -r 78786 http://llvm.org/svn/llvm-project/llvm/trunk llvm

3. Configure and build LLVM:

% cd path/to/llvm-obj
% ~/llvm/configure [configure opts]
% make

4. Check out PoolAlloc (we need it for the DataStructure library):

% svn -q co http://llvm.org/svn/llvm-project/poolalloc/trunk poolalloc

5. Configure and build PoolAlloc:

% cd path/to/poolalloc-obj
% ~/poolalloc/configure --with-llvmsrc=[path] \
                        --with-llvmobj=[path]
% make

2. Configure and compile llvm-tv:

% cd path/to/llvm-tv-obj
# If you're building in llvm/projects/llvm-tv, then you don't need
# to specify the --with-llvm-* options.
% ~/llvm-tv/configure --with-llvm-src=[path] --with-llvm-obj=[path] \
                      --with-poolalloc-src=[path] --with-poolalloc-obj=[path]
% make

Note: the llvm-tv.exe and opt-snap scripts will be placed into LLVM's binary
directory, not the LLVM-TV binary directory.

Example of usage:

% llvm-tv.exe &
   The .exe is not a typo; this command starts up the visualizer in
   the background using its wrapper script.
% opt-snap -debug -licm -snapshot -gcse -snapshot < bytecode-file.bc > /dev/null
   This runs the llvm optimizer driver with the snapshot pass loaded, using
   another wrapper script, and makes two snapshots, which should appear
   in your visualizer.

