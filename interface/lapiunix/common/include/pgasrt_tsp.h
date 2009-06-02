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


#ifndef __PGASRT_TSP_H_
#define __PGASRT_TSP_H_

#undef EXTERN
#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

#include "pgasrt_config.h"
#include "pgasrt_types.h"

#include <stdlib.h>         /* for size_t */
#include <limits.h>
#include <stddef.h>

/* *********************************************************************** */
/** @file pgasrt_tsp.h                                                     */
/* *********************************************************************** */

/* ************************************************************************ */
/** \brief short hand for \ref __pgasrt_tsp_myID */
/* ************************************************************************ */

#define PGASRT_MYNODE    __pgasrt_tsp_myID()

/* ************************************************************************ */
/** \brief short hand for \ref __pgasrt_tsp_numnodes */
/* ************************************************************************ */

#define PGASRT_NODES     __pgasrt_tsp_numnodes()

/* **************************************************************************/
/** \brief[INIT] Initialize communication system . Not reentrant.
 * \param lthreads: number of local threads (== threads on local node)
 * \param mynode: [out] my process number
 * \param numnodes: [out]how many processes are executing
 * \returns nothing
 */
/* **************************************************************************/

EXTERN void *
__pgasrt_tsp_setup                 (unsigned               lthreads,
				    int                   * pargc,
				    char                *** pargv);

/* **************************************************************************/
/** \brief Return my node ID */
/* **************************************************************************/

EXTERN int
__pgasrt_tsp_myID                 (void);

/* **************************************************************************/
/** \brief Return number of nodes in this job */
/* **************************************************************************/

EXTERN int
__pgasrt_tsp_numnodes              (void);

/* **************************************************************************/
/** \brief Set the number of nodes the transport expects to work with       */
/* **************************************************************************/

EXTERN void
__pgasrt_tsp_set_expected_nodes(int expected_nodes);

EXTERN int __pgasrt_tsp_expected_nodes;

/* **************************************************************************/
/** \brief [INIT] Terminate communication system. Not reentrant.
 * \returns nothing useful that I know of. We can roll over and die now.
 */
/* **************************************************************************/

EXTERN void *
__pgasrt_tsp_finish                (void);

/* **************************************************************************/
/* **************************************************************************/

EXTERN void
__pgasrt_tsp_abort                 (int exitcode);

/* **************************************************************************/
/** 
 * \brief [INIT] registers (pins) memory for later access by DMA.
 * \param ptr: [in] pointer to memory wanted to register
 * \param len: [in] length of the memory region we want to pin
 * \returns nothing
 */
/* **************************************************************************/

EXTERN __pgasrt_addrdesc_t
__pgasrt_tsp_regmem                (void                 * ptr, 
				    unsigned               len);

/* **************************************************************************/
/** 
 * \brief [DMA] unpins memory
 * This call is needed for non-shared memory regions, we don't want to
 * keep them on the cache to avoid memory inconsistency
 * \param ptr: [in] pointer to memory wanted to deregister
 * \param desc: descriptor we want to unmap
 * \returns nothing
 */
/* **************************************************************************/

EXTERN void 
__pgasrt_tsp_deregmem              (void                 * ptr,
				    __pgasrt_addrdesc_t    desc,
				    unsigned               len);

/* **************************************************************************/
/**
 * \brief [ACCESS] wait for [local] completion of a distributed operation.
 *
 * \param request: pointer to the request we are waiting for
 */
/* **************************************************************************/

EXTERN void *
__pgasrt_tsp_wait                  (void                * request);

/* **************************************************************************/
/**
 * \brief [ACCESS] blocking fence: return when all outstanding ops complete
 *
 * \param lclthread          rank of local thread calling the fence. 
 *                           0 <= localThread < threads (as specified in setup)
 */
/* **************************************************************************/

EXTERN void
__pgasrt_tsp_fence                 (unsigned             lclthread);

/* **************************************************************************/
/**
 * \brief [ACCESS] post a GET Request.
 * Gets the shared data of size len in src to the local addressed dst.
 * This routine is non-blocking. To guarantee the completion
 * of the operations a call to \ref __pgasrt_tsp_wait should be issued.
 * When operation completes we can use the information received in dst buffer.
 *
 * \param localThread  local rank (not UPC rank!) of thread issuing request
 * \param srcNode      ID of node that will provide the data
 * \param dst          local address to which data is to be copied
 * \param src          shared variable handle for source
 * \param offset       offset on the source address (bytes)
 * \param len          length of data requested (bytes)
 * \returns            a handle that can be waited on for completion.
 */
/* **************************************************************************/

EXTERN void *
__pgasrt_tsp_get                   (unsigned                     localThread,
				    unsigned                     srcNode,
				    __pgasrt_local_addr_t        dst,
				    __pgasrt_svd_handle_t        src,
				    size_t                       offset,
				    size_t                       len,
				    __pgasrt_LCompHandler_t      comp_h,
				    void                       * info);

/* **************************************************************************/
/**
 * \brief [ACCESS] post a Direct GET Request.
 * This is like GET, but does no address translation at the remote end.
 *
 * \param localThread  local rank (not UPC rank!) of thread issuing request
 * \param srcNode      ID of node that will provide the data
 * \param dst          local address to which data is to be copied
 * \param remote_src   address descriptor at remote end
 * \param len          length of data requested (bytes)
 * \returns            a handle that can be waited on for completion.
 */
/* **************************************************************************/

EXTERN void *
__pgasrt_tsp_dget                  (unsigned                     localThread,
				    unsigned                     srcNode,
				    __pgasrt_local_addr_t        dst,
				    __pgasrt_addrdesc_t          remote_src,
				    size_t                       offset,
				    size_t                       len,
				    __pgasrt_LCompHandler_t      comp_h,
				    void                       * info);

/* **************************************************************************/
/**
 * \brief [ACCESS] post a STRIDED GET request
 * \param localThread  local rank (not UPC rank!) of thread issuing request
 * \param srcNode      ID of node that will provide the data
 * \param dst          the (contiguous, allocated) destination buffer
 * \param src          the (remote) shared variable handle 
 * \param offset       (remote) offset
 * \param elemSize     number of bytes in each (remote) chunk of data 
 * \param stride       number of bytes between successive chunks of data
 * \param numElems     number of chunks of data to bring.
 * \returns            a handle that can be waited on for completion
 */
/* **************************************************************************/

EXTERN void * 
__pgasrt_tsp_getv                  (unsigned                     localThread,
				    unsigned                     srcNode,
				    __pgasrt_local_addr_t        dst,
				    __pgasrt_svd_handle_t        src,
				    size_t                       offset,
				    size_t                       elemSize,
				    size_t                       stride,
				    size_t                       numElems,
				    __pgasrt_LCompHandler_t      comp_h,
                                    void                       * info);

/* **************************************************************************/
/**
 * \brief [ACCESS] post a PUT request. Local completion semantics
 * (i.e. complete when outbound data has cleared local buffers).
 * Invoke fence to wait for remote completion.
 * 
 * \param localThread  local rank (not UPC rank!) of thread issuing request
 * \param destNode     destination node
 * \param dst          shared variable handle for destination
 * \param offset       offset on the destination address
 * \param src          local address from which data is to be copied
 * \param len          length of data
 * \param              arg gets passed to signalling callback if nonzero.
 * \returns            a handle to be waited on for local completion
 */
/* **************************************************************************/

EXTERN void * 
__pgasrt_tsp_put                   (unsigned                     localThread,
				    unsigned                     destNode,
				    __pgasrt_svd_handle_t        dst,
				    size_t                       offset,
				    __pgasrt_local_addr_t        src,
				    size_t                       len,
				    void *                       arg,
				    __pgasrt_LCompHandler_t      comp_h,
                                    void                       * info);

/* **************************************************************************/
/**
 * \brief [ACCESS] post a Direct PUT Request.
 * This is like PUT, but does no address translation at the remote end.
 *
 * \param localThread  local rank (not UPC rank!) of thread issuing request
 * \param destNode     ID of node that will provide the data
 * \param dst          address descriptor to which data is to be copied
 * \param offset       offset to address descriptor
 * \param src          local address at target node (srcNode) for source
 * \param len          length of data requested (bytes)
 * \returns            a handle that can be waited on for completion.
 */
/* **************************************************************************/

EXTERN void * 
__pgasrt_tsp_dput                  (unsigned                     localThread,
				    unsigned                     destNode,
				    __pgasrt_addrdesc_t          dst,
				    size_t                       offset,
				    __pgasrt_local_addr_t        src,
				    size_t                       len,
				    __pgasrt_LCompHandler_t      comp_h,
                                    void                       * info);

/* **************************************************************************/
/**
 * \brief [ACCESS] post a strided PUT request. Local completion semantics
 * (i.e. complete when outbound data has cleared local buffers).
 * Invoke fence to wait for remote completion.

 * \param localThread  local rank (not UPC rank!) of thread issuing request
 * \param destNode     destination node
 * \param dst          shared variable handle for destination
 * \param offset       offset on the destination address
 * \param src          local address from which data is to be copied
 * \param elemSize     size of elements to be copied
 * \param stride       remote stride
 * \param numElems     how many elements are to be copied
 * \returns            a handle to be waited on for local completion
 */
/* **************************************************************************/

EXTERN void * 
__pgasrt_tsp_putv                  (unsigned                     localThread,
				    unsigned                     destNode,
				    __pgasrt_svd_handle_t        dst,
				    size_t                       offset,
				    __pgasrt_local_addr_t        src,
				    size_t                       elemSize,
				    size_t                       stride,
				    size_t                       numElems,
				    __pgasrt_LCompHandler_t      comp_h,
                                    void                       * info);

/* **************************************************************************/
/**
 * \brief [ACCESS] Remote Accumulate. Modifies the shared location
 * defined by dst according to the local data of size len in src and
 * the op indicated in op.  
 * Local completion semantics (complete when outbound data has cleared 
 * local buffers).
 *
 * \param localThread  local rank (not UPC rank!) of thread issuing request
 * \param destNode     destination node
 * \param dst          shared variable handle for destination 
 * \param offset       offset on the destination address 
 * \param src          local address from which data is to be copied
 * \param optype       data type at destination
 * \param op           operation to apply at the destination
 * \returns            a handle to be waited on for completion
 */
/* **************************************************************************/

EXTERN void * 
__pgasrt_tsp_update                (unsigned                     localThread,
				    unsigned                     destNode,
				    __pgasrt_svd_handle_t        dst,
				    size_t                       offset,
				    __pgasrt_local_addr_t        src,
				    __pgasrt_dtypes_t            optype,
				    __pgasrt_ops_t               op,
				    __pgasrt_LCompHandler_t      comp_h,
                                    void                       * info);


/* **************************************************************************/
/**
 * \brief send an active message to other node. Sends header and data
 * to destination, and invokes header handler on destination. Header
 * handler will return an address to copy data to.
 * 
 * Local completion semantics (complete when outbound data has cleared
 * local buffers). Local completion results in invocation of send side
 * completion hander (if specified non-NULL). Otherwise user has to
 * wait on handle to ascertain local completion.
 *
 * \param destNode    destination node
 * \param header      active message header
 * \param data        data buffer to be sent
 * \param datalen     length of send data buffer
 * \param comp_h      send side completion handler
 * \param info        send side completion handler argument
 * \returns           a handle to be waited on, or NULL if a send side
 *                    completion handler was specified.
 */
/* **************************************************************************/

EXTERN void * 
__pgasrt_tsp_amsend                (unsigned                     destNode,
				    __pgasrt_AMHeader_t        * header,
				    const __pgasrt_local_addr_t  data,
				    unsigned                     datalen,
				    __pgasrt_LCompHandler_t      comp_h,
				    void                       * info);



/* **************************************************************************/
/**
 * \brief The maximum number of registered header handlers.
 */
/* **************************************************************************/

#define PGASRT_TSP_AMSEND_REG_MAX        100

/* !!!TODO!!! There should be a better place for these defines */

#define PGASRT_TSP_AMSEND_COLLEXCHANGE    99
#define PGASRT_TSP_AMSEND_COLLGATHER      98
#define PGASRT_TSP_AMSEND_COLLSCATTER     97
#define PGASRT_TSP_AMSEND_UPCRTALLOC1     96
#define PGASRT_TSP_AMSEND_UPCRTALLOC2     95
#define PGASRT_TSP_AMSEND_UPCRTLOCK1      94
#define PGASRT_TSP_AMSEND_UPCRTLOCK2      93
#define PGASRT_TSP_AMSEND_BARRIER         92

/* **************************************************************************/
/**
 * \brief Register an amsend header handler. Thus the handler will be
 * accessible from remote nodes by reference to its ID instead of a
 * function pointer.
 * 
 * \param id         a small integer (< \ref PGASRT_TSP_AMSEND_REG_MAX)
 * \param hndlr      a function pointer valid in the current address space
 */
/* **************************************************************************/

EXTERN void
__pgasrt_tsp_amsend_reg            (int                          id,
				    __pgasrt_AMHeaderHandler_t   hndlr);
/* **************************************************************************/
/* **************************************************************************/

EXTERN __pgasrt_AMHeaderHandler_t
__pgasrt_tsp_amsend_lookup         (__pgasrt_AMHeaderHandler_t hndlr);

/* **************************************************************************/
/**
 * \brief barrier. This is a blocking call. All nodes must
 * participate, and nobody returns from the call until every node has
 * entered.
 *
 */
/* **************************************************************************/

EXTERN void __pgasrt_tsp_barrier        ();

/* **************************************************************************/
/** \page tsp Transport subsystem

The transport subsystem provides services for running multi-node jobs
on symmetrical systems. The API is described \ref pgasrt_tsp.h "here"
and provides:

- functions for the initialization and termination of multi-node jobs
  (\ref __pgasrt_tsp_setup, \ref __pgasrt_tsp_finish)

- basic inquiry functions about the number and identity of
participating nodes (\ref PGASRT_MYNODE and \ref PGASRT_NODES),

- functions for GET and PUT operations on remote memory (e.g. \ref
  __pgasrt_tsp_get and \ref __pgasrt_tsp_put)

- basic fence and barrier operations (\ref __pgasrt_tsp_barrier, \ref
  __pgasrt_tsp_fence)

- general purpose remote active message sends (\ref __pgasrt_tsp_amsend)

*/
/* **************************************************************************/


#endif

