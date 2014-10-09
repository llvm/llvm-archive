
#ifndef _VMKIT_STACK_WALKER_H_
#define _VMKIT_STACK_WALKER_H_

namespace vmkit {

class Thread;
class KnownFrame;
class FrameInfo;

/// StackWalker - This class walks the stack of threads, returning a FrameInfo
/// object at each iteration.
///
class StackWalker
{
public:
  word_t callFrameAddress;
  word_t returnAddress;
  KnownFrame* frame;
  Thread* thread;

  StackWalker() __attribute__ ((noinline));
  StackWalker(Thread* th) __attribute__ ((noinline));
  void operator++();
  word_t operator*();
  FrameInfo* get();

  FrameInfo* getNextFrameWithMetadata();
  FrameInfo* getNextFrameWithMetadata(size_t& totalFrameSizeWithoutMetadata);
  FrameInfo* getNextFrameWithMetadata(
    word_t& callFrameAddress, word_t& returnAddress, KnownFrame*& frame);
};

}

#endif
