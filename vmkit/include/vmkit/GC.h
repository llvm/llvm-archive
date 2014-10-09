//===----------- GC.h - Garbage Collection Interface -----------------------===//
//
//                     The Micro Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#ifndef VMKIT_GC_H
#define VMKIT_GC_H

#include <stdint.h>
#include "vmkit/System.h"

class gc;

#if OSGI_OBJECT_TIER_TAGGING

/*
	TierID bits = y xxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
	y: marked bit, used to avoid counting one object twice
	x...x: unsigned integer identifier
*/

#endif

class gcHeader {
public:
	word_t _header;

#if OSGI_OBJECT_TIER_TAGGING
	typedef uint16_t	TierIDType;		// 16-bits wide tag
	static const uint16_t TierIDBitMask_Mark = 0x8000, TierIDBitMask_ID = 0x7FFF;

	TierIDType _tier_id;
#endif

	inline gc* toReference() { return (gc*)((uintptr_t)this + hiddenHeaderSize()); }
	static inline size_t hiddenHeaderSize() { return sizeof(gcHeader); }
};

class gcRoot {
public:
#if OSGI_OBJECT_TIER_TAGGING
  inline gcHeader::TierIDType getTierID() const {
	  return (toHeader()->_tier_id & gcHeader::TierIDBitMask_ID);}

  inline void setTierID(gcHeader::TierIDType tierID)
  {
	  gcHeader::TierIDType& tid = toHeader()->_tier_id;
	  tid &= gcHeader::TierIDBitMask_Mark;
	  tid |= (tierID & gcHeader::TierIDBitMask_ID);
  }

  inline bool getTierObjectMarkState() const {
	  return ((toHeader()->_tier_id & gcHeader::TierIDBitMask_Mark) != 0);}

  inline void setTierObjectMarkState(bool markState)
  {
	  gcHeader::TierIDType& tid = toHeader()->_tier_id;
	  if (markState)
		  tid |= gcHeader::TierIDBitMask_Mark;
	  else
		  tid &= ~gcHeader::TierIDBitMask_Mark;
  }
#endif

  inline       word_t& header()       {return toHeader()->_header; }
  inline const word_t& header() const {return const_cast<gcRoot*>(this)->header(); }

  inline       gcHeader* toHeader()       {
      return (gcHeader*)((uintptr_t)this - gcHeader::hiddenHeaderSize()); }
  inline const gcHeader* toHeader() const {return const_cast<gcRoot*>(this)->toHeader(); }
};

namespace vmkit {
  // TODO(ngeoffray): Make these two constants easily configurable. For now they
  // work for all our supported GCs.
  static const uint32_t GCBits = 8;
  static const bool MovesObject = true;

  static const uint32_t HashBits = 8;
  static const uint64_t GCBitMask = ((1 << GCBits) - 1);
}

#endif
