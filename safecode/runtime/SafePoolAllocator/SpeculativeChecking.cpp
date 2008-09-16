//===- SpeculativeChecking.cpp - Implementation of Speculative Checking --*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file implements the asynchronous checking interfaces, enqueues checking requests
// and provides a synchronization token for each checking request.
//
//===----------------------------------------------------------------------===//

#include "Config.h"
#include "SafeCodeRuntime.h"
#include "PoolAllocator.h"


/// FIXME: the checking is not thread safe, so I use a lock here to 
/// serialize everything

static pthread_mutex_t mCheckLock;

NAMESPACE_SC_BEGIN

struct PoolCheckRequest {
  PoolTy * Pool;
  void * Node;
};

struct BoundsCheckRequest {
  PoolTy * Pool;
  void * Source;
  void * Dest;
};

#define CHECK_QUEUE_SIZE 4
typedef CircularQueue<PoolCheckRequest, CHECK_QUEUE_SIZE> RequestQueuePoolCheckTy;
typedef RequestQueuePoolCheckTy RequestQueuePoolCheckUITy;
typedef CircularQueue<BoundsCheckRequest, CHECK_QUEUE_SIZE> RequestQueueBoundsCheckTy;
typedef RequestQueueBoundsCheckTy RequestQueueBoundsCheckUITy;

RequestQueuePoolCheckTy gRequestQueuePoolCheck;
RequestQueuePoolCheckUITy gRequestQueuePoolCheckUI;
RequestQueueBoundsCheckTy gRequestQueueBoundsCheck;
RequestQueueBoundsCheckUITy gRequestQueueBoundsCheckUI;

SCSyncToken gSCSyncToken;

class PoolCheckWrapper {
public:
  void operator()(PoolCheckRequest & req) const {
    pthread_mutex_lock(&mCheckLock);
//    std::cerr << "poolcheck:" << req.Pool << " " << req.Node << std::endl;
    poolcheck(req.Pool, req.Node);
    --gSCSyncToken;
    pthread_mutex_unlock(&mCheckLock);
  }
};

class PoolCheckUIWrapper {
public:
  void operator()(PoolCheckRequest & req) const {
    pthread_mutex_lock(&mCheckLock);
//    std::cerr << "poolcheckui:" << req.Pool << " " << req.Node << std::endl;
    poolcheckui(req.Pool, req.Node);
    --gSCSyncToken;
    pthread_mutex_unlock(&mCheckLock);
  }
};

class BoundsCheckWrapper {
public:
  void operator()(BoundsCheckRequest & req) const {
    pthread_mutex_lock(&mCheckLock);
//    std::cerr << "boundscheck:" << req.Pool << " " << req.Dest << std::endl;
    boundscheck(req.Pool, req.Source, req.Dest);
    --gSCSyncToken;
    pthread_mutex_unlock(&mCheckLock);
  }
};

class BoundsCheckUIWrapper {
public:
  void operator()(BoundsCheckRequest & req) const {
    pthread_mutex_lock(&mCheckLock);
//    std::cerr << "boundscheckui:" << req.Pool << " " << req.Dest << std::endl;
    boundscheckui(req.Pool, req.Source, req.Dest);
    --gSCSyncToken;
    pthread_mutex_unlock(&mCheckLock);
  }
};

#define DEFINE_CHECK_TASK(NAME) Task<RequestQueue##NAME## Ty, NAME##Wrapper> gTask##NAME(gRequestQueue##NAME)

DEFINE_CHECK_TASK(PoolCheck);
DEFINE_CHECK_TASK(PoolCheckUI);
DEFINE_CHECK_TASK(BoundsCheck);
DEFINE_CHECK_TASK(BoundsCheckUI);

#undef DEFINE_CHECK_TASK

NAMESPACE_SC_END

SCSyncToken * __sc_poolcheck(PoolTy *Pool, void *Node) {
  ++llvm::safecode::gSCSyncToken;
  llvm::safecode::PoolCheckRequest req = {Pool, Node};
  llvm::safecode::gRequestQueuePoolCheck.enqueue(req);
  return &llvm::safecode::gSCSyncToken;
}

SCSyncToken * __sc_poolcheckui(PoolTy *Pool, void *Node) {
  ++llvm::safecode::gSCSyncToken;
  llvm::safecode::PoolCheckRequest req = {Pool, Node};
  llvm::safecode::gRequestQueuePoolCheckUI.enqueue(req);
  return &llvm::safecode::gSCSyncToken;
}

SCSyncToken * __sc_boundscheck   (PoolTy * Pool, void * Source, void * Dest) {
  ++llvm::safecode::gSCSyncToken;
  llvm::safecode::BoundsCheckRequest req = {Pool, Source, Dest};
  llvm::safecode::gRequestQueueBoundsCheck.enqueue(req);
  return &llvm::safecode::gSCSyncToken;
}

SCSyncToken * __sc_boundscheckui (PoolTy * Pool, void * Source, void * Dest) {
  ++llvm::safecode::gSCSyncToken;
  llvm::safecode::BoundsCheckRequest req = {Pool, Source, Dest};
  llvm::safecode::gRequestQueueBoundsCheckUI.enqueue(req);
  return &llvm::safecode::gSCSyncToken;
}

void __sc_wait_for_completion(SCSyncToken * token) {
  token->wait();
}

void __sc_spec_runtime_init (void) {
  pthread_mutex_init(&mCheckLock, NULL);
  llvm::safecode::gTaskPoolCheck.activate();
  llvm::safecode::gTaskPoolCheckUI.activate();
  llvm::safecode::gTaskBoundsCheck.activate();
  llvm::safecode::gTaskBoundsCheckUI.activate();
}

void __sc_spec_runtime_cleanup (void) {
// std::cerr << "Cleanup" << std::endl;
  llvm::safecode::gTaskPoolCheck.gracefulExit();
  llvm::safecode::gTaskPoolCheckUI.gracefulExit();
  llvm::safecode::gTaskBoundsCheck.gracefulExit();
  llvm::safecode::gTaskBoundsCheckUI.gracefulExit();
  pthread_mutex_destroy(&mCheckLock);
} 
