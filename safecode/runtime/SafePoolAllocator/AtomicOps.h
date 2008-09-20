//=== AtomicOps.h --- Declare atomic operation primitives -------*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file declares synchronization primitives used in speculative checking.
//
//===----------------------------------------------------------------------===//

#ifndef _ATOMIC_OPS_H_
#define _ATOMIC_OPS_H_

#include <pthread.h>
#include <semaphore.h>
#include <cassert>
#include <errno.h>
#include "Config.h"
#include <stdint.h>

NAMESPACE_SC_BEGIN

#define LOCK_PREFIX "lock "
#define ADDR "+m" (*(volatile long *) addr)

#define SPIN_AND_YIELD(COND) do { int counter = 0; \
  while (COND) { if (++counter == 1024) { sched_yield(); counter = 0;} } \
  } while (0)

/// FIXME: These codes are from linux header file, it should be rewritten
/// to avoid license issues.
/// JUST FOR EXPERIMENTAL USE!!!

static inline void clear_bit(int nr, volatile void *addr)
{
  asm volatile(LOCK_PREFIX "btr %1,%0"
	       : ADDR
	       : "Ir" (nr));
}
static inline void set_bit(int nr, volatile void *addr)
{
  asm volatile(LOCK_PREFIX "bts %1,%0"
	       : ADDR
	       : "Ir" (nr) : "memory");
}
/**

* __ffs - find first bit in word.
* @word: The word to search
*
* Undefined if no bit exists, so code should check against 0 first.
*/
static inline unsigned long __ffs(unsigned long word)
{
  __asm__( "bsfl %1,%0"
	  :"=r" (word)
	  :"rm" (word));
  return word;
}

struct __xchg_dummy {
	unsigned long a[100];
};

#define __xg(x) ((struct __xchg_dummy *)(x))


static inline unsigned long __cmpxchg(volatile void *ptr, unsigned long old,
				      unsigned long newval, int size)
{
	unsigned long prev;
	switch (size) {
	case 1:
		asm volatile(LOCK_PREFIX "cmpxchgb %b1,%2"
			     : "=a"(prev)
			     : "q"(newval), "m"(*__xg(ptr)), "0"(old)
			     : "memory");
		return prev;
	case 2:
		asm volatile(LOCK_PREFIX "cmpxchgw %w1,%2"
			     : "=a"(prev)
			     : "r"(newval), "m"(*__xg(ptr)), "0"(old)
			     : "memory");
		return prev;
	case 4:
		asm volatile(LOCK_PREFIX "cmpxchgl %1,%2"
			     : "=a"(prev)
			     : "r"(newval), "m"(*__xg(ptr)), "0"(old)
			     : "memory");
		return prev;
	}
	return old;
}

/* Copied from include/asm-x86_64 for use by userspace. */
#define mb()    asm volatile("mfence":::"memory")
/// A very simple allocator works on single-reader / single-writer cases

template<class Ty>
class SimpleSlabAllocator {
public:
  uint32_t m_mask;
  Ty * mMemory;
  SimpleSlabAllocator() : m_mask((uint32_t)(-1)) {
    mMemory = reinterpret_cast<Ty*>(::operator new[](sizeof(Ty) * sizeof(m_mask) * 8));
  }

  ~SimpleSlabAllocator() {
    ::operator delete[](reinterpret_cast<void*>(mMemory));
  }

  Ty * allocate() {
    SPIN_AND_YIELD(m_mask == 0);
    // Since there is only one thread for allocation, 
    // we don't need to lock here
    unsigned long pos = __ffs(m_mask);
    // clear_bit has lock prefix so we don't need a mb
    clear_bit(pos, &m_mask);
    return mMemory + pos;
  };

  void deallocate(Ty * ptr) {
    uint32_t pos = ptr - mMemory;
    // set_bit has lock prefix so we don't need a mb
    set_bit(pos, &m_mask);
  }
};

/// Based on
/// http://www.talkaboutprogramming.com/group/comp.programming.threads/messages/40308.html
/// Only for single-reader, single-writer cases.

template<class T, size_t N> class LockFreeFifo
{
public:
  typedef  T element_t;
  LockFreeFifo () : readidx(0), writeidx(0) {}

  T dequeue (void)
  {
    SPIN_AND_YIELD(empty());
    T result = buffer[readidx];
    mb();
    readidx = (readidx + 1) % N;
    return result;
  }

  void enqueue (T datum)
  {
    unsigned newidx = (writeidx + 1) % N;
    SPIN_AND_YIELD(newidx == readidx);
    buffer[writeidx] = datum;
    mb();
    writeidx = newidx;
  }

  bool empty() const {
    return readidx == writeidx;
  }

private:
  volatile unsigned readidx, writeidx;
  T buffer[N];
};

template <class QueueTy, class FuncTy>
class Task {
public:
  typedef typename QueueTy::element_t ElemTy;
  Task(QueueTy & queue) : mQueue(queue), mActive(false) {}
  void activate() {
    mActive = true;
    typedef void * (*start_routine_t)(void*);
    pthread_t thr;
    pthread_create(&thr, NULL, (start_routine_t)(&Task::runHelper), this);
  };

  void stop() {
    mActive = false;
  };

  QueueTy & getQueue() const {
    return mQueue;
  };

private:
  static void * runHelper(Task * this_) {
    this_->run();
    return NULL;
  };

  void run() {
    while(true) {
      typename QueueTy::element_t e = mQueue.dequeue();
      if (mActive) {
	mFunctor(e);
      } else {
	break;
      }
    }
  };

  QueueTy & mQueue;
  FuncTy mFunctor;
  bool mActive;
};

NAMESPACE_SC_END

#endif
