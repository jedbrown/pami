/**
 * \file sys/pami.h
 * \brief Common external interface for IBM's PAMI message layer.
 */

#ifndef __pami_h__
#define __pami_h__

#include <stdlib.h>
#include <stdint.h>
#include <sys/uio.h>

#include "pami_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * \brief PAMI result status codes.
   */
  typedef enum
  {
    PAMI_SUCCESS =  0, /**< Successful execution        */
    PAMI_NERROR  = -1, /**< Generic error (-1)          */
    PAMI_ERROR   =  1, /**< Generic error (+1)          */
    PAMI_INVAL,        /**< Invalid argument            */
    PAMI_UNIMPL,       /**< Function is not implemented */
    PAMI_EAGAIN,       /**< Not currently availible     */
    PAMI_ENOMEM,       /**< Out of memory               */
    PAMI_SHUTDOWN,     /**< Task has shutdown           */
    PAMI_CHECK_ERRNO,  /**< Check the errno val         */
    PAMI_OTHER,        /**< Other undefined error       */
  } pami_result_t;

  typedef void*    pami_client_t;   /**< Client of communication contexts */
  typedef void*    pami_context_t;  /**< Context for data transfers       */
  typedef void*    pami_type_t;     /**< Description for data layout      */
  typedef uint32_t pami_task_t;     /**< Identity of a process            */
  typedef uint32_t pami_endpoint_t; /**< Identity of a context            */

  /**
   * \brief Callback to handle message events
   *
   * \param[in] context   PAMI communication context that invoked the callback
   * \param[in] cookie    Event callback application argument
   * \param[in] result    Asynchronous result information
   */
  typedef void (*pami_event_function) ( pami_context_t   context,
                                       void          * cookie,
                                       pami_result_t    result );

  /**
   * \brief Prototype for function used for context-queued work (post)
   *
   * Posted function will continue to run or stop according to the return value
   * described below.
   *
   * \param[in] context   PAMI communication context running function
   * \param[in] cookie    Application argument
   * \return	PAMI_SUCCESS causes function to dequeue (stop running)
   *		PAMI_EAGAIN causes function to remain queued and is called on next advance
   * 		(any other value) causes function to dequeue and (optionally) report error
   */
  typedef pami_result_t (*pami_work_function)(pami_context_t context, void *cookie);

  typedef uintptr_t pami_work_t[8];

  typedef struct
  {
    pami_event_function  function;
    void               *clientdata;
  } pami_callback_t;

/**
 * \brief Message layer operation types
 */

  typedef enum
  {
    PAMI_UNDEFINED_OP = 0,
    PAMI_NOOP,
    PAMI_MAX,
    PAMI_MIN,
    PAMI_SUM,
    PAMI_PROD,
    PAMI_LAND,
    PAMI_LOR,
    PAMI_LXOR,
    PAMI_BAND,
    PAMI_BOR,
    PAMI_BXOR,
    PAMI_MAXLOC,
    PAMI_MINLOC,
    PAMI_USERDEFINED_OP,
    PAMI_OP_COUNT
  }
    pami_op;

  /**
   * \brief Message layer data types
   */

  typedef enum
  {
    /* Standard/Primative DT's */
    PAMI_UNDEFINED_DT = 0,
    PAMI_SIGNED_CHAR,
    PAMI_UNSIGNED_CHAR,
    PAMI_SIGNED_SHORT,
    PAMI_UNSIGNED_SHORT,
    PAMI_SIGNED_INT,
    PAMI_UNSIGNED_INT,
    PAMI_SIGNED_LONG_LONG,
    PAMI_UNSIGNED_LONG_LONG,
    PAMI_FLOAT,
    PAMI_DOUBLE,
    PAMI_LONG_DOUBLE,
    PAMI_LOGICAL,
    PAMI_SINGLE_COMPLEX,
    PAMI_DOUBLE_COMPLEX,
    /* Max/Minloc DT's */
    PAMI_LOC_2INT,
    PAMI_LOC_SHORT_INT,
    PAMI_LOC_FLOAT_INT,
    PAMI_LOC_DOUBLE_INT,
    PAMI_LOC_2FLOAT,
    PAMI_LOC_2DOUBLE,
    PAMI_USERDEFINED_DT,
    PAMI_DT_COUNT
  }
    pami_dt;


  /*****************************************************************************/
  /**
   * \defgroup CA Compact attributes
   * \{
   */
  /*****************************************************************************/

  /** \todo Should this be size_t ??? */
  typedef int pami_ca_mask;

#define PAMI_CA_MASK_NUM_BYTES  (sizeof(pami_ca_mask))
#define PAMI_CA_BYTE_SIZE       (sizeof(char))
#define PAMI_CA_TOTAL_BITS      (PAMI_CA_BYTE_SIZE * PAMI_CA_MASK_NUM_BYTES) /**< \todo Either this definition is wrong, or the name is misleading; this is not a bit-count for the array */
#define PAMI_CA_NUM_ELEMENTS    1

  /* compact list of informative attributes in the form of bits*/
  typedef struct
  {
    /* initially 1 list element of 32 bits, assuming sizeof(pami_ca_mask)==4 */
    pami_ca_mask bits[PAMI_CA_NUM_ELEMENTS];
  } pami_ca_t;

  /**
   * \brief A metadata structure to describe a collective protocol
   */
  typedef struct
  {
    pami_ca_t geometry; /**< geometry attributes                   */
    pami_ca_t buffer;   /**< buffer attributes (contig, alignment) */
    pami_ca_t misc;     /**< other attributes (i.e. threaded)      */
    char name[32];     /**< name of algorithm                     */
  } pami_metadata_t;

  extern void pami_metadata_multiset(pami_ca_t *, ...);

  /** \} */ /* end of "Compact attributes" group */


  /*****************************************************************************/
  /**
   * \defgroup activemessage pami active messaging interface
   *
   * Some brief documentation on active message stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Hints for sending a message
   *
   * \todo better names for the hints
   * \todo better documentation for the hints
   */
  typedef struct
  {
    uint32_t consistency       : 1; /**< Force match ordering semantics                          */
    uint32_t recv_immediate    : 1; /**< Assert that sends will result in an 'immediate' receive */
    uint32_t buffer_registered : 1; /**< Send and receive buffers are ready for RDMA operations  */
    uint32_t use_rdma          : 1; /**< Assert/enable RDMA operations                           */
    uint32_t no_rdma           : 1; /**< Disable RDMA operations                                 */
    uint32_t no_local_copy     : 1; /**< Disable PAMI making a local copy of data                */
    uint32_t interrupt_on_recv : 1; /**< Interrupt the remote task when the first packet arrives */
    uint32_t high_priority     : 1; /**< Message is delivered with high priority,
                                       which may result in out-of-order delivery                 */
    uint32_t no_long_header    : 1; /**< Disable long header support                             */
    uint32_t use_shmem         : 1; /**< Assert/enable shared memory optimizations               */
    uint32_t no_shmem          : 1; /**< Disable shared memory optimizationss                    */

    uint32_t reserved          :21; /**< Unused at this time                                     */
  } pami_send_hint_t;

  typedef struct
  {
    uint32_t reserved          :32; /**< Unused at this time                                     */
  } pami_collective_hint_t;



  /**
   * \brief Active message send common parameters structure
   */
  typedef struct
  {
    struct iovec      header;   /**< Header buffer address and size in bytes */
    struct iovec      data;     /**< Data buffer address and size in bytes */
    size_t            dispatch; /**< Dispatch identifier */
    pami_send_hint_t   hints;    /**< Hints for sending the message */
    pami_endpoint_t    dest;     /**< Destination endpoint */
  } pami_send_immediate_t;

  /**
   * \brief Structure for event notification
   */
  typedef struct
  {
    void               * cookie;   /**< Argument to \b all event callbacks */
    pami_event_function   local_fn; /**< Local message completion event */
    pami_event_function   remote_fn;/**< Remote message completion event */
  } pami_send_event_t;

  /**
   * \brief Structure for send parameters unique to a simple active message send
   */
  typedef struct
  {
    pami_send_immediate_t send;     /**< Common send parameters */
    pami_send_event_t     events;   /**< Non-blocking event parameters */
  } pami_send_t;

  /**
   * \brief Structure for send parameters unique to a typed active message send
   */
  typedef struct
  {
    pami_send_immediate_t send;     /**< Common send parameters */
    pami_send_event_t     events;   /**< Non-blocking event parameters */
    struct
    {
      size_t              offset;   /**< Starting offset in \c datatype */
      pami_type_t         datatype; /**< Datatype */
    } typed;                       /**< Typed send parameters */
  } pami_send_typed_t;

  /**
   * \brief Non-blocking active message send for contiguous data
   *
   * A low-latency send operation may be enhanced by using a dispatch id which
   * was set with the \c recv_immediate hint bit enabled. This hint asserts
   * that all receives with the dispatch id will not exceed a certain limit.
   *
   * The implementation configuration attribute \c PAMI_RECV_IMMEDIATE_MAX
   * defines the maximum size of data buffers that can be completely received
   * with a single dispatch callback. Typically this limit is associated with
   * a network resource attribute, such as a packet size.
   *
   * \see pami_send_hint_t
   * \see PAMI_Configuration_query
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters Send simple parameter structure
   *
   * \retval PAMI_SUCCESS  The request has been accepted.
   * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
   */
  pami_result_t PAMI_Send (pami_context_t    context,
                         pami_send_t     * parameters);

  /**
   * \brief Immediate active message send for small contiguous data
   *
   * The blocking send is only valid for small data buffers. The implementation
   * configuration attribute \c PAMI_SEND_IMMEDIATE_MAX defines the upper
   * bounds for the size of data buffers, including header data, that can be
   * sent with this function. This function will return an error if a data
   * buffer larger than the \c PAMI_SEND_IMMEDIATE_MAX is attempted.
   *
   * This function provides a low-latency send that can be optimized by the
   * specific pami implementation. If network resources are immediately
   * available the send data will be injected directly into the network. If
   * resources are not available the specific pami implementation may internally
   * buffer the send parameters and data until network resource are available
   * to complete the transfer. In either case the send will immediately return,
   * no doce callback is invoked, and is considered complete.
   *
   * The low-latency send operation may be further enhanced by using a dispatch
   * id which was set with the \c recv_immediate hint bit enabled. This hint
   * asserts that all receives with the dispatch id will not exceed a certain
   * limit.
   *
   * The implementation configuration attribute \c PAMI_RECV_IMMEDIATE_MAX
   * defines the maximum size of data buffers that can be completely received
   * with a single dispatch callback. Typically this limit is associated with
   * a network resource attribute, such as a packet size.
   *
   * \see pami_send_hint_t
   * \see PAMI_Configuration_query
   *
   * \todo Better define send parameter structure so done callback is not required
   * \todo Define configuration attribute for the size limit
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters Send parameter structure
   *
   * \retval PAMI_SUCCESS  The request has been accepted.
   * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
   */
  pami_result_t PAMI_Send_immediate (pami_context_t          context,
                                   pami_send_immediate_t * parameters);

  /**
   * \brief Non-blocking active message send for non-contiguous typed data
   *
   * Transfers data according to a predefined data memory layout, or type, to
   * the remote task.
   *
   * Conceptually, the data is transfered as a byte stream which may be
   * received by the remote task into a different format, such as a contiguous
   * buffer or the same or different predefined type.
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters Send typed parameter structure
   *
   * \retval PAMI_SUCCESS  The request has been accepted.
   * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
   */
  pami_result_t PAMI_Send_typed (pami_context_t      context,
                               pami_send_typed_t * parameters);

  /**
   * \brief Active message receive hints
   */
  typedef struct
  {
    uint32_t inline_completion :  1; /**< The receive completion callback
                                      *   \b must be invoked by the thread that
                                      *   receives the dispatch notification. */
    uint32_t reserved          : 31; /**< Reserved. Do not use. */
  } pami_recv_hint_t;

  /**
   * \brief Receive message structure
   *
   * This structure is initialized and then returned as an output parameter from
   * the active message dispatch callback to direct the pami runtime how to
   * receive the data stream.
   *
   * When \c type is \c PAMI_BYTE, the receive buffer is contiguous and it
   * must be large enough to hold the entire message.
   *
   * With non-contiguous \c type, the receive buffer in general must be large
   * enough for the incoming message as well but \c type can be constructed
   * in such a way that unwanted portions of the incoming are disposed into
   * a circular junk buffer.
   *
   * \see pami_dispatch_p2p_fn
   */
  typedef struct
  {
    pami_recv_hint_t        hints;    /**< Hints for receiving the message */
    void                  * cookie;   /**< Argument to \b all event callbacks */
    pami_event_function     local_fn; /**< Local message completion event */
    void                  * addr;     /**< Starting address of the buffer */
    pami_type_t             type;     /**< Datatype */
    size_t                  offset;   /**< Starting offset of the type */
  } pami_recv_t;

  /**
   * \brief Dispatch callback
   *
   * This single dispatch function type supports two kinds of receives:
   * "immediate" and "asynchronous".
   *
   * An immediate receive occurs when the dispatch function is invoked and all
   * of the data sent is \em immediately available in the buffer. In this case
   * \c pipe_addr will point to a valid memory location - even when the number
   * of bytes sent is zero, and the \c recv output structure will be \c NULL.
   *
   * An asynchronous receive occurs when the dispatch function is invoked and
   * all of the data sent is \b not immediately available. In this case the
   * application must provide information to direct how the receive will
   * complete. The \c recv output structure will point to a valid memory
   * location for this purpose, and the \c pipe_addr pointer will be \c NULL.
   * The \c data_size parameter will contain the number of bytes that are being
   * sent from the remote endpoint.
   *
   * \note A zero-byte send will \b always result in an immediate receive.
   *
   * \note The maximum number of bytes that may be immediately received can be
   *       queried with the \c PAMI_RECV_IMMEDIATE configuration attribute.
   *
   * \see PAMI_Configuration_query
   *
   * "pipe" has nothing to do with "PipeWorkQueue"s
   */
  typedef void (*pami_dispatch_p2p_fn) (pami_context_t    context,      /**< IN:  communication context which invoked the dispatch function */
                                        void            * cookie,       /**< IN:  dispatch cookie */
                                        const void      * header_addr,  /**< IN:  header address  */
                                        size_t            header_size,  /**< IN:  header size     */
                                        const void      * pipe_addr,    /**< IN:  address of PAMI pipe  buffer, valid only if non-NULL        */
                                        size_t            data_size,    /**< IN:  number of byts of message data, valid regardless of message type */
                                        pami_endpoint_t   origin,       /**< IN:  Endpoint that originated the transfer */
                                        pami_recv_t     * recv);        /**< OUT: receive message structure, only needed if addr is non-NULL */

  /** \} */ /* end of "active message" group */


  /*****************************************************************************/
  /**
   * \defgroup rma pami remote memory access data transfer interface
   *
   * Some brief documentation on rma stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Input parameters common to all rma transfers
   **/
  typedef struct
  {
    pami_endpoint_t       dest;      /**< Destination endpoint */
    pami_send_hint_t      hints;     /**< Hints for sending the message */
    size_t               bytes;     /**< Data transfer size in bytes */
    void               * cookie;    /**< Argument to \b all event callbacks */
    pami_event_function   done_fn;   /**< Local completion event */
  } pami_rma_t;

  /**
   * \brief Input parameters for rma simple transfers
   **/
  typedef struct
  {
    void               * local;     /**< Local transfer virtual address */
    void               * remote;    /**< Remote transfer virtual address */
  } pami_rma_addr_t;

  /**
   * \brief Input parameters for rma typed transfers
   */
  typedef struct
  {
    pami_type_t           local;     /**< Data type of local buffer */
    pami_type_t           remote;    /**< Data type of remote buffer */
  } pami_rma_typed_t;

  /*****************************************************************************/
  /**
   * \defgroup put pami remote memory access put transfer interface
   *
   * Some brief documentation on put stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Input parameters for rma put transfers
   */
  typedef struct
  {
    pami_event_function   rdone_fn;  /**< Remote completion event - all local
                                         data has been received by remote task */
  } pami_rma_put_t;


  /**
   * \brief Input parameters for simple put transfers
   * \code
   * pami_put_simple_t parameters;
   * parameters.rma.dest     = ;
   * parameters.rma.hints    = ;
   * parameters.rma.bytes    = ;
   * parameters.rma.cookie   = ;
   * parameters.rma.done_fn  = ;
   * parameters.addr.local   = ;
   * parameters.addr.remote  = ;
   * parameters.put.rdone_fn = ;
   * \endcode
   */
  typedef struct
  {
    pami_rma_t            rma;       /**< Common rma parameters */
    pami_rma_addr_t       addr;      /**< Simple rma address parameters */
    pami_rma_put_t        put;       /**< Common put parameters */
  } pami_put_simple_t;

  /**
   * \brief One-sided put operation for simple contiguous data transfer
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters Simple put input parameters
   *
   * \retval PAMI_SUCCESS  The request has been accepted.
   * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
   */
  pami_result_t PAMI_Put (pami_context_t      context,
                        pami_put_simple_t * parameters);

  /**
   * \brief Input parameters for simple typed put transfers
   * \code
   * pami_put_typed_t parameters;
   * parameters.rma.dest     = ;
   * parameters.rma.hints    = ;
   * parameters.rma.bytes    = ;
   * parameters.rma.cookie   = ;
   * parameters.rma.done_fn  = ;
   * parameters.addr.local   = ;
   * parameters.addr.remote  = ;
   * parameters.type.local   = ;
   * parameters.type.remote  = ;
   * parameters.put.rdone_fn = ;
   * \endcode
   */
  typedef struct
  {
    pami_rma_t            rma;       /**< Common rma parameters */
    pami_rma_addr_t       addr;      /**< Simple rma address parameters */
    pami_rma_typed_t      type;      /**< Simple rma typed parameters */
    pami_rma_put_t        put;       /**< Common put parameters */
  } pami_put_typed_t;

  /**
   * \brief One-sided put operation for typed non-contiguous data transfer
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters Typed put input parameters
   *
   * \retval PAMI_SUCCESS  The request has been accepted.
   * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
   */
  pami_result_t PAMI_Put_typed (pami_context_t     context,
                              pami_put_typed_t * parameters);


  /** \} */ /* end of "put" group */

  /*****************************************************************************/
  /**
   * \defgroup get pami remote memory access get transfer interface
   *
   * Some brief documentation on get stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Input parameter structure for simple get transfers
   * \code
   * pami_get_simple_t parameters;
   * parameters.rma.dest    = ;
   * parameters.rma.hints   = ;
   * parameters.rma.bytes   = ;
   * parameters.rma.cookie  = ;
   * parameters.rma.done_fn = ;
   * parameters.addr.local  = ;
   * parameters.addr.remote = ;
   * \endcode
   */
  typedef struct
  {
    pami_rma_t            rma;       /**< Common rma parameters */
    pami_rma_addr_t       addr;      /**< Simple rma address parameters */
  } pami_get_simple_t;

  /**
   * \brief One-sided get operation for simple contiguous data transfer
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters Simple get input parameters
   *
   * \retval PAMI_SUCCESS  The request has been accepted.
   * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
   */
  pami_result_t PAMI_Get (pami_context_t      context,
                        pami_get_simple_t * parameters);

  /**
   * \brief Input parameter structure for typed get transfers
   * \code
   * pami_get_typed_t parameters;
   * parameters.rma.dest    = ;
   * parameters.rma.hints   = ;
   * parameters.rma.bytes   = ;
   * parameters.rma.cookie  = ;
   * parameters.rma.done_fn = ;
   * parameters.addr.local  = ;
   * parameters.addr.remote = ;
   * parameters.type.local  = ;
   * parameters.type.remote = ;
   * \endcode
   */
  typedef struct
  {
    pami_rma_t            rma;       /**< Common rma parameters */
    pami_rma_addr_t       addr;      /**< Simple rma address parameters */
    pami_rma_typed_t      type;      /**< Simple rma typed parameters */
  } pami_get_typed_t;

  /**
   * \brief One-sided get operation for typed non-contiguous data transfer
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters Typed get input parameters
   *
   * \retval PAMI_SUCCESS  The request has been accepted.
   * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
   */
  pami_result_t PAMI_Get_typed (pami_context_t     context,
                              pami_get_typed_t * parameters);

  /** \} */ /* end of "get" group */

  /*****************************************************************************/
  /**
   * \defgroup rmw pami remote memory access read-modify-write interface
   *
   * Some brief documentation on rmw stuff ...
   * \{
   */
  /*****************************************************************************/

  /** \brief Atomic rmw data type */
  typedef enum
  {
    PAMI_RMW_KIND_UINT32      = 0x0001, /**< 32-bit unsigned integer operation */
    PAMI_RMW_KIND_UINT64      = 0x0002  /**< 64-bit unsigned integer operation */
  } pami_rmw_kind_t;

  /** \brief Atomic rmw assignment type */
  typedef enum
  {
    PAMI_RMW_ASSIGNMENT_SET   = 0x0010, /**< =  operation */
    PAMI_RMW_ASSIGNMENT_ADD   = 0x0020, /**< += operation */
    PAMI_RMW_ASSIGNMENT_OR    = 0x0040  /**< |= operation */
  } pami_rmw_assignment_t;

  /** \brief Atomic rmw comparison type */
  typedef enum
  {
    PAMI_RMW_COMPARISON_NOOP  = 0x0100, /**< No comparison operation */
    PAMI_RMW_COMPARISON_EQUAL = 0x0200  /**< Equality comparison operation */
  } pami_rmw_comparison_t;

  typedef struct
  {
    pami_rma_t               rma;       /**< Common rma parameters */
    struct
    {
      pami_rmw_comparison_t  compare;   /**< read-modify-write comparison type */
      pami_rmw_assignment_t  assign;    /**< read-modify-write assignment type */
      pami_rmw_kind_t        kind;      /**< read-modify-write variable type */
      union
      {
        struct
        {
          uint32_t          value;     /**< 32-bit data value */
          uint32_t          test;      /**< 32-bit test value */
        } uint32;                      /**< 32-bit rmw input parameters */
        struct
        {
          uint64_t          value;     /**< 64-bit data value */
          uint64_t          test;      /**< 64-bit test value */
        } uint64;                      /**< 64-bit rmw input parameters */
      } input;
    } rmw;                             /**< Parameters specific to rmw */
  } pami_rmw_t;

  /**
   * \brief Atomic read-modify-write operation to a remote memory location
   *
   * The specific operation is determined by the combination of the three rmw
   * identifier enumeration types. All operations will perform the same
   * generic logical atomic operation:
   *
   * \code
   * *result = *remote; (*remote COMPARISON test) ? *remote ASSIGNMENT value;
   * \endcode
   *
   * \warning All read-modify-write operations are \b unordered relative
   *          to all other data transfer operations - including other
   *          read-modify-write operations.
   *
   * Example read-modify-write operations include:
   *
   * \par PAMI_RMW_KIND_UINT32 | PAMI_RMW_COMPARISON_NOOP | PAMI_RMW_ASSIGNMENT_ADD
   *      "32-bit unsigned integer fetch-and-add operation"
   * \code
   * uint32_t *result, *remote, value, test;
   * *result = *remote; *remote += value;
   * \endcode
   *
   * \par PAMI_RMW_KIND_UINT32 | PAMI_RMW_COMPARISON_NOOP | PAMI_RMW_ASSIGNMENT_OR
   *      "32-bit unsigned integer fetch-and-or operation"
   * \code
   * uint32_t *result, *remote, value, test;
   * *result = *remote; *remote |= value;
   * \endcode
   *
   * \par PAMI_RMW_KIND_UINT64 | PAMI_RMW_COMPARISON_NOOP | PAMI_RMW_ASSIGNMENT_SET
   *      "64-bit unsigned integer swap (fetch-and-set) operation"
   * \code
   * uint64_t *result, *remote, value, test;
   * *result = *remote; *remote = value;
   * \endcode
   *
   * \par PAMI_RMW_KIND_UINT64 | PAMI_RMW_COMPARISON_EQUAL | PAMI_RMW_ASSIGNMENT_SET
   *      "64-bit unsigned integer compare-and-swap operation"
   * \code
   * uint64_t *result, *remote, value, test;
   * *result = *remote; (*remote == test) ? *remote = value;
   * \endcode
   *
   * \param[in] context    PAMI communication context
   * \param[in] parameters read-modify-write input parameters
   *
   * \retval PAMI_SUCCESS  The request has been accepted.
   * \retval PAMI_INVAL    The request has been rejected due to invalid parameters.
   */
  pami_result_t PAMI_Rmw (pami_context_t context, pami_rmw_t * parameters);

  /** \} */ /* end of "rmw" group */

  /*****************************************************************************/
  /**
   * \defgroup rdma pami remote memory access rdma interfaces
   *
   * Some brief documentation on rdma stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Memory region opaque object
   *
   * A memory region must be created before it may be used. The opaque
   * object may be directly transfered to a remote endpoint using the
   * PAMI_Send() or PAMI_Send_immediate() interface.
   *
   * \see PAMI_Memregion_create
   * \see PAMI_Memregion_destroy
   */
  typedef size_t pami_memregion_t[8];

  /**
   * \brief Create a local memory region for one sided operations
   *
   * The local memregion may be transfered, via a send message, to a remote task
   * to allow the remote task to perform one-sided operations with this local
   * task
   *
   * \param[in]  context   PAMI application context
   * \param[in]  address   Base virtual address of the memory region
   * \param[in]  bytes_in  Number of bytes requested
   * \param[out] bytes_out Number of bytes granted
   * \param[out] memregion Memory region object to initialize
   *
   * \retval PAMI_SUCCESS The entire memory region, or a portion of
   *                      the memory region was pinned. The actual
   *                      number of bytes pinned from the start of the
   *                      buffer is returned in the \c bytes_out
   *                      parameter. The memory region must be free'd with
   *                      with PAMI_Memregion_destroy().
   *
   * \retval PAMI_EAGAIN  The memory region was not pinned due to an
   *                      unavailable resource. The memory region does not
   *                      need to be free'd with PAMI_Memregion_destroy().
   *
   * \retval PAMI_INVAL   An invalid parameter value was specified. The memory
   *                      region does not need to be free'd with
   *                      PAMI_Memregion_destroy().
   *
   * \retval PAMI_ERROR   The memory region was not pinned and does not need to
   *                      be free'd with PAMI_Memregion_destroy().
   */
  pami_result_t PAMI_Memregion_create (pami_context_t     context,
                                       void             * address,
                                       size_t             bytes_in,
                                       size_t           * bytes_out,
                                       pami_memregion_t * memregion);

  /**
   * \brief Destroy a local memory region for one sided operations
   *
   * \param[in] context   PAMI application context
   * \param[in] memregion Memory region object
   */
  pami_result_t PAMI_Memregion_destroy (pami_context_t     context,
                                        pami_memregion_t * memregion);

  typedef struct
  {
    pami_memregion_t      * mr;     /**< Memory region */
    size_t                  offset; /**< Offset from beginning of memory region */
  } pami_rma_mr_t;

  typedef struct
  {
    pami_rma_mr_t           local;  /**< Local memory region information */
    pami_rma_mr_t           remote; /**< Remote memory region information */
  } pami_rdma_t;

  /**
   * \brief Input parameter structure for simple rdma put transfers
   * \code
   * pami_rput_simple_t parameters;
   * parameters.rma.dest           = ;
   * parameters.rma.hints          = ;
   * parameters.rma.bytes          = ;
   * parameters.rma.cookie         = ;
   * parameters.rma.done_fn        = ;
   * parameters.rdma.local.mr      = ;
   * parameters.rdma.local.offset  = ;
   * parameters.rdma.remote.mr     = ;
   * parameters.rdma.remote.offset = ;
   * parameters.put.rdone_fn       = ;
   * \endcode
   */
  typedef struct
  {
    pami_rma_t            rma;       /**< Common rma parameters */
    pami_rdma_t           rdma;      /**< Common rdma memregion parameters */
    pami_rma_put_t        put;       /**< Common put parameters */
  } pami_rput_simple_t;

  /**
   * \brief Input parameter structure for typed rdma put transfers
   * \code
   * pami_rput_typed_t parameters;
   * parameters.rma.dest           = ;
   * parameters.rma.hints          = ;
   * parameters.rma.bytes          = ;
   * parameters.rma.cookie         = ;
   * parameters.rma.done_fn        = ;
   * parameters.rdma.local.mr      = ;
   * parameters.rdma.local.offset  = ;
   * parameters.rdma.remote.mr     = ;
   * parameters.rdma.remote.offset = ;
   * parameters.type.local         = ;
   * parameters.type.remote        = ;
   * parameters.put.rdone_fn       = ;
   * \endcode
   */
  typedef struct
  {
    pami_rma_t            rma;       /**< Common rma parameters */
    pami_rdma_t           rdma;      /**< Common rdma memregion parameters */
    pami_rma_typed_t      type;      /**< Common rma typed parameters */
    pami_rma_put_t        put;       /**< Common put parameters */
  } pami_rput_typed_t;

  /**
   * \brief Simple put operation for one-sided contiguous data transfer.
   *
   * \param[in] context    PAMI application context
   * \param[in] parameters Input parameters structure
   */
  pami_result_t PAMI_Rput (pami_context_t context, pami_rput_simple_t * parameters);

  /**
   * \brief Put operation for data type specific one-sided data transfer.
   *
   * \param[in] context    PAMI application context
   * \param[in] parameters Input parameters structure
   */
  pami_result_t PAMI_Rput_typed (pami_context_t context, pami_rput_typed_t * parameters);

  /**
   * \brief Input parameter structure for simple rdma get transfers
   * \code
   * pami_rget_simple_t parameters;
   * parameters.rma.dest           = ;
   * parameters.rma.hints          = ;
   * parameters.rma.bytes          = ;
   * parameters.rma.cookie         = ;
   * parameters.rma.done_fn        = ;
   * parameters.rdma.local.mr      = ;
   * parameters.rdma.local.offset  = ;
   * parameters.rdma.remote.mr     = ;
   * parameters.rdma.remote.offset = ;
   * \endcode
   */
  typedef struct
  {
    pami_rma_t            rma;       /**< Common rma parameters */
    pami_rdma_t           rdma;      /**< Common rdma memregion parameters */
  } pami_rget_simple_t;

  /**
   * \brief Simple get operation for one-sided contiguous data transfer.
   *
   * \param[in] context    PAMI application context
   * \param[in] parameters Input parameters structure
   */
  pami_result_t PAMI_Rget (pami_context_t context, pami_rget_simple_t * parameters);

  /**
   * \brief Input parameter structure for typed rdma get transfers
   * \code
   * pami_rget_typed_t parameters;
   * parameters.rma.dest           = ;
   * parameters.rma.hints          = ;
   * parameters.rma.bytes          = ;
   * parameters.rma.cookie         = ;
   * parameters.rma.done_fn        = ;
   * parameters.rdma.local.mr      = ;
   * parameters.rdma.local.offset  = ;
   * parameters.rdma.remote.mr     = ;
   * parameters.rdma.remote.offset = ;
   * parameters.type.local         = ;
   * parameters.type.remote        = ;
   * \endcode
   */
  typedef struct
  {
    pami_rma_t            rma;       /**< Common rma parameters */
    pami_rdma_t           rdma;      /**< Common rdma memregion parameters */
    pami_rma_typed_t      type;      /**< Common rma typed parameters */
  } pami_rget_typed_t;

  /**
   * \brief Get operation for data type specific one-sided data transfer.
   *
   * \param[in] context    PAMI application context
   * \param[in] parameters Input parameters structure
   */
  pami_result_t PAMI_Rget_typed (pami_context_t context, pami_rget_typed_t * parameters);

  /** \} */ /* end of "rdma" group */
  /** \} */ /* end of "rma" group */



  /*****************************************************************************/
  /**
   * \defgroup dynamictasks pami dynamic task interface
   *
   * Some brief documentation on dynamic task stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Clean up local resources to an endpoint in preparation for
   *        task shutdown or checkpoint
   *
   *        It is the user of this API's responsibility to ensure
   *        that all communication has been quiesced to and from
   *        the destination via a fence call and synchronization
   *
   * \param[in] context    PAMI communication context
   * \param[in] dest       Array of destination endpoints to close connections to
   * \param[in] count      Number of endpoints in the array dest
   */

  pami_result_t PAMI_Purge (pami_context_t    context,
                          pami_endpoint_t * dest,
                          size_t           count);

  /**
   * \brief Setup local resources to an endpoint in preparation for
   *        task restart or creation
   *
   * \param[in] context    PAMI communication context
   * \param[in] dest       Array of destination endpoints to resume connections to
   * \param[in] count      Number of endpoints in the array dest
   */
  pami_result_t PAMI_Resume (pami_context_t    context,
                           pami_endpoint_t * dest,
                           size_t           count);

  /** \} */ /* end of "dynamic tasks" group */


  /*****************************************************************************/
  /**
   * \defgroup sync pami memory synchronization and data fence interface
   *
   * Some brief documentation on sync stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Begin a memory synchronization region
   *
   * A fence region is defined as an area of program control on the local task
   * bounded by the PAMI_Fence_begin() and PAMI_Fence_end() functions.
   *
   * \warning It is considered \b illegal to invoke a fence operation outside of
   *          a fence region.
   *
   * \warning It is considered \b illegal to begin a fence region inside an
   *          existing fence region. Fence regions can not be nested.
   *
   * \param[in] context PAMI communication context
   */
  pami_result_t PAMI_Fence_begin (pami_context_t context);

  /**
   * \brief End a memory synchronization region
   *
   * A fence region is defined as an area of program control on the local task
   * bounded by the PAMI_Fence_begin() and PAMI_Fence_end() functions.
   *
   * \warning It is considered \b illegal to invoke a fence operation outside of
   *          a fence region.
   *
   * \warning It is considered \b illegal to end a fence region outside of an
   *          existing fence region.
   *
   * \param[in] context PAMI communication context
   */
  pami_result_t PAMI_Fence_end (pami_context_t context);


  /**
   * \brief Syncronize all transfers between all endpoints on a context.
   *
   * \param[in] context PAMI communication context
   * \param[in] done_fn Event callback to invoke when the fence is complete
   * \param[in] cookie  Event callback argument
   */
  pami_result_t PAMI_Fence_all (pami_context_t        context,
                              pami_event_function   done_fn,
                              void               * cookie);

  /**
   * \brief Syncronize all transfers to an endpoints.
   *
   * \param[in] context PAMI communication context
   * \param[in] done_fn Event callback to invoke when the fence is complete
   * \param[in] cookie  Event callback argument
   * \param[in] target  Endpoint to synchronize
   */
  pami_result_t PAMI_Fence_endpoint (pami_context_t        context,
                                   pami_event_function   done_fn,
                                   void               * cookie,
                                   pami_endpoint_t       target);

  /** \} */ /* end of "sync" group */


  /* ************************************************************************* */
  /* ********* Transfer Types, used by geometry and xfer routines ************ */
  /* ************************************************************************* */
  typedef enum
  {
    PAMI_XFER_BROADCAST = 0,
    PAMI_XFER_ALLREDUCE,
    PAMI_XFER_REDUCE,
    PAMI_XFER_ALLGATHER,
    PAMI_XFER_ALLGATHERV,
    PAMI_XFER_ALLGATHERV_INT,
    PAMI_XFER_SCATTER,
    PAMI_XFER_SCATTERV,
    PAMI_XFER_SCATTERV_INT,
    PAMI_XFER_BARRIER,
    PAMI_XFER_ALLTOALL,
    PAMI_XFER_ALLTOALLV,
    PAMI_XFER_ALLTOALLV_INT,
    PAMI_XFER_SCAN,
    PAMI_XFER_AMBROADCAST,
    PAMI_XFER_AMSCATTER,
    PAMI_XFER_AMGATHER,
    PAMI_XFER_AMREDUCE,
    PAMI_XFER_COUNT
  } pami_xfer_type_t;

  /* ************************************************************************* */
  /* **************     Geometry (like groups/communicators)  **************** */
  /* ************************************************************************* */

  typedef void*   pami_geometry_t;
  typedef size_t  pami_algorithm_t;

  typedef struct
  {
    size_t lo, hi;
  }
    pami_geometry_range_t;

  /**
   * \brief Initialize the geometry
   *        A synchronizing operation will take place during geometry_initialize
   *        on the parent geometry
   *        If the output geometry "geometry" is NULL, then no geometry will be
   *        created, however, all nodes in the parent must participate in the
   *        geometry_initialize operation, even if they do not create a geometry
   *
   * \param[in]  client         pami client
   * \param[out] geometry        Opaque geometry object to initialize
   * \param[in] parent          Parent geometry containing all the nodes in the task list
   * \param[in]  id              Identifier for this geometry
   *                             which uniquely represents this geometry(if tasks overlap)
   * \param[in]  task_slices     Array of node slices participating in the geometry
   *                             User must keep the array of slices in memory for the
   *                             duration of the geometry's existence
   * \param[in]  slice_count     Number of task slices participating in the geometry
   * \param[in]  context         context to deliver async callback to
   * \param[in]  fn              event function to call when geometry has been created
   * \param[in]  cookie          user cookie to deliver with the callback
   */

  pami_result_t PAMI_Geometry_create_taskrange (pami_client_t                client,
                                        pami_geometry_t            * geometry,
                                              pami_geometry_t              parent,
                                        unsigned                    id,
                                        pami_geometry_range_t      * task_slices,
                                              size_t                      slice_count,
                                              pami_context_t               context,
                                              pami_event_function          fn,
                                              void                      * cookie);

   /**
   * \brief Initialize the geometry
   *        A synchronizing operation will take place during geometry_initialize
   *        on the parent geometry
   *        If the output geometry "geometry" is NULL, then no geometry will be
   *        created, however, all nodes in the parent must participate in the
   *        geometry_initialize operation, even if they do not create a geometry
   *
   * \param[in]  client         pami client
   * \param[out] geometry        Opaque geometry object to initialize
   * \param[in] parent          Parent geometry containing all the nodes in the task list
   * \param[in]  id              Identifier for this geometry
   *                             which uniquely represents this geometry(if tasks overlap)
   * \param[in]  tasks           Array of tasks to build the geometry list
   *                             User must keep the task list in memory
   *                             duration of the geometry's existence
   * \param[in]  task_count      Number of tasks participating in the geometry
   * \param[in]  context         context to deliver async callback to
   * \param[in]  fn              event function to call when geometry has been created
   * \param[in]  cookie          user cookie to deliver with the callback
   */

  pami_result_t PAMI_Geometry_create_tasklist (pami_client_t                client,
                                             pami_geometry_t            * geometry,
                                             pami_geometry_t              parent,
                                             unsigned                    id,
                                             pami_task_t                * tasks,
                                             size_t                      task_count,
                                             pami_context_t               context,
                                             pami_event_function          fn,
                                             void                      * cookie);

  /**
   * \brief Initialize the geometry
   *
   * \param[in]  client          pami client
   * \param[in]  world_geometry  world geometry object
   */
  pami_result_t PAMI_Geometry_world (pami_client_t                client,
                                   pami_geometry_t            * world_geometry);

  /**
   * \brief determines the number of algorithms available for a given op
   *        in the two different lists (always work list,
   *        under-cetain conditions list).
   *
   * \param[in]     context       pami context
   * \param[in]     geometry      An input geometry to be analyzed.
   * \param[in]     coll_type     type of collective op.
   * \param[in,out] lists_lengths array of 2 numbers representing all valid
   algorithms and optimized algorithms.
   * \retval        PAMI_SUCCESS   number of algorithms is determined.
   * \retval        ?????         There is an error with input parameters
   */
  pami_result_t PAMI_Geometry_algorithms_num (pami_context_t context,
                                            pami_geometry_t geometry,
                                            pami_xfer_type_t coll_type,
                                            int              *lists_lengths);

  /**
   * \brief fills in the protocols and attributes for a set of algorithms
   *        The first lists are used to populate collectives that work under
   *        any condidtion.  The second lists are used to populate
   *        collectives that the metadata must be checked before use
   *
   * \param[in]     context        pami context
   * \param[in]     coll_type      type of collective op.
   * \param[in,out] algs0          array of algorithms to query
   * \param[in,out] mdata0         metadata array to be filled in if algorithms
   *                               are applicable, can be NULL.
   * \param[in]     num0           number of algorithms to fill in.
   * \param[in,out] algs1          array of algorithms to query
   * \param[in,out] mdata1         metadata array to be filled in if algorithms
   *                               are applicable, can be NULL.
   * \param[in]     num1           number of algorithms to fill in.
   * \retval        PAMI_SUCCESS    algorithm is applicable to geometry.
   * \retval        ?????          Error in input arguments or not applicable.
   */
  pami_result_t PAMI_Geometry_algorithms_query (pami_context_t context,
                                             pami_geometry_t geometry,
                                             pami_xfer_type_t  colltype,
                                             pami_algorithm_t *algs0,
                                             pami_metadata_t  *mdata0,
                                             int              num0,
                                             pami_algorithm_t *algs1,
                                             pami_metadata_t  *mdata1,
                                             int              num1);

  /**
   * \brief Free any memory allocated inside of a geometry.
   * \param[in] client   pami client
   * \param[in] geometry The geometry object to free
   * \retval PAMI_SUCCESS Memory free didn't fail
   */
  pami_result_t PAMI_Geometry_destroy(pami_client_t    client,
                                     pami_geometry_t  geometry);

  /**
   * \brief Create and post a non-blocking alltoall vector operation.
   *
   * The alltoallv operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sndbuf       The base address of the buffers containing data to be sent
   * \param[in]  stype        A single type datatype
   * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
   * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
   * \param[out] rcvbuf       The base address of the buffer for data reception
   * \param[in]  rtype        A single type datatype
   * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
   * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
   *
   * \retval     0            Success
   *
   */
  typedef struct
  {
    char                     * sndbuf;
    pami_type_t               * stype;
    size_t                   * stypecounts;
    size_t                   * sdispls;
    char                     * rcvbuf;
    pami_type_t               * rtype;
    size_t                   * rtypecounts;
    size_t                   * rdispls;
  } pami_alltoallv_t;

  /**
   * \brief Create and post a non-blocking alltoall vector operation.
   *
   * The alltoallv_int operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sndbuf       The base address of the buffers containing data to be sent
   * \param[in]  stype        A single type datatype
   * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
   * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
   * \param[out] rcvbuf       The base address of the buffer for data reception
   * \param[in]  rtype        A single type datatype
   * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
   * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
   *
   * \retval     0            Success
   *
   */
  typedef struct
  {
    char                    * sndbuf;
    pami_type_t              * stype;
    int                     * stypecounts;
    int                     * sdispls;
    char                    * rcvbuf;
    pami_type_t              * rtype;
    int                     * rtypecounts;
    int                     * rdispls;
  } pami_alltoallv_int_t;


  /**
   * \brief Create and post a non-blocking alltoall operation.
   * The alltoall operation ...
   *
   * \param[in]  cb_done     Callback to invoke when message is complete.
   * \param[in]  geometry    Geometry to use for this collective operation.
   * \param[in]  sndbuf      The base address of the buffers containing data to be sent
   * \param[in]  stype       Single datatype of the send buffer
   * \param[in]  stypecount  Single type replication count
   * \param[out] rbuf        The base address of the buffer for data reception
   * \param[in]  rtype       Single datatype of the receive buffer
   * \param[in]  rtypecount  Single type replication count
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    char                      * sndbuf;
    pami_type_t                * stype;
    size_t                      stypecount;
    char                      * rcvbuf;
    pami_type_t                * rtype;
    size_t                      rtypecount;
  } pami_alltoall_t;

  /**
   * \brief Create and post a non-blocking reduce operation.
   * The reduce operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         Task ID of the reduce root node.
   * \param[in]  sbuffer      Source buffer.
   * \param[in]  stype        Source buffer type
   * \param[in]  stypecount   Source buffer type count
   * \param[in]  rbuffer      Receive buffer.
   * \param[in]  rtype        Receive buffer layout
   * \param[in]  rtypecount   Receive buffer type count
   * \param[in]  dt           Element data type
   * \param[in]  op           Reduce operation
   *
   * \retval     0            Success
   */
  typedef struct
  {
    size_t                     root;
    char                     * sndbuf;
    pami_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    pami_type_t               * rtype;
    size_t                     rtypecount;
    pami_dt                     dt;
    pami_op                     op;
  } pami_reduce_t;

  /**
   * \brief Create and post a non-blocking reduce_scatter operation.
   *
   * The reduce_scatter operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sbuffer      Source buffer.
   * \param[in]  stype        Source buffer type
   * \param[in]  stypecount   Source buffer type count
   * \param[in]  rbuffer      Receive buffer.
   * \param[in]  rtype        Receive buffer layout
   * \param[in]  rtypecount   Receive buffer type count
   * \param[in]  rcounts      number of elements to receive from the destinations(common on all nodes)
   * \param[in]  dt           Element data type
   * \param[in]  op           Reduce operation
   *
   * \retval     0            Success
   *
   */
  typedef struct
  {
    char                    * sndbuf;
    pami_type_t              * stype;
    size_t                    stypecount;
    char                    * rcvbuf;
    pami_type_t              * rtype;
    size_t                    rtypecount;
    size_t                  * rcounts;
    pami_dt                    dt;
    pami_op                    op;
  } pami_reduce_scatter_t;

  /**
   * \brief Create and post a non-blocking broadcast operation.
   *
   * The broadcast operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         Task ID of the node performing the broadcast.
   * \param[in]  buf          Source buffer to broadcast on root, dest buffer on non-root
   * \param[in]  type         data type layout, may be different on root/destinations
   * \param[in]  count        Single type replication count
   *
   * \retval     0            Success
   *
   */
  typedef struct
  {
    size_t                      root;
    char                      * buf;
    pami_type_t                * type;
    size_t                      typecount;
  } pami_broadcast_t;


  /**
   * \brief Create and post a non-blocking allgather
   *
   * The allgather
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  src          Source buffer to send
   * \param[in]  stype        data layout of send buffer
   * \param[in]  stypecount   replication count of the type
   * \param[in]  rcv          Source buffer to receive the data
   * \param[in]  rtype        data layout of each receive buffer
   * \param[in]  rtypecount   replication count of the type
   *
   * \retval     0            Success
   *
   */
  typedef struct
  {
    char                      * sndbuf;
    pami_type_t                * stype;
    size_t                      stypecount;
    char                      * rcvbuf;
    pami_type_t                * rtype;
    size_t                      rtypecount;
  } pami_allgather_t;


  /**
   * \brief Create and post a non-blocking allgather
   *
   * The gather
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         The root node of the gather operation
   * \param[in]  src          Source buffer to send
   * \param[in]  stype        data layout of send buffer
   * \param[in]  stypecount   replication count of the type
   * \param[in]  rcv          Source buffer to receive the data
   * \param[in]  rtype        data layout of each receive buffer
   * \param[in]  rtypecount   replication count of the type
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                      root;
    char                      * sndbuf;
    pami_type_t                * stype;
    size_t                      stypecount;
    char                      * rcvbuf;
    pami_type_t                * rtype;
    size_t                      rtypecount;
  } pami_gather_t;

  /**
   * \brief Create and post a non-blocking gatherv
   *
   * The gatherv
   *
   * \param[in]  request      Opaque memory to maintain internal message state.
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         The root node for the gatherv operation
   * \param[in]  sndbuf       The base address of the buffers containing data to be sent
   * \param[in]  stype        A single type datatype
   * \param[in]  stypecount   type replication count.
   * \param[out] rcvbuf       The base address of the buffer for data reception
   * \param[in]  rtype        A single type datatype
   * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
   * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                     root;
    char                     * sndbuf;
    pami_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    pami_type_t               * rtype;
    size_t                   * rtypecounts;
    size_t                   * rdispls;
  } pami_gatherv_t;

  /**
   * \brief Create and post a non-blocking gatherv
   *
   * The gatherv_int routine
   *
   * \param[in]  request      Opaque memory to maintain internal message state.
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         The root node for the gatherv operation
   * \param[in]  sndbuf       The base address of the buffers containing data to be sent
   * \param[in]  stype        A single type datatype
   * \param[in]  stypecount   type replication count.
   * \param[out] rcvbuf       The base address of the buffer for data reception
   * \param[in]  rtype        A single type datatype
   * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
   * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                     root;
    char                     * sndbuf;
    pami_type_t               * stype;
    int                        stypecount;
    char                     * rcvbuf;
    pami_type_t               * rtype;
    int                      * rtypecounts;
    int                      * rdispls;
  } pami_gatherv_int_t;


  /**
   * \brief Create and post a non-blocking allgatherv
   *
   * The allgatherv
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sndbuf       The base address of the buffers containing data to be sent
   * \param[in]  stype        A single type datatype
   * \param[in]  stypecount   type replication count.
   * \param[out] rcvbuf       The base address of the buffer for data reception
   * \param[in]  rtype        A single type datatype
   * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
   * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    char                     * sndbuf;
    pami_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    pami_type_t               * rtype;
    size_t                   * rtypecounts;
    size_t                   * rdispls;
  } pami_allgatherv_t;

  /**
   * \brief Create and post a non-blocking allgatherv
   *
   * The allgatherv_int
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sndbuf       The base address of the buffers containing data to be sent
   * \param[in]  stype        A single type datatype
   * \param[in]  stypecount   type replication count.
   * \param[out] rcvbuf       The base address of the buffer for data reception
   * \param[in]  rtype        A single type datatype
   * \param[in]  rtypecounts  Array of type replication counts.  Size of geometry length
   * \param[in]  rdispls      Array of offsets into the rcvbuf.  Size of geometry length
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    char                     * sndbuf;
    pami_type_t               * stype;
    int                        stypecount;
    char                     * rcvbuf;
    pami_type_t               * rtype;
    int                      * rtypecounts;
    int                      * rdispls;
  } pami_allgatherv_int_t;


  /**
   * \brief Create and post a non-blocking scatter
   *
   * The scatter
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         Task ID of the reduce root node.
   * \param[in]  sbuffer      Source buffer.
   * \param[in]  stype        Source buffer type
   * \param[in]  stypecount   Source buffer type count
   * \param[in]  rbuffer      Receive buffer.
   * \param[in]  rtype        Receive buffer layout
   * \param[in]  rtypecount   Receive buffer type count
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                      root;
    char                      * sndbuf;
    pami_type_t                * stype;
    size_t                      stypecount;
    char                      * rcvbuf;
    pami_type_t                * rtype;
    size_t                      rtypecount;
  } pami_scatter_t;

  /**
   * \brief Create and post a non-blocking scatterv
   *
   * The scatterv
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sndbuf       The base address of the buffers containing data to be sent
   * \param[in]  stype        A single type datatype
   * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
   * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
   * \param[in]  rbuffer      Receive buffer.
   * \param[in]  rtype        A single type datatype
   * \param[in]  rtypecount   Receive buffer type replication count
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                      root;
    char                      * sndbuf;
    pami_type_t                * stype;
    size_t                    * stypecounts;
    size_t                    * sdispls;
    char                      * rcvbuf;
    pami_type_t                * rtype;
    size_t                      rtypecount;
  } pami_scatterv_t;

  /**
   * \brief Create and post a non-blocking scatterv
   *
   * The scatterv_int
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sndbuf       The base address of the buffers containing data to be sent
   * \param[in]  stype        A single type datatype
   * \param[in]  stypecounts  An array of type replication counts.  Size of geometry length
   * \param[in]  sdispls      Array of offsets into the sndbuf.  Size of geometry length
   * \param[in]  rbuffer      Receive buffer.
   * \param[in]  rtype        A single type datatype
   * \param[in]  rtypecount   Receive buffer type replication count
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                      root;
    char                      * sndbuf;
    pami_type_t                * stype;
    int                       * stypecounts;
    int                       * sdispls;
    char                      * rcvbuf;
    pami_type_t                * rtype;
    int                         rtypecount;
  } pami_scatterv_int_t;


  /**
   * \brief Create and post a non-blocking allreduce operation.
   *
   * The allreduce operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sbuffer      Source buffer.
   * \param[in]  stype        Source buffer type
   * \param[in]  stypecount   Source buffer type count
   * \param[in]  rbuffer      Receive buffer.
   * \param[in]  rtype        Receive buffer layout
   * \param[in]  rtypecount   Receive buffer type count
   * \param[in]  dt           Element data type
   * \param[in]  op           Reduce operation
   *
   * \retval     0            Success
   *
   * \todo doxygen
   * \todo discuss collapsing PAMI_dt into type type.
   */
  typedef struct
  {
    char                     * sndbuf;
    pami_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    pami_type_t               * rtype;
    size_t                     rtypecount;
    pami_dt                     dt;
    pami_op                     op;
  } pami_allreduce_t;


  /**
   * \brief Create and post a non-blocking scan operation.
   *
   * The scan operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  sbuffer      Source buffer.
   * \param[in]  stype        Source buffer type
   * \param[in]  stypecount   Source buffer type count
   * \param[in]  rbuffer      Receive buffer.
   * \param[in]  rtype        Receive buffer layout
   * \param[in]  rtypecount   Receive buffer type count
   * \param[in]  dt           Element data type
   * \param[in]  op           Reduce operation
   * \param[in]  exclusive    scan operation is exclusive of current node?
   *
   * \retval     0            Success
   *
   * \todo doxygen
   * \todo discuss collapsing PAMI_dt into type type.
   */
  typedef struct
  {
    char                     * sndbuf;
    pami_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    pami_type_t               * rtype;
    size_t                     rtypecount;
    pami_dt                     dt;
    pami_op                     op;
    int                        exclusive;
  } pami_scan_t;

  /**
   * \brief Create and post a non-blocking barrier operation.
   * The barrier operation ...
   * \param      geometry     Geometry to use for this collective operation.
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \retval  0            Success
   *
   * \see PAMI_Barrier_register
   *
   * \todo doxygen
   */
  typedef struct
  {
  } pami_barrier_t;


  /**
   * \brief Create and post a non-blocking active message broadcast operation.
   * The Active Message broadcast operation ...
   *
   * This differs from AMSend in only one particular: it takes geometry/team
   * as an argument. The semantics are as follows: the included header and data
   * are broadcast to every place in the team. The completion handler is invoked
   * on the sender side as soon as send buffers can be reused. On the receive
   * side the usual two-phase reception protocol is executed: a header handler
   * determines the address to which to deposit the data and sets the address
   * of a receive completion hander to be invoked once the data has arrived.
   *
   * \param[in]  dispatch     registered dispatch id to use
   * \param[in]  user_header  single metadata to send to destination in the header
   * \param[in]  headerlen    length of the metadata (can be 0)
   * \param[in]  src          Base source buffer to broadcast.
   * \param[in]  stype        Datatype of the send buffer
   * \param[in]  stypecount   replication count of the send buffer data type
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                      dispatch;
    void                      * user_header;
    size_t                      headerlen;
    void                      * sndbuf;
    pami_type_t                * stype;
    size_t                      stypecount;
  } pami_ambroadcast_t;
  /**
   * \brief The active message callback function, delivered to the user
   * \param[in]   root       system defined metadata:  root initiating the broadcast
   * \param[in]   comm       system defined metadata:  geometry id of the broadcast
   * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
   * \param[in]   user_header user defined metadata:  active message header
   * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
   * \param[out]  rcvbuf     target buffer for the incoming collective
   * \param[out]  rtype      data layout of the incoming collective
   * \param[out]  rtypecount replication count of the incoming collective
   * \param[out]  cb_info    data done callback to call on completion
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef void (*pami_dispatch_ambroadcast_fn) (pami_context_t         context,
                                                size_t                 root,
                                                pami_geometry_t        geometry,
                                                const size_t           sndlen,
                                                const void           * user_header,
                                                const size_t           headerlen,
                                                void                ** rcvbuf,
                                                pami_type_t          * rtype,
                                                size_t               * rtypecount,
                                                pami_event_function  * const cb_info,
                                                void                ** cookie);


  /**
   * \brief Create and post a non-blocking active message scatter operation.
   * The Active Message scatter operation ...
   *
   * This is slightly more complicated than an AMBroadcast, because it allows
   * different headers and data buffers to be sent to everyone in the team.
   *
   * \param[in]  dispatch     registered dispatch id to use
   * \param[in]  headers      array of  metadata to send to destination
   * \param[in]  headerlength length of every header in the headers array
   * \param[in]  src          Base source buffer to scatter (size of geometry)
   * \param[in]  stype        single Datatype of the send buffer
   * \param[in]  stypecount   replication count of the send buffer data type
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                      dispatch;
    void                      * headers;
    size_t                      headerlen;
    void                      * sndbuf;
    pami_type_t                * stype;
    size_t                      stypecount;
  } pami_amscatter_t;
  /**
   * \brief The active message callback function, delivered to the user
   * \param[in]   root       system defined metadata:  root initiating the scatter
   * \param[in]   comm       system defined metadata:  geometry id of the scatter
   * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
   * \param[in]   user_header user defined metadata:  active message header
   * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
   * \param[out]  rcvbuf     target buffer for the incoming collective
   * \param[out]  rtype      data layout of the incoming collective
   * \param[out]  rtypecount replication count of the incoming collective
   * \param[out]  cb_info    data done callback to call on completion
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef void (*pami_dispatch_amscatter_fn) (size_t                 root,
                                              pami_geometry_t        geometry,
                                              const unsigned         sndlen,
                                              const void           * user_header,
                                              const size_t           headerlen,
                                              void                ** rcvbuf,
                                              pami_type_t          * rtype,
                                              size_t               * rtypecount,
                                              pami_event_function  * const cb_info);

  /**
   * \brief Create and post a non-blocking active message gather operation.
   * The Active Message gather operation ...
   *
   * This is the reverse of amscatter. It works as follows. The header only,
   * no data, is broadcast to the team. Each place in the team executes the
   * header handler and points to a data buffer in local space. A reverse transfer
   * then takes place (the buffer is sent from the receiver back to the sender,
   * and deposited in one of the buffers provided as part of the original call
   * (the "data" parameter).
   *
   * \param[in]  dispatch     registered dispatch id to use
   * \param[in]  headers      array of metadata to send to destination
   * \param[in]  headerlen    length of every header in headers array
   * \param[in]  rcvbuf       target buffer of the gather operation (size of geometry)
   * \param[in]  rtype        data layout of the incoming gather
   * \param[in]  rtypecount   replication count of the incoming gather
   * \param[in]  cb_info      data done callback to call on completion
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                      dispatch;
    void                      * headers;
    size_t                      headerlen;
    void                      * rcvbuf;
    pami_type_t                * rtype;
    size_t                      rtypecount;
  } pami_amgather_t;
  /**
   * \brief The active message callback function, delivered to the user
   * \param[in]   root       system defined metadata:  root initiating the gather
   * \param[in]   comm       system defined metadata:  geometry id of the gather
   * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
   * \param[in]   user_header user defined metadata:  active message header
   * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
   * \param[out]  sbuf       source buffer for the incoming collective
   * \param[out]  stype      data layout of the send buffer
   * \param[out]  stypecount replication count of the send buffer
   * \param[out]  cb_info    data done callback to call on completion
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef void (*pami_dispatch_amgather_fn) (size_t                 root,
                                             pami_geometry_t        geometry_id,
                                             const unsigned         sndlen,
                                             const void           * user_header,
                                             const size_t           headerlen,
                                             void                ** sndbuf,
                                             pami_type_t          * stype,
                                             size_t               * stypecount,
                                             pami_event_function  * const cb_info);

  /**
   * \brief Create and post a non-blocking active message reduce operation.
   * The Active Message reduce operation ...
   *
   * This is fairly straightforward given how amgather works. Instead of
   * collecting the data without processing, all buffers are reduced using the
   * operation and data type provided by the sender. The final reduced data is
   * deposited in the original buffer provided by the initiator. On the receive
   * side the algorithm has the right to change the buffers provided by the header
   * handler (this may avoid having the implementor allocate more memory for
   * internal buffering)
   *
   * \param[in]  dispatch     registered dispatch id to use
   * \param[in]  geometry     Geometry to use for this collective operation.
   *                          \c NULL indicates the global geometry.
   * \param[in]  headers      metadata to send to destinations in the header
   * \param[in]  rcvbuf       target buffer of the reduce operation (size of geometry)
   * \param[in]  rtype        data layout of the incoming reduce
   * \param[in]  rtypecount   replication count of the incoming reduce
   * \param[in]  dt           datatype of reduction operation
   * \param[in]  op           operation type
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef struct
  {
    size_t                      dispatch;
    void                      * user_header;
    size_t                      headerlen;
    void                      * rcvbuf;
    pami_type_t                * rtype;
    size_t                      rtypecount;
    pami_dt                      dt;
    pami_op                      op;
  } pami_amreduce_t;

  /**
   * \brief The active message callback function, delivered to the user
   *        Note that the user does no math in these routines, just provides the buffer.
   *        The system provided metadata is information that the underlying messaging stack
   *        must transmit anyways, so it is delivered to the user for extra information about
   *        the incoming message
   * \param[in]   root       system defined metadata:  root initiating the reduce
   * \param[in]   comm       system defined metadata:  geometry id of the reduce
   * \param[in]   dt         system defined metadata:  datatype of reduction operation
   * \param[in]   op         system defined metadata:  operation type
   * \param[in]   sndlen     system defined metadata:  total number of bytes of the send
   * \param[in]   user_header user defined metadata:  active message header
   * \param[in]   headerlen  length of the user defined header (may be 0 bytes)
   * \param[out]  sbuf       source buffer for the incoming collective
   * \param[out]  stype      data layout of the send buffer
   * \param[out]  stypecount replication count of the send buffer
   * \param[out]  cb_info    data done callback to call on completion
   *
   * \retval     0            Success
   *
   * \todo doxygen
   */
  typedef void (*pami_dispatch_amreduce_fn) (size_t                 root,
                                             pami_geometry_t        geometry_id,
                                             const unsigned         sndlen,
                                             pami_dt                dt,
                                             pami_op                op,
                                             const void           * user_header,
                                             const size_t           headerlen,
                                             void                ** sndbuf,
                                             pami_type_t          * stype,
                                             size_t               * stypecount,
                                             pami_event_function  * const cb_info);


  typedef union
  {
    pami_allreduce_t        xfer_allreduce;
    pami_broadcast_t        xfer_broadcast;
    pami_reduce_t           xfer_reduce;
    pami_allgather_t        xfer_allgather;
    pami_allgatherv_t       xfer_allgatherv;
    pami_allgatherv_int_t   xfer_allgatherv_int;
    pami_scatter_t          xfer_scatter;
    pami_scatterv_t         xfer_scatterv;
    pami_scatterv_int_t     xfer_scatterv_int;
    pami_scatter_t          xfer_gather;
    pami_scatter_t          xfer_gatherv;
    pami_scatterv_t         xfer_gatherv_int;
    pami_alltoall_t         xfer_alltoall;
    pami_alltoallv_t        xfer_alltoallv;
    pami_alltoallv_int_t    xfer_alltoallv_int;
    pami_ambroadcast_t      xfer_ambroadcast;
    pami_amscatter_t        xfer_amscatter;
    pami_amgather_t         xfer_amgather;
    pami_amreduce_t         xfer_amreduce;
    pami_scan_t             xfer_scan;
    pami_barrier_t          xfer_barrier;
    } pami_collective_t;

  typedef struct
  {
    pami_event_function       cb_done;
    void                   * cookie;
    pami_algorithm_t          algorithm;
    pami_collective_t         cmd;
  } pami_xfer_t;

  pami_result_t PAMI_Collective (pami_context_t context, pami_xfer_t *cmd);

#define PAMI_BYTE NULL

  /**
   * \brief Create a new type for noncontiguous transfers
   *
   * \todo provide example code
   *
   * \param[out] type Type identifier to be created
   */
  pami_result_t PAMI_Type_create (pami_type_t * type);

  /**
   * \brief Append a simple contiguous buffer to an existing type identifier
   *
   * \todo doxygen for offset parameter
   * \todo provide example code
   *
   * \param[in,out] type   Type identifier to be modified
   * \param[in]     bytes  Number of contiguous bytes to append
   * \param[in]     offset Offset from the end of the type to place the buffer
   * \param[in]     count  Number of buffers
   * \param[in]     stride Data stride
   */
  pami_result_t PAMI_Type_add_simple (pami_type_t type,
                                    size_t     bytes,
                                    size_t     offset,
                                    size_t     count,
                                    size_t     stride);

  /**
   * \brief Append a typed buffer to an existing type identifier
   *
   * \todo doxygen for offset parameter
   * \todo provide example code
   *
   * \warning It is considered \b illegal to append an imcomplete type to
   *          another type.
   *
   * \param[in,out] type    Type identifier to be modified
   * \param[in]     subtype Subtype to append
   * \param[in]     offset  Offset from the end of the type to place the buffer
   * \param[in]     count   Number of buffers
   * \param[in]     stride  Data stride
   */
  pami_result_t PAMI_Type_add_typed (pami_type_t type,
                                   pami_type_t subtype,
                                   size_t     offset,
                                   size_t     count,
                                   size_t     stride);

  /**
   * \brief Complete the type identifier
   *
   * \warning It is considered \b illegal to modify a type identifier after it
   *          has been completed.
   *
   * \param[in] type Type identifier to be completed
   */
  pami_result_t PAMI_Type_complete (pami_type_t type);

  /**
   * \brief Get the byte size of a completed type
   *
   * \param[in] type Type identifier to get size from
   */
  pami_result_t PAMI_Type_sizeof (pami_type_t type);

  /**
   * \brief Destroy the type
   *
   * Q. What if some in-flight messages are still using it?  What if some
   * other types have references to it?
   *
   * A. Maintain an internal reference count and release internal type
   * resources when the count hits zero.
   *
   * \param[in] type Type identifier to be destroyed
   */
  pami_result_t PAMI_Type_destroy (pami_type_t type);

  /**
   * \brief Pack data from a non-contiguous buffer to a contiguous buffer
   *
   * \param[in] src_type   source data type
   * \param[in] src_offset starting offset of source data type
   * \param[in] src_addr   starting address of source buffer
   * \param[in] dst_addr   starting address of destination buffer
   * \param[in] dst_size   destination buffer size
   */
  pami_result_t PAMI_Type_pack_data (pami_type_t src_type,
                                   size_t     src_offset,
                                   void     * src_addr,
                                   void     * dst_addr,
                                   size_t     dst_size);

  /**
   * \brief Unpack data from a contiguous buffer to a non-contiguous buffer
   *
   * \param[in] dst_type   destination data type
   * \param[in] dst_offset starting offset of destination data type
   * \param[in] dst_addr   starting address of destination buffer
   * \param[in] src_addr   starting address of source buffer
   * \param[in] src_size   source buffer size
   */
  pami_result_t PAMI_Type_unpack_data (pami_type_t dst_type,
                                     size_t     dst_offset,
                                     void     * dst_addr,
                                     void     * src_addr,
                                     size_t     src_size);
  /** \} */ /* end of "datatype" group */

#include "pami_ext.h"


  typedef union
  {
    pami_dispatch_p2p_fn         p2p;
    pami_dispatch_ambroadcast_fn ambroadcast;
    pami_dispatch_amscatter_fn   amscatter;
    pami_dispatch_amreduce_fn    amreduce;
    PAMI_DISPATCH_EXTEND         /** Extensions to this structure included from xm_ext.h */
  } pami_dispatch_callback_fn;

  /*****************************************************************************/
  /**
   * \defgroup dispatch pami dispatch interface
   *
   * Some brief documentation on dispatch stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief PAMI type of dispatch
   */
  typedef enum
  {
    PAMI_P2P_SEND,               /**< Point-to-point send         */
    PAMI_DISPATCH_TYPE_EXTEND    /** Extensions to this structure included from xm_ext.h */
  } pami_dispatch_type_t;

  /**
   * \brief Hints for dispatch
   *
   */
  typedef struct
  {
    pami_dispatch_type_t    type;      /**< Type of dispatch reqistered    */
    union{
      pami_send_hint_t      send;
      PAMI_HINT_EXTEND                 /** Extensions to this structure included from xm_ext.h */
    }                      hint;      /**< Type-specific hints            */
    void*                  config;    /**< Type-specific additional config*/
  } pami_dispatch_hint_t;

  /**
   * \brief Initialize the dispatch functions for a dispatch id.
   *
   * This is a local, non-collective operation. There is no communication
   * between tasks.
   *
   * \note The maximum allowed dispatch id attribute, \c PAMI_DISPATCH_ID_MAX,
   *       can be queried with the configuration interface
   *
   * \see PAMI_Configuration_query
   *
   * \param[in] context    PAMI communication context
   * \param[in] dispatch   Dispatch identifier to initialize
   * \param[in] fn         Dispatch receive function
   * \param[in] cookie     Dispatch function cookie
   * \param[in] options    Dispatch registration assertions
   *
   */
  //#ifdef __pami_target_mpi__
  pami_result_t PAMI_Dispatch_set_new(pami_context_t              context,
                                 size_t                     dispatch,
                                 pami_dispatch_callback_fn   fn,
                                 void                     * cookie,
                                 pami_dispatch_hint_t        options);
  //#endif
  pami_result_t PAMI_Dispatch_set (pami_context_t              context,
                                 size_t                     dispatch,
                                 pami_dispatch_callback_fn   fn,
                                 void                     * cookie,
                                 pami_send_hint_t            options);

  /**
   * \brief Initialize the dispatch functions for a dispatch id.
   *
   * This is a local, non-collective operation. There is no communication
   * between tasks.
   *
   * \param[in] context    PAMI communication context
   * \param[in] algorithm  The AM collective to set the dispatch
   * \param[in] dispatch   Dispatch identifier to initialize
   * \param[in] fn         Dispatch receive function
   * \param[in] cookie     Dispatch function cookie
   * \param[in] options    Dispatch registration assertions
   *
   */
  pami_result_t PAMI_AMCollective_dispatch_set(pami_context_t              context,
                                             pami_algorithm_t            algorithm,
                                             size_t                     dispatch,
                                             pami_dispatch_callback_fn   fn,
                                             void                     * cookie,
                                             pami_collective_hint_t      options);
  /** \} */ /* end of "dispatch" group */

  /*****************************************************************************/
  /**
   * \defgroup configuration pami configuration interface
   *
   * Some brief documentation on configuration stuff ...
   * \{
   */
  /*****************************************************************************/

  typedef char* pami_user_key_t;   /**< Char string as configuration key */
  typedef char* pami_user_value_t; /**< Char string as configuration value */
  typedef struct
  {
    pami_user_key_t    key;   /**< The configuration key   */
    pami_user_value_t  value; /**< The configuration value */
  } pami_user_config_t;

  /**
   * This enum contains ALL possible attributes for all hardware
   */
  typedef enum {
    /* Attribute            Query / Update                                 */
    PAMI_TASK_ID,            /**< Q : size_t : ID of this task (AKA "rank") */
    PAMI_NUM_TASKS,          /**< Q : size_t : Total number of tasks        */
    PAMI_NUM_CONTEXTS,       /**< Q : size_t : The maximum number of contexts allowed on this process */
    PAMI_CONST_CONTEXTS,     /**< Q : size_t : All processes will return the same PAMI_NUM_CONTEXTS */
    PAMI_HWTHREADS_AVAILABLE,/**< Q : size_t : The number of HW threads available to a process without over-subscribing (at least 1) */
    PAMI_CLOCK_MHZ,          /**< Q : size_t : Frequency of the CORE clock, in units of 10^6/seconds.  This can be used to approximate the performance of the current task. */
    PAMI_WTIMEBASE_MHZ,      /**< Q : size_t : Frequency of the WTIMEBASE clock, in units of 10^6/seconds.  This can be used to convert from PAMI_Wtimebase to PAMI_Timer manually. */
    PAMI_WTICK,              /**< Q : double : This has the same definition as MPI_Wtick(). */
    PAMI_MEM_SIZE,           /**< Q : size_t : Size of the core main memory, in units of 1024^2 Bytes    */
    PAMI_SEND_IMMEDIATE_MAX, /**< Q : size_t : Maximum number of bytes that can be transfered with the PAMI_Send_immediate() function. */
    PAMI_RECV_IMMEDIATE_MAX, /**< Q : size_t : Maximum number of bytes that can be received, and provided to the application, in a dispatch function. */
    PAMI_PROCESSOR_NAME,     /**< Q : char[] : A unique name string for the calling process, and should be suitable for use by
                                              MPI_Get_processor_name(). The storage should *not* be freed by the caller. */
    PAMI_DISPATCH_ID_MAX,    /**< Q : size_t : Maximum allowed dispatch id, see PAMI_Dispatch_set() */
  } pami_attribute_name_t;

  typedef union
  {
    size_t      intval;
    double      doubleval;
    const char* chararray;
  } pami_attribute_value_t;

#define PAMI_EXT_ATTR 1000 /**< starting value for extended attributes */

  /**
   * \brief General purpose configuration structure.
   */
  typedef struct
  {
    pami_attribute_name_t  name;  /**< Attribute type */
    pami_attribute_value_t value; /**< Attribute value */
  } pami_configuration_t;


  /**
   * \brief Query the value of an attribute
   *
   * \param [in]     client        The PAMI client
   * \param [in,out] configuration  The configuration attribute of interest
   *
   * \note
   *
   * \retval PAMI_SUCCESS  The query has completed successfully.
   * \retval PAMI_INVAL    The query has failed due to invalid parameters.
   */
  pami_result_t PAMI_Configuration_query (pami_client_t client,
                                        pami_configuration_t * configuration);

  /**
   * \brief Update the value of an attribute
   *
   * \param [in] client       The PAMI client
   * \param [in] configuration The configuration attribute to update
   *
   * \note
   *
   * \retval PAMI_SUCCESS  The update has completed successfully.
   * \retval PAMI_INVAL    The update has failed due to invalid parameters.
   *                       For example, trying to update a read-only attribute.
   */
  pami_result_t PAMI_Configuration_update (pami_client_t client,
                                         pami_configuration_t * configuration);

  /**
   * \brief Provides the detailed description of the most recent pami result.
   *
   * The "most recent pami result" is specific to each thread.
   *
   * \note  PAMI implementations may provide translated (i18n) text.
   *
   * \param[in] string Character array to write the descriptive text
   * \param[in] length Length of the character array
   *
   * \return Number of characters written into the array
   */
  size_t PAMI_Error_text (char * string, size_t length);


  /** \} */ /* end of "configuration" group */


  /*****************************************************************************/
  /**
   * \defgroup Time Timer functions required by MPI
   *
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief  Returns an elapsed time on the calling processor.
   * \note   This has the same definition as MPI_Wtime
   * \return Time in seconds since an arbitrary time in the past.
   */
  double PAMI_Wtime();

  /**
   * \brief  Returns the number of "cycles" elapsed on the calling processor.
   * \return Number of "cycles" since an arbitrary time in the past.
   *
   * "Cycles" could be any quickly and continuously increasing counter
   * if true cycles are unavailable.
   */
  unsigned long long PAMI_Wtimebase();

  /** \} */ /* end of "Time" group */


  /*****************************************************************************/
  /**
   * \defgroup contexts_and_endpoints multi-context messaging interface
   *
   * Some brief documentation on context stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Initialize the PAMI runtime for a client program
   *
   * An PAMI client represents a collection of resources to enable network
   * communications. Each PAMI client that is initialized is unque and does not
   * directly communicate with other clients. This allows middleware to be
   * developed independently and each middleware can be used concurrently by an
   * application. Resources are allocated and assigned at client creation time.
   *
   * An PAMI client \em program is any software that invokes an PAMI function.
   * This includes applications, libraries, and other middleware. Some example
   * client names may include: "MPI", "UPC", "OpenSHMEM", and "ARMCI"
   *
   * A communication context must be created before any data transfer functions
   * may be invoked.
   *
   * \see PAMI_Context_createv
   *
   * \param[in]  name   PAMI client unique name
   * \param[out] client Opaque client object
   *
   * \retval PAMI_SUCCESS  The client has been successfully created.
   * \retval PAMI_INVAL    The client name has been rejected by the runtime.
   *                       It happens when a job scheduler requires the client
   *                       name to match what's in the job description.
   */
  pami_result_t PAMI_Client_create (const char   * name,
                                      pami_client_t * client);

  /**
   * \brief Finalize the PAMI runtime for a client program
   *
   * \warning It is \b illegal to invoke any PAMI functions using the client
   *          handle from any thread after the finalize function.
   *
   * \param[in] client PAMI client handle
   *
   * \retval PAMI_SUCCESS  The client has been successfully destroyed.
   * \retval PAMI_INVAL    The client is invalid, e.g. already destroyed.
   */
  pami_result_t PAMI_Client_destroy (pami_client_t client);

  /**
   * \brief Construct an endpoint to address communication destinations
   *
   * Endpoints are opaque objects that are used to address a destination
   * in a client and are constructed from a client, task, and context offset.
   * - The client is required to disambiguate the task and context offset
   *   identifiers, as these identifiers may be the same for multiple clients
   * - The task is required to construct an endpoint to address the specific
   *   process that contains the destination context
   * - The context offset is required to identify the specific context on the
   *   destination task. Recall that a context identifies a specific threading
   *   point on a task. The context offset identifies which threading point
   *   will process the communication operation.
   *
   * Point-to-point communication operations, such as send, put, and get, will
   * address a destination with the opaque endpoint object. Collective
   * communication operations are addressed by an opaque geometry object.
   *
   * The application may choose to write an endpoint table in shared memory to
   * save storage in an environment where multiple tasks of a client have
   * access to the same shared memory area  It is the responsibility of the
   * application to allocate this shared memory area and coordinate the
   * initialization and access of any shared data structures. This includes
   * any opaque endpoint objects which may be created by one task and read by
   * another task.
   *
   * \internal The endpoint opaque object should not contain any pointers to
   *           the local address space of a particular process, as doing so will
   *           prevent the application from placing an array of endpoints in a
   *           shared memory area to be used, read-only, by all tasks with
   *           access to the shared memory area.
   *
   * \note This function may be replaced with a generated macro specific to the
   *       install platform if needed for performance reasons.
   *
   * \param[in]  client   Opaque destination client object
   * \param[in]  task     Opaque destination task object
   * \param[in]  offset   Destination context offset
   * \param[out] endpoint Opaque endpoint object
   */
  pami_result_t PAMI_Endpoint_create (pami_client_t     client,
                                      pami_task_t       task,
                                      size_t            offset,
                                      pami_endpoint_t * endpoint);

  /**
   * \example endpoint_table.c
   * This example demonstrates how to construct an endpoint table containing
   * all endpoints in the system. The endpoint table can then be used, in
   * concert with a wrapper function, as a way to address all endpoints as
   * monotonically increasing integers.
   *
   * \todo Move the doxygen comment block somewhere else
   */

  /**
   * \brief Construct an endpoint for each context in a task
   *
   * This function may be used to initialize an array of all endpoints for a
   * client by looping over all tasks.
   *
   * \param[in]     client    Opaque destination client object
   * \param[in]     task      Opaque destination task object
   * \param[in,out] endpoints Array of opaque endpoint objects to initialize
   * \param[in,out] count     Length of the endpoint array available/initialized
   *
   * \retval PAMI_INVAL The number endpoints on the task is larger than the
   *                   size of endpoint array
   */
  pami_result_t PAMI_Endpoint_createv (pami_client_t     client,
                                       pami_task_t       task,
                                       pami_endpoint_t * endpoints,
                                       size_t          * count);

  /**
   * \brief Retrieve the client, task, and context offset associated with an endpoint
   *
   * The endpoint must have been previously initialized with the
   * PAMI_Endpoint_create or PAMI_Endpoint_createv function.
   *
   * \note This function may be replaced with a generated macro specific to the
   *       install platform if needed for performance reasons.
   *
   * \param[in]  endpoint Opaque endpoint object
   * \param[out] task     Opaque destination task object
   * \param[out] offset   Destination context offset
   */
  pami_result_t PAMI_Endpoint_query (pami_endpoint_t   endpoint,
                                     pami_task_t     * task,
                                     size_t          * offset);



  /**
   * \brief Create new independent communication contexts for a client
   *
   * Contexts are local "threading points" that an application may use to
   * optimize concurrent communcation operations. A context handle is an
   * opaque object type that the application must not directly read or write
   * the value of the object.
   *
   * Communication contexts have these features:
   * - Each context is a partition of the local resources assigned to the
   *   client object for each task
   * - Every context within a client has equivalent functionality and
   *   semantics
   * - Communcation operations initiated by the local task will use the
   *   opaque context object to identify the specific threading point that
   *   will be used to issue the communication independent of communication
   *   occuring in other contexts
   * - All local event callbacks(s) associated with a communication operation
   *   will be invoked by the thread which advances the context that was used
   *   to initiate the operation
   * - A context is a local object and is not used to directly address a
   *   communication destination
   * - Progress is driven independently among contexts
   * - Progress may be driven concurrently among contexts, by using multiple
   *   threads, as desired by the application
   * - <b>All contexts created by a client must be advanced by the application
   *   to prevent deadlocks</b>
   *
   * \note The progress rule may be relaxed in future versions of the interface
   *
   * \par Thread considerations
   *       Applications map, or "apply", threading resources to contexts.
   *       Operations on contexts are critical sections and not thread-safe.
   *       The application must ensure that critical sections are protected
   *       from re-entrant use. PAMI provides mechanisms for controlling access
   *       to critical sections
   *
   * The context configuration attributes may include:
   * - Context optimizations, such as shared memory, collective acceleration, etc
   *
   * Context creation is a local operation and does not involve communication or
   * syncronization with other tasks.
   *
   * \param[in]  client        Client handle
   * \param[in]  configuration List of configurable attributes and values
   * \param[in]  count         Number of configurations, may be zero
   * \param[out] context       Array of communication contexts to initialize
   * \param[in]  ncontexts     num contexts requested (in), created (out)
   *
   * \retval PAMI_SUCCESS  Contexts have been created. The number of contexts
   *                       created could be less the number requested.
   * \retval PAMI_INVAL    Configuration could not be satisified or there were
   *                       errors in other parameters.
   */
  pami_result_t PAMI_Context_createv (pami_client_t          client,
                                    pami_configuration_t   configuration[],
                                    size_t                count,
                                    pami_context_t       * context,
                                    size_t                ncontexts);


  /**
   * \brief Destroy the communication context
   *
   * \warning It is \b illegal to invoke any PAMI functions using the
   *          communication context from any thread after the context is
   *          destroyed.
   *
   * \param[in,out] contexts  PAMI communication context list
   * \param[in]     ncontexts The number of contexts in the list.
   *
   * \retval PAMI_SUCCESS  The contexts have been destroyed.
   * \retval PAMI_INVAL    Some context is invalid, e.g. already destroyed.
   */
  pami_result_t PAMI_Context_destroyv (pami_context_t* contexts,
                                       size_t          ncontexts);

  /**
   * \brief Post work to a context, thread-safe
   *
   * It is \b not required that the target context is locked, or otherwise
   * reserved, by an external atomic operation to ensure thread safety. The PAMI
   * runtime will internally perform any neccessary atomic operations in order
   * to post the work to the context.
   *
   * The callback function will be invoked in the thread that advances the
   * \em work context. There is no implicit completion notification provided
   * to the \em posting thread when the thread advancing the \em work context
   * returns from the callback event function.  If the posting thread desires
   * a completion notification it must explicitly program such notifications,
   * via the PAMI_Context_post() interface, from the target thread back to the
   * origin thread
   *
   * \todo Needs some opaque storage to enqueue on to the work queue. This is
   *       neccesary to improve the performance for the MMPS benchmark. In
   *       other words, latency may degrade if the internal implementation
   *       must allocate memory.
   *
   * \param[in] context PAMI communication context
   * \param[in] work_fn Event callback function to post to the context
   * \param[in] cookie  Opaque data pointer to pass to the event function
   *
   * \retval PAMI_SUCCESS  The work has been posted.
   * \retval PAMI_INVAL    There were errors in the parameters.
   */
  pami_result_t PAMI_Context_post (pami_context_t        context,
                                 pami_work_t         * work,
                                 pami_work_function    fn,
                                 void               * cookie);


  /**
   * \brief Advance the progress engine for a single communication context
   *
   * May complete zero, one, or more outbound transfers. May invoke dispatch
   * handlers for incoming transfers. May invoke work event callbacks previously
   * posted to the communication context.
   *
   * This polling advance function will return after the first poll iteration
   * that results in a processed event or if, no events are processed, after
   * polling for the maximum number of iterations.
   *
   * \warning This function is \b not \b threadsafe and the application must
   *          ensure that only one thread advances a context at any time.
   *
   * \see PAMI_Context_lock
   * \see PAMI_Context_trylock
   *
   * \todo Define return code, event bitmask ?
   *
   * \param[in] context PAMI communication context
   * \param[in] maximum Maximum number of internal poll iterations
   *
   * \retval PAMI_SUCCESS  An event has occurred and been processed.
   * \retval PAMI_EAGAIN   No event has occurred.
   */
  pami_result_t PAMI_Context_advance (pami_context_t context, size_t maximum);

  /**
   * \brief Advance the progress engine for multiple communication contexts
   *
   * May complete zero, one, or more outbound transfers. May invoke dispatch
   * handlers for incoming transfers. May invoke work event callbacks previously
   * posted to a communication context.
   *
   * This polling advance function will return after the first poll iteration
   * that results in a processed event on any context, or if, no events are
   * processed, after polling for the maximum number of iterations.
   *
   * \warning This function is \b not \b threadsafe and the application must
   *          ensure that only one thread advances the contexts at any time.
   *
   * \note It is possible to define a set of communication contexts that are
   *       always advanced together by any pami client thread.  It is the
   *       responsibility of the pami client to atomically lock the context set,
   *       perhaps by using the PAMI_Context_lock() function on a designated
   *       \em leader context, and to manage the pami client threads to ensure
   *       that only one thread ever advances the set of contexts.
   *
   * \todo Define return code, event bitmask ?
   * \todo Rename function to something better
   *
   * \see PAMI_Context_lock
   * \see PAMI_Context_trylock
   *
   * \param[in] context Array of PAMI communication contexts
   * \param[in] count   Number of communication contexts
   * \param[in] maximum Maximum number of internal poll iterations on each context
   * \retval PAMI_SUCCESS  An event has occurred and been processed.
   * \retval PAMI_EAGAIN   No event has occurred.
   */
  pami_result_t PAMI_Context_advancev (pami_context_t context[],
                                         size_t        count,
                                         size_t        maximum);

  /**
   * \brief Acquire an atomic lock on a communication context
   *
   * \warning This function will block until the lock is aquired.
   *
   * \param[in] context PAMI communication context
   *
   * \retval PAMI_SUCCESS  The lock has been acquired.
   * \retval PAMI_INVAL    The context is invalid.
   */
  pami_result_t PAMI_Context_lock (pami_context_t context);

  /**
   * \brief Attempt to acquire an atomic lock on a communication context
   *
   * \param[in] context PAMI communication context
   *
   * \retval PAMI_SUCCESS  The lock has been acquired.
   * \retval PAMI_EAGAIN   The lock has not been acquired. Try again later.
   * \retval PAMI_INVAL    The context is invalid.
   */
  pami_result_t PAMI_Context_trylock (pami_context_t context);

  /**
   * \brief Release an atomic lock on a communication context
   *
   * \param[in] context PAMI communication context
   *
   * \retval PAMI_SUCCESS  The lock has been released.
   * \retval PAMI_INVAL    The context is invalid.
   */
  pami_result_t PAMI_Context_unlock (pami_context_t context);

  /** \} */ /* end of "context" group */

#ifdef __cplusplus
};
#endif

#endif /* __pami__h__ */
