/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#ifndef __PGASRT_CONFIG_h__
#define __PGASRT_CONFIG_h__

/* ************************************************************************ */
/*                 Target configuration                                     */
/* ************************************************************************ */
/*   __PGASRT_DISTRIBUTED - pure distributed memory environment             */
/*   __PGASRT_HYBRID      - both shared and distributed memory.             */
/*                                                                          */
/* ************************************************************************ */
/* ONLY ONE OF THESE THREE VARIABLES CAN BE SET IN ANY CONFIGURATION.       */
/* ************************************************************************ */
/* We set these variables based on the value of the TRANSPORT variable      */
/* We get from Make.rules and configure.in. Check TRANSPORT values against  */
/* those defined by configure.in.                                           */
/* ************************************************************************ */

#define sockets  101
#define bgl      201
#define bgp      301
#define lapi     401
#define mx	 501
#define ompi     601

#define TRANSPORT lapi

#ifndef TRANSPORT
#error "TRANSPORT is not defined"
#endif

#undef __PGASRT_DISTRIBUTED
#undef __PGASRT_HYBRID

#if TRANSPORT == sockets
#define __PGASRT_HYBRID        1
#endif

#if TRANSPORT == bgl
#define __PGASRT_DISTRIBUTED   1
#endif

#if TRANSPORT == bgp
#define __PGASRT_HYBRID        1
#endif

#if TRANSPORT == lapi
#define  __PGASRT_HYBRID       1
#endif

#if TRANSPORT == mx
#define __PGASRT_HYBRID        1
#endif

#if TRANSPORT == ompi
#define __PGASRT_HYBRID        1
#endif

#if !defined(__PGASRT_DISTRIBUTED)
#if !defined(__PGASRT_HYBRID)
#error "TRANSPORT not defined correctly."
#endif
#endif

/* ************************************************************************ */
/*       Mutex declarations for distributed and hybrid                      */
/* ************************************************************************ */

#if !defined (__GNUC__)
#include <builtins.h>
#endif

#ifdef __PGASRT_DISTRIBUTED /* pure distributed - no pthreads library */

#define DECL_STATIC_MUTEX(x)    int x = 0
#define DECL_MUTEX(x)           int x
#define MUTEX_INIT(x)           { (*(x)) = 0; }
#define MUTEX_TRYLOCK(x)        ((*(x)) > 0 ? 1 : (((*x)=1),0))
#define MUTEX_LOCK(x) { while ((*(x)) > 0) __pgasrt_tsp_wait(NULL); (*(x))=1; }
#define MUTEX_UNLOCK(x)         { (*(x))=0; }
#define MUTEX_DESTROY(x)        { }
#define DECL_RW_MUTEX(x)
#define RDLOCK(x)
#define WRLOCK(x)
#define RWUNLOCK(x)

#else /* shared memory & hybrid */

#include <pthread.h>

#define DECL_STATIC_MUTEX(x) \
static pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
#define DECL_MUTEX(x)    pthread_mutex_t x
#define MUTEX_INIT(x)    pthread_mutex_init(x,NULL)
#define MUTEX_TRYLOCK(x) pthread_mutex_trylock(x)
#define MUTEX_LOCK(x)    pthread_mutex_lock(x)
#define MUTEX_UNLOCK(x)  pthread_mutex_unlock(x)
#define MUTEX_DESTROY(x) pthread_mutex_destroy(x)

#if (_XOPEN_SOURCE - 0) >= 500
#define DECL_RW_MUTEX(x) \
static pthread_rwlock_t x = PTHREAD_RWLOCK_INITIALIZER
#define RDLOCK(x)        pthread_rwlock_rdlock(x)
#define WRLOCK(x)        pthread_rwlock_wrlock(x)
#define RWUNLOCK(x)      pthread_rwlock_unlock(x)
#else
#define DECL_RW_MUTEX(x) \
static pthread_mutex_t x = PTHREAD_MUTEX_INITIALIZER
#define RDLOCK(x)        pthread_mutex_lock(x)
#define WRLOCK(x)        pthread_mutex_lock(x)
#define RWUNLOCK(x)      pthread_mutex_unlock(x)
#endif

#endif

#endif /* __PGASRT_CONFIG_h__ */
