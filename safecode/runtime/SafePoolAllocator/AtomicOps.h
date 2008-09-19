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
#include <deque>

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
  __asm__("bsfl %1,%0"
	  :"=r" (word)
	  :"rm" (word));
  return word;
}

template<class Ty>
class SimpleSlabAllocator {
public:
  uint32_t m_mask;
  Ty * mMemory;
  pthread_spinlock_t mLock;
  SimpleSlabAllocator() : m_mask((uint32_t)(-1)) {
    mMemory = reinterpret_cast<Ty*>(::operator new[](sizeof(Ty) * sizeof(m_mask) * 8));
    pthread_spin_init(&mLock, false);
  }

  ~SimpleSlabAllocator() {
    pthread_spin_destroy(&mLock);
    ::operator delete[](reinterpret_cast<void*>(mMemory));
  }

  Ty * allocate() {
    pthread_spin_lock(&mLock);
    while(m_mask == 0) {
      pthread_spin_unlock(&mLock);
//      sched_yield();
      pthread_spin_lock(&mLock);
    }
    unsigned long pos = __ffs(m_mask);
    clear_bit(pos, &m_mask);
    pthread_spin_unlock(&mLock);
    return mMemory + pos;
  };

  void deallocate(Ty * ptr) {
    pthread_spin_lock(&mLock);
    uint32_t pos = ptr - mMemory;
    set_bit(pos, &m_mask);
    pthread_spin_unlock(&mLock);
  }
};

/// Based on
/// http://www.talkaboutprogramming.com/group/comp.programming.threads/messages/40308.html

template<class T, size_t N> class LockFreeFifo
{
public:
  typedef  T element_t;
  LockFreeFifo () : readidx(0), writeidx(0) {}

  T dequeue (void)
  {
    SPIN_AND_YIELD(empty());
    T result = buffer[readidx];
    readidx = (readidx + 1) % N;
    return result;
  }

  void enqueue (T datum)
  {
    unsigned newidx = (writeidx + 1) % N;
    SPIN_AND_YIELD(newidx == readidx);
    buffer[writeidx] = datum;
    writeidx = newidx;
  }

  bool empty() const {
    return readidx == writeidx;
  }

private:
  volatile unsigned  readidx, writeidx;
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
