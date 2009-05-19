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

#include "pgasrt.h"

#ifndef __xlupc_types_h__
#define __xlupc_types_h__

/* ************************************************************************ */
/** @file xlupc_types.h                                                     */
/* ************************************************************************ */

/* ************************************************************************ */
/*                         __xlupc_shared_addr_t                            */
/* ************************************************************************ */
/* The compiler knows about the size and composition of this structure, but */
/* does not consult this header file. Be careful about modifying the struct.*/
/* ************************************************************************ */
/** 
 * \brief This type describes a fat pointer, the mechanism to address
 * locations in shared space. These can be \em anomymous \em
 * variables, \em private \em pointers \em with \em shared \em
 * targets, or \em named \em variables.  Named variables are accessed
 * through a handle that points to a __xlupc_shared_addr_t.
 */
/* ************************************************************************ */

typedef struct __xlupc_shared_addr_t
{
  __pgasrt_svd_handle_t       _svd_handle;    /**< shared variable handle   */
  __pgasrt_thread_t           _tgt_thread_id; /**< shared pointer affinity  */
  unsigned int                _tgt_course;    /**< shared pointer course    */
  signed int                  _tgt_phase;     /**< shared pointer phase     */
  signed int                  _tgt_blk;       /**< shared pointer block     */
  unsigned short              _elt_sz;        /**< size of elt pointed to   */
  unsigned short              _unused;        /**< unused                   */
  signed int                  _offset;        /**< offset for ptr to struct */
  unsigned int                _unused2;       /**< unused                   */
} 
__xlupc_shared_addr_t;                        /* total 32 bytes */

/* ************************************************************************ */
/*                        __xlupc_shared_array_t                            */
/* ************************************************************************ */
/**
 * \brief This type describes an XLUPC shared array as used by the SVD.
 */
/* ************************************************************************ */

typedef struct __xlupc_shared_array_t
{
  __pgasrt_local_addr_t     _local_addr;    /**< local memory address */
  __pgasrt_array_index_t    _num_elts;      /**< total number of elts */
  size_t                    _local_sz;      /**< bytes in local part  */
  unsigned int              _blk;           /**< block size           */
  __pgasrt_shared_var_types _type;          /**< the type of elements */
  unsigned short            _elt_sz;        /**< element size (bytes) */
  int                       _addr_is_global;/**< nodes use same local addr */
  void *                    _mmap_ret;      /**< return value of mmap, if used, else MAP_FAILED */
}
__xlupc_shared_array_t;

#define UPC_MB_MAXBLKSIZE 65535
#define UPC_MB_MAXDIM     10

/* ************************************************************************ */
/*                __xlupc_shared_blocked_array_t                            */
/* ************************************************************************ */
/**
 * \brief This type describes a shared tiled array
 * representation. Assumptions (for now):
 - the number of dimesions == the number of block dimensions             
 - the blocking factors can not be indefinite or 0                       
 - these arrays are only for native types                                
*/
/* ************************************************************************ */

typedef struct __xlupc_shared_mbarray_t
{
  __pgasrt_local_addr_t     _local_addr;           /**< local address m     */
  size_t                    _local_sz;             /**< local size (bytes)  */
  unsigned int              _nDims;                /**< # dims              */
  __pgasrt_array_index_t    _dim  [UPC_MB_MAXDIM]; /**< dim sizes           */
  unsigned int              _blk  [UPC_MB_MAXDIM]; /**< block sizes         */
  unsigned int              _tblk [UPC_MB_MAXDIM]; /**< thread blocks       */
  __pgasrt_array_index_t    _nblk [UPC_MB_MAXDIM]; /**< # blocks in each dim*/
  unsigned short            _elt_sz;               /**< element size (bytes)*/
}
__xlupc_shared_mbarray_t;

/* ************************************************************************ */
/** \brief shared lock                                                      */
/* ************************************************************************ */

typedef struct __xlupc_lock_queue_element_t
{
  __pgasrt_thread_t                     requestor;
  int                                 * flagaddr;
  struct __xlupc_lock_queue_element_t * prev;
  struct __xlupc_lock_queue_element_t * next;
}
  __xlupc_lock_queue_element_t;

typedef struct __xlupc_lock_queue_t
{
  __xlupc_lock_queue_element_t        * front;
  __xlupc_lock_queue_element_t        * back;
  // DECL_MUTEX(mutex);
}
  __xlupc_lock_queue_t;

typedef struct __xlupc_lock_impl_t
{
  DECL_MUTEX(_mutex);
  __pgasrt_thread_t                    _owner;
  __xlupc_lock_queue_t                 _queue;
}
__xlupc_lock_impl_t;

/* ************************************************************************ */
/** \brief type used for increments in UPC pointers                         */
/* ************************************************************************ */

typedef ptrdiff_t                 __xlupc_ptr_incr_t;

/* ************************************************************************ */
/** \brief UPC collective synchronization flags                             */
/* These values are reproduced in the compiler's upc_collective.h file      */
/* ************************************************************************ */

#define UPC_IN_ALLSYNC  (1<<0)
#define UPC_IN_MYSYNC   (1<<1)
#define UPC_IN_NOSYNC   (1<<2)
#define UPC_OUT_ALLSYNC (1<<3)
#define UPC_OUT_MYSYNC  (1<<4)
#define UPC_OUT_NOSYNC  (1<<5)



#endif
