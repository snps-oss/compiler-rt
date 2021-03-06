//===-- dd_rtl.h ----------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef DD_RTL_H
#define DD_RTL_H

#include "sanitizer_common/sanitizer_internal_defs.h"
#include "sanitizer_common/sanitizer_deadlock_detector_interface.h"
#include "sanitizer_common/sanitizer_allocator_internal.h"
#include "sanitizer_common/sanitizer_mutex.h"

namespace __dsan {

struct Mutex {
  Mutex *link;
  uptr addr;
  DDMutex dd;
};

struct Thread {
  DDPhysicalThread *dd_pt;
  DDLogicalThread *dd_lt;

  bool in_symbolizer;
};

struct Context {
  DDetector *dd;

  SpinMutex mutex_mtx;
  Mutex *mutex_list;
  u64 mutex_seq;
};

void InitializeInterceptors();

void ThreadInit(Thread *thr);
void ThreadDestroy(Thread *thr);

void MutexLock(Thread *thr, uptr m, bool writelock, bool trylock);
void MutexUnlock(Thread *thr, uptr m, bool writelock);
void MutexDestroy(Thread *thr, uptr m);

}  // namespace __dsan
#endif  // DD_RTL_H
