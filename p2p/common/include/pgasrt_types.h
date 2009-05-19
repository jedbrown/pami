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

#include <stdlib.h>
#include "pgasrt_config.h"

#ifndef __pgasrt_types_h__
#define __pgasrt_types_h__

/* ************************************************************************ */
/** @file pgasrt_types.h                                                    */
/* ************************************************************************ */

/* ************************************************************************ */
/** \brief type to describe UPC thread IDs, CAF image numbers               */
/* ************************************************************************ */

typedef size_t                    __pgasrt_thread_t;

/* ************************************************************************ */
/** \brief type alias for generic local addresses                           */
/* ************************************************************************ */

typedef unsigned char *           __pgasrt_local_addr_t;

/* ************************************************************************ */
/** \brief type alias for UPC and CAF array indices                         */
/* ************************************************************************ */

typedef size_t                    __pgasrt_array_index_t;

/* ************************************************************************ */
/** \brief enumerate all shared object types (UPC, CAF)                     */
/* ************************************************************************ */

typedef enum {
  UPC_SHR_UNINITIALIZED, /**< handle is not initialized */
  UPC_SHR_SCALAR,        /**< shared scalars */
  UPC_SHR_STRUCT,        /**< shared structs */
  UPC_SHR_ARRAY,         /**< shared arrays  */
  UPC_SHR_TARGET,        /**< shared ptrs with shared target */
  UPC_PVT_TARGET,        /**< shared ptrs with private target */
  UPC_SHR_BLOCKED_ARRAY, /**< shared array with multiple block factors */
  UPC_SHR_LOCK           /**< shared lock */
} __pgasrt_shared_var_types;

/* ************************************************************************ */
/** \brief UPC memory semantics: access flags                               */
/* ************************************************************************ */

typedef enum {
  UPC_ACCESS_STRICT=0,   /**< strict access */
  UPC_ACCESS_RELAXED=1   /**< relaxed access */
} __pgasrt_memory_semantics_t;

/* ************************************************************************ */
/** \brief SVD handles: index type for the SVD                              */
/* ************************************************************************ */
/* [GA] BlueGene Hack alert. The compiler thinks of SV handles as 32 bit    */
/* entities consisting of 16+16 bit values (partition ID + index). On       */
/* BlueGene 16 bits are not enough for partition IDs (>64K UPC threads).    */
/* Hence on BlueGene partition IDs are 20 bits each.                        */
/* Long term solution: make SV handles 64 bit entities.                     */
/* ************************************************************************ */
/* The compiler knows about the size and composition of this structure, but */
/* does not consult this header file. Be careful about modifying the struct.*/
/* ************************************************************************ */

typedef struct __pgasrt_svd_handle {
#if TRANSPORT == bgl
  unsigned int                    _partition:20;
  unsigned int                    _index:12;
#else
  unsigned short                  _partition;       /**< thread id or ALL */
  unsigned short                  _index;           /**< variable index */
#endif
} __pgasrt_svd_handle_t;


/* ************************************************************************ */
/** \brief remote address descriptors (used by memory registration)         */
/* ************************************************************************ */

#if TRANSPORT==lapi
typedef unsigned long long  __pgasrt_addrdesc_t;
#else
typedef void *              __pgasrt_addrdesc_t;
#endif

/* ************************************************************************ */
/** \brief shared variable control block (one per shared var in SVD)        */
/* ************************************************************************ */

typedef struct __pgasrt_shared_var_ctrl_block
{
  __pgasrt_shared_var_types  _type;
  union 
  {
    __pgasrt_local_addr_t                  _scalar;    /**< shared scalar  */
    __pgasrt_local_addr_t                  _struct;    /**< shared struct  */
    __pgasrt_local_addr_t                  _pvt_tgt;   /**< shared pvt-ptr */
    struct __xlupc_shared_array_t        * _array;     /**< shared array   */
    struct __xlupc_shared_mbarray_t      * _mbarray;   /**< tiled arr      */
    struct __xlupc_shared_addr_t         * _shr_tgt;   /**< shared fatptr  */
    struct __xlupc_lock_impl_t           * _shr_lock;  /**< UPC lock       */
  } u;
}
  __pgasrt_shared_var_ctrl_block;

/* ************************************************************************ */
/** \brief shared variable partition: the SVD is made up of a list of these */
/* ************************************************************************ */

/**
 * An SVD partition holds all the variables owned by one thread.
 * An SVD table holds all shared variables owned by all threads.
 */

typedef struct __pgasrt_svd_partition
{
  __pgasrt_shared_var_ctrl_block  * _directory;
  unsigned int                      _dirSize;        /** allocated size */
  unsigned int                      _nextFreeIndex;  /**< next free index */
}
  __pgasrt_svd_partition;

/* ************************************************************************ */
/** \brief type definition of shared variable directory                     */
/* ************************************************************************ */

typedef __pgasrt_svd_partition * __pgasrt_svd_table_t;

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

struct __pgasrt_AMHeader_t;

/* ************************************************************************ */
/** \brief Local completion handler function type                           */
/* ************************************************************************ */

typedef void (* __pgasrt_LCompHandler_t) (void *);

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


#define ALL_PARTITION PGASRT_THREADS




#endif
