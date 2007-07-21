#include "llvm/Instructions.h"

using namespace llvm;

//
// Function: indexesStructsOnly()
//
// Description:
//  Determines whether the given GEP expression only indexes into structures.
//
// Return value:
//  true - This GEP only indexes into structures.
//  false - This GEP indexes into one or more arrays.
//
static bool
indexesStructsOnly (GetElementPtrInst * GEP) {
  unsigned int index = 0;
  const Type * ElementType = GEP->getPointerOperand()->getType();
  if (isa<ArrayType>(ElementType))
    return false;
  for (index = 1; index < (GEP->getNumOperands() - 1); ++index) {
    ElementType=GetElementPtrInst::getIndexedType (ElementType,
                                                   GEP->getOperand(++index));
    if (!ElementType)
      return false;
                                                                                
    if (isa<ArrayType>(ElementType))
      return false;
  }
  return true;
}

