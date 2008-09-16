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
    gRequestQueuePoolCheck.dequeue(req);
    poolcheck(req.Pool, req.Node);
    --gSCSyncToken;
  }
};

class PoolCheckUIWrapper {
public:
  void operator()(PoolCheckRequest & req) const {
    gRequestQueuePoolCheckUI.dequeue(req);
    poolcheckui(req.Pool, req.Node);
    --gSCSyncToken;
  }
};

class BoundsCheckWrapper {
public:
  void operator()(BoundsCheckRequest & req) const {
    gRequestQueueBoundsCheck.dequeue(req);
    boundscheck(req.Pool, req.Source, req.Dest);
    --gSCSyncToken;
  }
};

class BoundsCheckUIWrapper {
public:
  void operator()(BoundsCheckRequest & req) const {
    gRequestQueueBoundsCheckUI.dequeue(req);
    boundscheckui(req.Pool, req.Source, req.Dest);
    --gSCSyncToken;
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
  llvm::safecode::gTaskPoolCheck.activate();
  llvm::safecode::gTaskPoolCheckUI.activate();
  llvm::safecode::gTaskBoundsCheck.activate();
  llvm::safecode::gTaskBoundsCheckUI.activate();
}

void __sc_spec_runtime_cleanup (void) {
  llvm::safecode::gTaskPoolCheck.gracefulExit();
  llvm::safecode::gTaskPoolCheckUI.gracefulExit();
  llvm::safecode::gTaskBoundsCheck.gracefulExit();
  llvm::safecode::gTaskBoundsCheckUI.gracefulExit();
} 
