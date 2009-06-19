/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/ccmi_internal.h
 * \brief ???
 */

#ifndef   __ccmi_internal_h__
#define   __ccmi_internal_h__

#include <stdlib.h>

///
/// \brief CCMI Adaptor dependent definitions
///
#if TARGET==genericmpi
#include "collectives/interface/genericmpi/adaptor_pre.h"
#elif TARGET==lapiunix
#include "collectives/interface/lapiunix/adaptor_pre.h"
#else
#error "Incorrect Target Specified"
#endif


// Pgas types \todo:  rename
typedef size_t                    __pgasrt_thread_t;
typedef unsigned char *           __pgasrt_local_addr_t;
typedef size_t                    __pgasrt_array_index_t;
/* ************************************************************************ */
/** \brief Operation types in remote updates and in allreduce collectives   */
/* ************************************************************************ */

typedef enum 
  {
    PGASRT_OP_ADD = 0,       /**< addition */
    PGASRT_OP_MUL,           /**< multiplication */
    PGASRT_OP_DIV,           /**< division */
    PGASRT_OP_AND,           /**< logical and */
    PGASRT_OP_OR,            /**< logical or */
    PGASRT_OP_XOR,           /**< logical xor */
    PGASRT_OP_MAX,           /**< maximum */
    PGASRT_OP_MIN            /**< minimum */
  }
__pgasrt_ops_t;

/* ************************************************************************ */
/** \brief Data types used in remote updates and in allreduce collectives   */
/* ************************************************************************ */

typedef enum
  {
    PGASRT_DT_byte,    /**<  8 bits unsigned */
    PGASRT_DT_chr,     /**<  8 bits signed   */
    PGASRT_DT_srt,     /**< 16 bits signed   */
    PGASRT_DT_hwrd,    /**< 16 bits unsigned */
    PGASRT_DT_int,     /**< 32 bits signed   */
    PGASRT_DT_word,    /**< 32 bits unsigned */
    PGASRT_DT_llg,     /**< 64 bits signed   */
    PGASRT_DT_dwrd,    /**< 64 bits unsigned */
    PGASRT_DT_dbl,     /**< 64 bits, IEEE fp */
    PGASRT_DT_flt,     /**< 32 bits, IEEE fp */
    PGASRT_DT_dblint   /**< 96 bits, dbl+int */
  }
__pgasrt_dtypes_t;

/* ************************************************************************ */
/** \brief Runtime error codes (needs update)                               */
/* ************************************************************************ */

typedef enum
  {
    PGASRT_OK=0,                         /**< no error */
    PGASRT_ERR_NO_PRIVATE_MEM,           /**< local memory allocation error */
    PGASRT_ERR_NO_SHARED_MEM,            /**< shared memory allocation error */
    PGASRT_ERR_NO_MEM,                   /**< generic allocation error */
    PGASRT_ERR_SHARED_PTR,               /**< malformed pointer-to-shared */
    PGASRT_ERR_COMMUNICATION,            /**< generic communication error */
    PGASRT_ERR_NUM_THREADS,              /**< wrong number of threads */
    PGASRT_ERR_BARRIER,                  /**< barrier counter error */
    PGASRT_ERR_INDEX,                    /**< index arithmetic problem */
    PGASRT_ERR_CAST                      /**< UPC casting error */
  }
__pgasrt_error_t;

/* ************************************************************************ */
/** \brief AM header handler data type                                      */
/* ************************************************************************ */

typedef __pgasrt_local_addr_t
(* __pgasrt_AMHeaderHandler_t) (const struct __pgasrt_AMHeader_t *,
				void (** completionHandler)(void *, void *),
				void ** arg);

/* ************************************************************************ */
/** \brief AM header data type                                              */
/* ************************************************************************ */

typedef struct __pgasrt_AMHeader_t
{
  __pgasrt_AMHeaderHandler_t   handler;    /**< header handler */
  unsigned                     headerlen;  /**< total bytes in header */
  void                       * data[1];    /**< dummy */
}
__pgasrt_AMHeader_t __attribute__((__aligned__(16)));

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

#define PGASRT_TSP_AMSEND_COLLEXCHANGE    99
#define PGASRT_TSP_AMSEND_COLLSCATTER     97

///
///\brief CCMI definitions
///

#ifndef __ccmi_consistency_defined__
  #define __ccmi_consistency_defined__
typedef enum
{
  CCMI_UNDEFINED_CONSISTENCY = -1,
  CCMI_RELAXED_CONSISTENCY,
  CCMI_MATCH_CONSISTENCY,
  CCMI_WEAK_CONSISTENCY,
  CCMI_CONSISTENCY_COUNT
}
CCMI_Consistency;
#endif

#ifndef __ccmi_op_defined__
  #define __ccmi_op_defined__
typedef enum
{
  CCMI_UNDEFINED_OP = 0,
  CCMI_NOOP,
  CCMI_MAX,
  CCMI_MIN,
  CCMI_SUM,
  CCMI_PROD,
  CCMI_LAND,
  CCMI_LOR,
  CCMI_LXOR,
  CCMI_BAND,
  CCMI_BOR,
  CCMI_BXOR,
  CCMI_MAXLOC,
  CCMI_MINLOC,
  CCMI_USERDEFINED_OP,
  CCMI_OP_COUNT
}
CCMI_Op;
#endif

#ifndef __ccmi_dt_defined__
  #define __ccmi_dt_defined__
typedef enum
{
  /* Standard/Primative DT's */
  CCMI_UNDEFINED_DT = 0,
  CCMI_SIGNED_CHAR,
  CCMI_UNSIGNED_CHAR,
  CCMI_SIGNED_SHORT,
  CCMI_UNSIGNED_SHORT,
  CCMI_SIGNED_INT,
  CCMI_UNSIGNED_INT,
  CCMI_SIGNED_LONG_LONG,
  CCMI_UNSIGNED_LONG_LONG,
  CCMI_FLOAT,
  CCMI_DOUBLE,
  CCMI_LONG_DOUBLE,
  CCMI_LOGICAL,
  CCMI_SINGLE_COMPLEX,
  CCMI_DOUBLE_COMPLEX,
  /* Max/Minloc DT's */
  CCMI_LOC_2INT,
  CCMI_LOC_SHORT_INT,
  CCMI_LOC_FLOAT_INT,
  CCMI_LOC_DOUBLE_INT,
  CCMI_LOC_2FLOAT,
  CCMI_LOC_2DOUBLE,
  CCMI_USERDEFINED_DT
}
CCMI_Dt;
#endif

//--------------------------------------------------
//----- Communication CCMI_Subtasks ---------------------
//----- Each communication operation is split ------
//----- into phases of one or more sub tasks -------
//--------------------------------------------------

#ifndef __ccmi_subtask_defined__
  #define __ccmi_subtask_defined__
  #define  LINE_BCAST_MASK    (CCMI_LINE_BCAST_XP|CCMI_LINE_BCAST_XM|\
                                 CCMI_LINE_BCAST_YP|CCMI_LINE_BCAST_YM|\
                                 CCMI_LINE_BCAST_ZP|CCMI_LINE_BCAST_ZM)

typedef enum
{
  CCMI_PT_TO_PT_SUBTASK           =  0,      //Send a pt-to-point message
  CCMI_LINE_BCAST_XP              =  0x20,   //Bcast along x+
  CCMI_LINE_BCAST_XM              =  0x10,   //Bcast along x-
  CCMI_LINE_BCAST_YP              =  0x08,   //Bcast along y+
  CCMI_LINE_BCAST_YM              =  0x04,   //Bcast along y-
  CCMI_LINE_BCAST_ZP              =  0x02,   //Bcast along z+
  CCMI_LINE_BCAST_ZM              =  0x01,   //Bcast along z-
  CCMI_COMBINE_SUBTASK            =  0x0100,   //Combine the incoming message
  //with the local state
  CCMI_GI_BARRIER                 =  0x0200,
  CCMI_LOCKBOX_BARRIER            =  0x0300,
  CCMI_TREE_BARRIER               =  0x0400,
  CCMI_TREE_BCAST                 =  0x0500,
  CCMI_TREE_ALLREDUCE             =  0x0600,
  CCMI_REDUCE_RECV_STORE          =  0x0700,
  CCMI_REDUCE_RECV_NOSTORE        =  0x0800,
  CCMI_BCAST_RECV_STORE           =  0x0900,
  CCMI_BCAST_RECV_NOSTORE         =  0x0a00,
  CCMI_LOCALCOPY                  =  0x0b00,

  CCMI_UNDEFINED_SUBTASK          =  (~LINE_BCAST_MASK),
} CCMI_Subtask;
#endif


// Optimized reduction function (comes from the math lib)
typedef void (*CCMI_ReduceFunc)(void *dst, void **srcs, int nsrcs, int count);

//Descriptive quad of data that identifies each collective
typedef struct _cheader_data
{
  unsigned        _root;         //root of the collective
  unsigned        _comm;         //communicator
  unsigned        _count;        //count for this collective operation
  unsigned        _phase:12;     //phase of the collective
  unsigned        _iteration:4;  //iteration of the allreduce, otherwise unused
  unsigned        _op:8;         //op code of the allreduce, otherwise unused
  unsigned        _dt:8;         //datatype of the allreduce, otherwise unused
} CollHeaderData  __attribute__((__aligned__(16)));


#ifndef CCMI_Alloc
  #define CCMI_Alloc(x)  malloc(x)
#endif

#ifndef CCMI_Free
  #define CCMI_Free(x)   free(x)
#endif

#define CCMI_UNDEFINED_PHASE ((unsigned)-1)

#define  CCMI_UNDEFINED_RANK ((unsigned)-1)  //Return this when the destination rank is irrelevant like a
//GI or Tree collective operation

#ifndef __ccmi_quad_defined__
  #warning Adaptor did not declare CCMIQuad!
typedef struct CCMIQuad
{
  unsigned w0; /**< Word[0] */
  unsigned w1; /**< Word[1] */
  unsigned w2; /**< Word[2] */
  unsigned w3; /**< Word[3] */
}
CCMIQuad __attribute__ ((__aligned__ (16)));
#define __ccmi_quad_defined__

#endif

typedef CCMIQuad CCMI_Geometry_t [32];
typedef CCMIQuad CCMI_CollectiveProtocol_t [32*2];
typedef CCMIQuad CCMI_CollectiveRequest_t  [32*8*4];
typedef CCMIQuad CCMI_Request_t            [32];
typedef CCMIQuad CCMI_Executor_t           [128];  //32 for schedule and 32 for executor

#ifndef __ccmi_error_defined__

typedef int CCMI_Error_t;
typedef int CCMI_Result;
const CCMI_Result CCMI_SUCCESS=0;

  #define __ccmi_error_defined__
#endif


#ifndef __ccmi_callback_defined__
typedef struct CCMI_Callback_t
{
  void (*function) (void *, CCMI_Error_t *);
  void *clientdata;
}
CCMI_Callback_t;
  #define __ccmi_callback_defined__
#endif


#ifndef __ccmi_recvasynccallback_defined__
typedef void * (*CCMI_RecvAsyncBroadcast) (unsigned           root,
                                           unsigned           comm,
                                           const unsigned     sndlen,
                                           unsigned         * rcvlen,
                                           char            ** rcvbuf,
                                           CCMI_Callback_t  * const cb_info);
  #define __ccmi_recvasynccallback_defined__
#endif


/**
 *  \brief A callback to map the geometry id to the geometry
 *  structure. The CCMI runtime has to be able to freely translate
 *  between geometry id's and geometry structures.
 */

typedef CCMI_Geometry_t * (*CCMI_mapIdToGeometry) (int comm);


#endif
