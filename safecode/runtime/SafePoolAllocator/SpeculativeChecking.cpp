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

typedef enum {
  CHECK_POOL_CHECK,
  CHECK_POOL_CHECK_UI,
  CHECK_BOUNDS_CHECK,
  CHECK_BOUNDS_CHECK_UI,
  CHECK_REQUEST_COUNT
} RequestTy;

struct CheckRequest {
  RequestTy type;
  union {
    PoolCheckRequest poolcheck;
    BoundsCheckRequest boundscheck;
  };
};

typedef CircularQueue<CheckRequest, 4> CheckQueueTy;
CheckQueueTy gCheckQueue;

SCSyncToken gSCSyncToken;

class CheckWrapper {
public:
  void operator()(CheckRequest & req) const {
    switch (req.type) {
    case CHECK_POOL_CHECK:
      poolcheck(req.poolcheck.Pool, req.poolcheck.Node);
      break;

    case CHECK_POOL_CHECK_UI:
      poolcheckui(req.poolcheck.Pool, req.poolcheck.Node);
      break;

    case CHECK_BOUNDS_CHECK:
      boundscheck(req.boundscheck.Pool, req.boundscheck.Source, req.boundscheck.Dest);
      break;

    case CHECK_BOUNDS_CHECK_UI:
      boundscheckui(req.boundscheck.Pool, req.boundscheck.Source, req.boundscheck.Dest);
      break;
    default:
      break;
    } 
    --gSCSyncToken;
  }
};

Task<CheckQueueTy, CheckWrapper> gCheckTask(gCheckQueue);

NAMESPACE_SC_END

void __sc_poolcheck(PoolTy *Pool, void *Node) {
  ++llvm::safecode::gSCSyncToken;
  llvm::safecode::CheckRequest req;
  req.type = llvm::safecode::CHECK_POOL_CHECK;
  req.poolcheck.Pool = Pool;
  req.poolcheck.Node = Node;
  llvm::safecode::gCheckQueue.enqueue(req);
}

void __sc_poolcheckui(PoolTy *Pool, void *Node) {
  ++llvm::safecode::gSCSyncToken;
  llvm::safecode::CheckRequest req;
  req.type = llvm::safecode::CHECK_POOL_CHECK_UI;
  req.poolcheck.Pool = Pool;
  req.poolcheck.Node = Node;
  llvm::safecode::gCheckQueue.enqueue(req);
}

void __sc_boundscheck   (PoolTy * Pool, void * Source, void * Dest) {
  ++llvm::safecode::gSCSyncToken;
  llvm::safecode::CheckRequest req;
  req.type = llvm::safecode::CHECK_BOUNDS_CHECK;
  req.boundscheck.Pool = Pool;
  req.boundscheck.Source = Source;
  req.boundscheck.Dest = Dest;
  llvm::safecode::gCheckQueue.enqueue(req);
}

void __sc_boundscheckui (PoolTy * Pool, void * Source, void * Dest) {
  ++llvm::safecode::gSCSyncToken;
  llvm::safecode::CheckRequest req;
  req.type = llvm::safecode::CHECK_BOUNDS_CHECK_UI;
  req.boundscheck.Pool = Pool;
  req.boundscheck.Source = Source;
  req.boundscheck.Dest = Dest;
  llvm::safecode::gCheckQueue.enqueue(req);
}

void __sc_wait_for_completion(SCSyncToken * token) {
  token->wait();
}

void __sc_spec_runtime_init (void) {
  llvm::safecode::gCheckTask.activate();
 }

void __sc_spec_runtime_cleanup (void) {
  llvm::safecode::gCheckTask.gracefulExit();
} 
