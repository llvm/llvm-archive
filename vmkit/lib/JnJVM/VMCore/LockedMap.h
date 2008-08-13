//===------- LockedMap.h - A thread-safe map implementation ---------------===//
//
//                              JnJVM
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef JNJVM_LOCKED_MAP_H
#define JNJVM_LOCKED_MAP_H

#include <map>

#include <string.h>

#include "types.h"

#include "mvm/Object.h"
#include "mvm/PrintBuffer.h"
#include "mvm/Threads/Locks.h"

#include "JavaArray.h"
#include "JavaClass.h"
#include "JavaTypes.h"
#include "JavaString.h"
#include "Zip.h"

namespace jnjvm {

class JavaObject;
class Jnjvm;

struct ltutf8
{
  bool operator()(const UTF8* s1, const UTF8* s2) const
  {
    return s1->lessThan(s2);
  }
};

template<class Key, class Container, class Compare>
class LockedMap : public mvm::Object {
public:
  typedef typename std::map<const Key, Container, Compare>::iterator iterator;
  typedef Container (*funcCreate)(Key& V, Jnjvm *vm);

  mvm::Lock* lock;
  std::map<const Key, Container, Compare,
           gc_allocator<std::pair<const Key, Container> > > map;
  
  inline Container lookupOrCreate(Key& V, Jnjvm *vm, funcCreate func) {
    lock->lock();
    iterator End = map.end();
    iterator I = map.find(V);
    if (I == End) {
      Container res = func(V, vm);
      map.insert(std::make_pair(V, res));
      lock->unlock();
      return res;
    } else {
      lock->unlock();
      return ((Container)(I->second));
    }
  }
  
  inline void remove(Key V) {
    lock->lock();
    map.erase(V);
    lock->unlock();
  }

  inline Container lookup(Key V) {
    lock->lock();
    iterator End = map.end();
    iterator I = map.find(V);
    lock->unlock();
    return I != End ? ((Container)(I->second)) : 0; 
  }

  inline void hash(Key k, Container c) {
    lock->lock();
    map.insert(std::make_pair(k, c));
    lock->unlock();
  }


  virtual void print(mvm::PrintBuffer* buf) const {
    buf->write("Hashtable<>");
  }

};

class UTF8Map {
public:
  typedef std::multimap<const uint32, const UTF8*>::iterator iterator;
  
  mvm::Lock* lock;
  std::multimap<const uint32, const UTF8*> map;
  const UTF8* lookupOrCreateAsciiz(Jnjvm* vm, const char* asciiz); 
  const UTF8* lookupOrCreateReader(Jnjvm* vm, const uint16* buf, uint32 size);
  const UTF8* lookupAsciiz(const char* asciiz); 
  const UTF8* lookupReader(const uint16* buf, uint32 size);
  
  UTF8Map() {
    lock = mvm::Lock::allocNormal();
  }

  ~UTF8Map() {
    delete lock;
    for (iterator i = map.begin(), e = map.end(); i!= e; ++i) {
      delete(i->second);
    }
  }

  void copy(UTF8Map* newMap) {
    for (iterator i = map.begin(), e = map.end(); i!= e; ++i) {
      newMap->map.insert(*i);
    }
  }
  
  void replace(const UTF8* oldUTF8, const UTF8* newUTF8);
  void insert(const UTF8* val);
};

class ClassMap : 
    public LockedMap<const UTF8*, CommonClass*, ltutf8 > {
public:
  static VirtualTable* VT;
  
  ClassMap() {
    lock = mvm::Lock::allocNormal();
  }

  ~ClassMap() {
    delete lock;
  }
  
  virtual void TRACER;
};

class StringMap {
public:
  
  mvm::Lock* lock;
  typedef std::map<const UTF8*, JavaString*, ltutf8>::iterator iterator;
  std::map<const UTF8*, JavaString*, ltutf8 > map;
  
  typedef JavaString* (*funcCreate)(const UTF8*& V, Jnjvm *vm);
  
  StringMap() {
    lock = mvm::Lock::allocNormal();
  }
  
  ~StringMap() {
    delete lock;
    for (iterator i = map.begin(), e = map.end(); i!= e; ++i) {
      free(i->second);
    }
  }
  
  inline JavaString* lookupOrCreate(const UTF8*& V, Jnjvm *vm, funcCreate func) {
    lock->lock();
    iterator End = map.end();
    iterator I = map.find(V);
    if (I == End) {
      JavaString* res = func(V, vm);
      map.insert(std::make_pair(V, res));
      lock->unlock();
      return res;
    } else {
      lock->unlock();
      return I->second;
    }
  } 
};

class TypeMap {
public:
  mvm::Lock* lock;
  
  std::map<const UTF8*, Typedef*, ltutf8> map;
  typedef std::map<const UTF8*, Typedef*, ltutf8>::iterator iterator;
  
  inline Typedef* lookup(const UTF8* V) {
    lock->lock();
    iterator End = map.end();
    iterator I = map.find(V);
    lock->unlock();
    return I != End ? I->second : 0; 
  }

  inline void hash(const UTF8* k, Typedef* c) {
    lock->lock();
    map.insert(std::make_pair(k, c));
    lock->unlock();
  }
  
  TypeMap() {
    lock = mvm::Lock::allocRecursive();
  }
  
  ~TypeMap() {
    for (iterator i = map.begin(), e = map.end(); i!= e; ++i) {
      delete i->second;
    }
    delete lock;
  }
  
};

class SignMap {
public:
  mvm::Lock* lock;
  
  std::map<const UTF8*, Signdef*, ltutf8> map;
  typedef std::map<const UTF8*, Signdef*, ltutf8>::iterator iterator;
  
  inline Signdef* lookup(const UTF8* V) {
    lock->lock();
    iterator End = map.end();
    iterator I = map.find(V);
    lock->unlock();
    return I != End ? I->second : 0; 
  }

  inline void hash(const UTF8* k, Signdef* c) {
    lock->lock();
    map.insert(std::make_pair(k, c));
    lock->unlock();
  }
  
  SignMap() {
    lock = mvm::Lock::allocRecursive();
  }
  
  ~SignMap() {
    for (iterator i = map.begin(), e = map.end(); i!= e; ++i) {
      delete i->second;
    }
    delete lock;
  }
  
};

class StaticInstanceMap :
    public LockedMap<Class*, std::pair<JavaState, JavaObject*>*, std::less<Class*> > {
public:
  static VirtualTable* VT;
  
  StaticInstanceMap() {
    lock = mvm::Lock::allocNormal();
  }
  
  virtual void TRACER;

  ~StaticInstanceMap() {
    for (iterator i = map.begin(), e = map.end(); i!= e; ++i) {
      delete i->second;
    }
    delete lock;
  }
}; 

class DelegateeMap :
    public LockedMap<CommonClass*, JavaObject*, std::less<CommonClass*> > {
public:
  static VirtualTable* VT;
  
  DelegateeMap() {
    lock = mvm::Lock::allocNormal();
  }
  
  ~DelegateeMap() {
    delete lock;
  }
  
  virtual void TRACER;
}; 

} // end namespace jnjvm

#endif
