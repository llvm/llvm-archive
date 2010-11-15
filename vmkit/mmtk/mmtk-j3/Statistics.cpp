//===----- Statistics.cpp - Implementation of the Statistics class  -------===//
//
//                              The VMKit project
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "JavaObject.h"
#include "JavaThread.h"

#include <sys/time.h>
#include <ctime>

using namespace j3;

extern "C" int64_t Java_org_j3_mmtk_Statistics_cycles__ (JavaObject* S) {
  return 0;
}

extern "C" int64_t Java_org_j3_mmtk_Statistics_nanoTime__ (JavaObject* S) {
  int64_t result;
  struct timeval tp; 

  int res = gettimeofday (&tp, NULL);
  assert(res != -1 && "failed gettimeofday.");

  result = (int64_t) tp.tv_sec;
  result *= (int64_t)1000000L;
  result += (int64_t)tp.tv_usec;
  result *= (int64_t)1000;

  return result;
}


extern "C" int32_t Java_org_j3_mmtk_Statistics_getCollectionCount__ (JavaObject* S) {
  return 0;
}

extern "C" void Java_org_j3_mmtk_Statistics_perfEventInit__Ljava_lang_String_2(JavaObject* S, JavaObject* Str) {
}

extern "C" void Java_org_j3_mmtk_Statistics_perfEventRead__I_3J(JavaObject* S, int id, int64_t* values) {
}