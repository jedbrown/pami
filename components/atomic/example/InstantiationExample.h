/**
 * \file components/atomic/example/InstantiationExample.h
 * \brief ???
 */

#ifndef __components_atomic_example_InstantiationExample_h__
#define __components_atomic_example_InstantiationExample_h__

MutexUser<LockboxMutex> _foo; // This will compile.

MutexUser<PthreadMutex> _bar; // This will not compile.

#endif
