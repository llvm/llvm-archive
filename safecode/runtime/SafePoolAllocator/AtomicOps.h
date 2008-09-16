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
#include "Config.h"

NAMESPACE_SC_BEGIN

/// Implementation of a circular queue
/// TODO: use lock-free alogirthm to improve the performance

template<class Ty, size_t N>
class CircularQueue {
public:
  typedef Ty element_t;
  void enqueue(const Ty & elem) {
    sem_wait(&mSemQueueNotFull);
    pthread_mutex_lock(&mLock);    
    mTail = next(mTail);
    mQueue[mTail] = elem;
    pthread_mutex_unlock(&mLock);
    sem_post(&mSemQueueNotEmpty);
  };

  void dequeue(Ty & elem) {
    sem_wait(&mSemQueueNotEmpty);
    pthread_mutex_lock(&mLock);    
    elem = mQueue[mHead];
    mHead = next(mHead);    
    pthread_mutex_unlock(&mLock);
    sem_post(&mSemQueueNotFull);
  };

  CircularQueue() : mHead(0), mTail(N - 1) {
    sem_init(&mSemQueueNotFull, false, N);
    sem_init(&mSemQueueNotEmpty, false, 0);
  };

  ~CircularQueue() {
    pthread_mutex_destroy(&mLock);
    sem_destroy(&mSemQueueNotFull);
    sem_destroy(&mSemQueueNotEmpty);
  };

private:
  size_t next(size_t pos) {
    return (pos + 1) % (N);
  };
  Ty mQueue[N];
  size_t mHead, mTail;
  pthread_mutex_t mLock;
  sem_t mSemQueueNotFull;
  sem_t mSemQueueNotEmpty;
};


template <class QueueTy, class FuncTy>
class Task {
public:
  Task(QueueTy & queue) : mQueue(queue) {}
  void activate() {
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

  void gracefulExit() {
    typedef typename QueueTy::element_t element_t;
    stop();
    mQueue.enqueue(element_t());
  };

private:
  static void * runHelper(Task * this_) {
    this_->run();
    return NULL;
  };

  void run() {
    while(true) {
      typename QueueTy::element_t e;
      mQueue.dequeue(e);
      if (mActive) {
	mFunctor(e);
      } else {
	break;
      }
    }
  };

  QueueTy & mQueue;
  bool mActive;
  FuncTy mFunctor;
};

struct ConditionalCounter {
public:
  ConditionalCounter() : mCount(0) {
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondVar, NULL);
  }

  ~ConditionalCounter() {
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondVar);
  }

  ConditionalCounter & operator++() {
    pthread_mutex_lock(&mLock);
    ++mCount;
    pthread_mutex_unlock(&mLock);
    return *this;
  }

  ConditionalCounter & operator--() {
    pthread_mutex_lock(&mLock);
    if (--mCount == 0) {
      pthread_cond_broadcast(&mCondVar);
    }
    pthread_mutex_unlock(&mLock);
    return *this;
  }

  void wait() {
    pthread_mutex_lock(&mLock);
    while (mCount)
      pthread_cond_wait(&mCondVar, &mLock);
    pthread_mutex_unlock(&mLock);
  }

private:
  int mCount;
  pthread_mutex_t mLock;
  pthread_cond_t mCondVar;
};

NAMESPACE_SC_END

#endif
