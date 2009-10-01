#ifndef __xmi__h__
#define __xmi__h__

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "xmi_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * \brief XMI result status codes.
   */
  typedef enum
  {
    XMI_SUCCESS = 0,  /**< Successful execution        */
    XMI_NERROR  = -1, /**< Generic error (-1)          */
    XMI_ERROR   = 1,  /**< Generic error (+1)          */
    XMI_INVAL,        /**< Invalid argument            */
    XMI_UNIMPL,       /**< Function is not implemented */
    XMI_EAGAIN,       /**< Not currently availible     */
    XMI_SHUTDOWN,     /**< Rank has shutdown           */
    XMI_CHECK_ERRNO,  /**< Check the errno val         */
    XMI_OTHER,        /**< Other undefined error       */
  }
    xmi_result_t;

  typedef void* xmi_client_t;   /**< ??? */
  typedef void* xmi_context_t;  /**< ??? */
  typedef void* xmi_hint_t;     /**< ??? */
  typedef void* xmi_type_t;     /**< ??? */

  /**
   * \brief Callback to handle message events
   *
   * \param[in] context   XMI communication context that invoked the callback
   * \param[in] cookie    Event callback application argument
   * \param[in] result    Asynchronous result information (was error information)
   */
  typedef void (*xmi_event_function) ( xmi_context_t   context,
                                       void          * cookie,
                                       xmi_result_t    result );

  typedef struct xmi_callback_t
  {
    xmi_event_function  function;
    void               *clientdata;
  }xmi_callback_t;




/**
 * \brief Message layer operation types
 */

  typedef enum
  {
    XMI_UNDEFINED_OP = 0,
    XMI_NOOP,
    XMI_MAX,
    XMI_MIN,
    XMI_SUM,
    XMI_PROD,
    XMI_LAND,
    XMI_LOR,
    XMI_LXOR,
    XMI_BAND,
    XMI_BOR,
    XMI_BXOR,
    XMI_MAXLOC,
    XMI_MINLOC,
    XMI_USERDEFINED_OP,
    XMI_OP_COUNT
  }
    xmi_op;

  /**
   * \brief Message layer data types
   */

  typedef enum
  {
    /* Standard/Primative DT's */
    XMI_UNDEFINED_DT = 0,
    XMI_SIGNED_CHAR,
    XMI_UNSIGNED_CHAR,
    XMI_SIGNED_SHORT,
    XMI_UNSIGNED_SHORT,
    XMI_SIGNED_INT,
    XMI_UNSIGNED_INT,
    XMI_SIGNED_LONG_LONG,
    XMI_UNSIGNED_LONG_LONG,
    XMI_FLOAT,
    XMI_DOUBLE,
    XMI_LONG_DOUBLE,
    XMI_LOGICAL,
    XMI_SINGLE_COMPLEX,
    XMI_DOUBLE_COMPLEX,
    /* Max/Minloc DT's */
    XMI_LOC_2INT,
    XMI_LOC_SHORT_INT,
    XMI_LOC_FLOAT_INT,
    XMI_LOC_DOUBLE_INT,
    XMI_LOC_2FLOAT,
    XMI_LOC_2DOUBLE,
    XMI_USERDEFINED_DT,
    XMI_DT_COUNT
  }
    xmi_dt;

  typedef void*   xmi_geometry_t;
  typedef size_t  xmi_algorithm_t;
  
  typedef struct
  {
    size_t lo;
    size_t hi;
  }
    xmi_geometry_range_t;
  

  /**
   * \brief Network type
   */
  typedef enum
  {
    XMI_DEFAULT_NETWORK = 0, /**< Default network type. \b Guaranteed to work. */
    XMI_TORUS_NETWORK,       /**< DEPRECATED! 3D-Torus / 1D-SMP network type. */
    XMI_N_TORUS_NETWORK,     /**< nD-Torus / nD-SMP network type.
                              * mapping->torusDims() for torus dim,
                              * mapping->globalDims() for all (torus+SMP) dim.
                              */
    XMI_SOCKET_NETWORK,      /**< Unix socket network type. */
    XMI_SHMEM_NETWORK,       /**< local shared memory "network" for smp nodes. */
    XMI_NETWORK_COUNT        /**< Number of network types defined. */
  }
    xmi_network;

/// \todo Remove this platform-specific #define
#define XMI_MAX_DIMS 7
//#define XMI_MAX_DIMS	4

  /**
   * \brief A structure to describe a network coordinate
   */
  typedef struct xmi_coord_t
  {
    xmi_network network; /**< Network type for the coordinates */
    union
    {
      struct
      {
        size_t x; /**< Torus network x coordinate */
        size_t y; /**< Torus network y coordinate */
        size_t z; /**< Torus network z coordinate */
        size_t t; /**< Torus network t coordinate */
      } torus;    /**< obsolete: XMI_TORUS_NETWORK coordinates */
      struct
      {
        size_t coords[XMI_MAX_DIMS];
      } n_torus;
      struct
      {
        int recv_fd;   /**< Receive file descriptor */
        int send_fd;   /**< Send file descriptor    */
      } socket;   /**< XMI_SOCKET_NETWORK coordinates */
      struct
      {
        size_t rank;   /**< Global rank of process */
        size_t peer;   /**< Local rank of process */
      } shmem;    /**< XMI_SHMEM_NETWORK coordinates */
    };
  } xmi_coord_t;

  /**
   * \brief A metadata structure to describe a collective protocol
   */
  typedef struct
  {
//  \todo: attribute definition
    //xmi_ca_list_t geometry_attrs; /**< geometry attributes */
    //xmi_ca_list_t buffer_attrs;  /**< buffer attributes (contig, alignment) */
    //xmi_ca_list_t misc_attributes; /**< other attributes (i.e. threaded) */
    char name[32]; /** < name of algorithm */
  } xmi_metadata_t;

  /*****************************************************************************/
  /**
   * \defgroup activemessage xmi active messaging interface
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
    uint32_t sync_send         : 1; /**< Assert that all sends will be synchronously received    */
    uint32_t buffer_registered : 1; /**< ???                                                     */
    uint32_t use_rdma          : 1; /**< Assert/enable rdma operations                           */
    uint32_t no_rdma           : 1; /**< Disable rdma operations                                 */
    uint32_t no_local_copy     : 1; /**< ???                                                     */
    uint32_t interrupt_on_recv : 1; /**< Interrupt the remote task when the first packet arrives */
    uint32_t high_priority     : 1; /**< Message is delivered with high priority,
                                       which may result in out-of-order delivery               */
    uint32_t reserved          :24; /**< Unused at this time                                     */
  } xmi_send_hint_t;

  /**
   * \brief Active message send common parameters structure
   */
  typedef struct
  {
    size_t            dispatch; /**< Dispatch identifier */
    xmi_send_hint_t   hints;    /**< Hints for sending the message */
    size_t            task;     /**< Destination task */
    void            * cookie;   /**< Argument to \b all event callbacks */
    struct
    {
      size_t          bytes;    /**< Header buffer size in bytes */
      void          * addr;     /**< Header buffer address */
    } header;                        /**< Send message metadata header */
  } xmi_send_t;

  /**
   * \brief Active message kind identifier
   */
  typedef enum {
    XMI_AM_KIND_SIMPLE = 0, /**< Simple contiguous data transfer */
    XMI_AM_KIND_TYPED       /**< Typed, non-contiguous, data transfer */
  } xmi_am_kind_t;

  /**
   * \brief Structure for send parameters unique to a simple active message send
   */
  typedef struct
  {
    xmi_send_t             send;     /**< Common send parameters */
    struct
    {
      size_t               bytes;    /**< Number of bytes of data */
      void               * addr;     /**< Address of the buffer */
      xmi_event_function   local_fn; /**< Local message completion event */
      xmi_event_function   remote_fn;/**< Remote message completion event ------ why is this needed ? */
    } simple;                        /**< Simple send parameters */
  } xmi_send_simple_t;

  /**
   * \brief Structure for send parameters unique to an immediate active message send
   */
  typedef struct
  {
    xmi_send_t               send;     /**< Common send parameters */
    struct
    {
      size_t                 bytes;    /**< Number of bytes of data */
      void                 * addr;     /**< Address of the buffer */
    } immediate;                       /**< Immediate send parameters */
  } xmi_send_immediate_t;

  /**
   * \brief Structure for send parameters unique to a typed active message send
   */
  typedef struct
  {
    xmi_send_t             send;     /**< Common send parameters */
    struct
    {
      size_t               bytes;    /**< Number of bytes of data */
      void               * addr;     /**< Starting address of the buffer */
      size_t               offset;   /**< Starting offset */
      xmi_type_t           type;     /**< Datatype */
      xmi_event_function   local_fn; /**< Local message completion event */
      xmi_event_function   remote_fn;/**< Remote message completion event ------ why is this needed ? */
    } typed;                         /**< Typed send parameters */
  } xmi_send_typed_t;

  /**
   * \brief Non-blocking active message send for contiguous data
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Send simple parameter structure
   */
  xmi_result_t XMI_Send (xmi_context_t       context,
                         xmi_send_simple_t * parameters);

  /**
   * \brief Immediate active message send for small contiguous data
   *
   * The blocking send is only valid for small data buffers. The implementation
   * configuration attribute \c IMMEDIATE_SEND_LIMIT defines the upper
   * bounds for the size of data buffers, including header data, that can be
   * sent with this function. This function will return an error if a data
   * buffer larger than the \c IMMEDIATE_SEND_LIMIT is attempted.
   *
   * This function provides a low-latency send that can be optimized by the
   * specific xmi implementation. If network resources are immediately
   * available the send data will be injected directly into the network. If
   * resources are not available the specific xmi implementation may internally
   * buffer the send parameters and data until network resource are available
   * to complete the transfer. In either case the send will immediately return,
   * no doce callback is invoked, and is considered complete.
   *
   * The low-latency send operation may be further enhanced by using a
   * specially configured dispatch id which asserts that all dispatch receive
   * callbacks will not exceed a certain limit. The implementation
   * configuration attribute \c SYNC_SEND_LIMIT defines the upper bounds for
   * the size of data buffers that can be completely received with a single
   * dispatch callback. Typically this limit is associated with a network
   * resource attribute, such as a packet size.
   *
   * \see XMI_Configuration_query
   *
   * \todo Better define send parameter structure so done callback is not required
   * \todo Define configuration attribute for the size limit
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Send immediate parameter structure
   */
  xmi_result_t XMI_Send_immediate (xmi_context_t          context,
                                   xmi_send_immediate_t * parameters);

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
   * \param[in] context    XMI communication context
   * \param[in] parameters Send typed parameter structure
   */
  xmi_result_t XMI_Send_typed (xmi_context_t      context,
                               xmi_send_typed_t * parameters);

  /**
   * \brief Active message receive hints
   */
  typedef struct
  {
    uint32_t inline_completion :  1; /**< The receive completion callback
                                      *   \b must be invoked by the thread that
                                      *   receives the dispatch notification. */
    uint32_t reserved          : 31; /**< Reserved. Do not use. */
  } xmi_recv_hint_t;

  /**
   * \brief Receive message structure
   *
   * This structure is initialized and then returned as an output parameter from
   * the active message dispatch callback to direct the xmi runtime how to
   * receive the data stream.
   *
   * \see xmi_dispatch_p2p_fn
   */
  typedef struct
  {
    xmi_recv_hint_t         hints;    /**< Hints for receiving the message */
    void                  * cookie;   /**< Argument to \b all event callbacks */
    xmi_event_function      local_fn; /**< Local message completion event */
    xmi_am_kind_t           kind;     /**< Which kind receive is to be done */
    union
    {
      struct
      {
        size_t              bytes;    /**< Number of bytes of data */
        void              * addr;     /**< Starting address of the buffer */
      } simple;                       /**< Contiguous buffer receive */
      struct
      {
        size_t              bytes;    /**< Number of bytes of data */
        void              * addr;     /**< Starting address of the buffer */
        xmi_type_t          type;     /**< Datatype */
        size_t              offset;   /**< Starting offset of the type */
      } typed;                        /**< Typed receive */
    } data;                           /**< Receive message destination information */
  } xmi_recv_t;

  /**
   * \brief Dispatch callback
   *
   * "pipe" has nothing to do with "PipeWorkQueue"s
   */
  typedef void (*xmi_dispatch_p2p_fn) (
    xmi_context_t        context,      /**< IN:  XMI context     */
    void               * cookie,       /**< IN:  dispatch cookie */
    size_t               task,         /**< IN:  source task     */
    void               * header_addr,  /**< IN:  header address  */
    size_t               header_size,  /**< IN:  header size     */
    void               * pipe_addr,    /**< IN:  address of XMI pipe  buffer, valid only if non-NULL        */
    size_t               pipe_size,    /**< IN:  size of XMI pipe buffer, valid regarldless of message type */
    xmi_recv_t         * recv);        /**< OUT: receive message structure, only needed if addr is non-NULL */

  /** \} */ /* end of "active message" group */


  /*****************************************************************************/
  /**
   * \defgroup rma xmi remote memory access data transfer interface
   *
   * Some brief documentation on rma stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Common input parameters for all XMI rma functions
   **/
  typedef struct
  {
    size_t               task;      /**< Destination task */
    xmi_send_hint_t      hints;     /**< Hints for sending the message */
    void               * local;     /**< Local transfer virtual address */
    void               * remote;    /**< Remote transfer virtual address */
    void               * cookie;    /**< Argument to \b all event callbacks */
    xmi_event_function   done_fn;   /**< Local completion event */
  } xmi_rma_t;

  /*****************************************************************************/
  /**
   * \defgroup put xmi remote memory access put transfer interface
   *
   * Some brief documentation on put stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Input parameter structure for simple put transfers
   */
  typedef struct
  {
    xmi_rma_t            rma;       /**< Common rma parameters */
    struct
    {
      size_t             bytes;     /**< Data transfer size in bytes */
      xmi_event_function rdone_fn;  /**< Remote completion event - all local
                                       data has been received by remote task */
    } put;                          /**< Parameters specific to put */
  } xmi_put_simple_t;

  /**
   * \brief Input parameter structure for typed put transfers
   */
  typedef struct
  {
    xmi_rma_t            rma;       /**< Common rma parameters */
    struct
    {
      size_t             bytes;     /**< Data transfer size in bytes */
      xmi_event_function rdone_fn;  /**< Remote completion event - all local
                                       data has been received by remote task */
      xmi_type_t         local;     /**< Data type of local buffer */
      xmi_type_t         remote;    /**< Data type of remote buffer */
    } put;                          /**< Parameters specific to put */
  } xmi_put_typed_t;

  /**
   * \brief One-sided put operation for simple contiguous data transfer
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Simple put input parameters
   */
  xmi_result_t XMI_Put (xmi_context_t      context,
                        xmi_put_simple_t * parameters);

  /**
   * \brief One-sided put operation for typed non-contiguous data transfer
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Typed put input parameters
   */
  xmi_result_t XMI_Put_typed (xmi_context_t     context,
                              xmi_put_typed_t * parameters);


  /** \} */ /* end of "put" group */

  /*****************************************************************************/
  /**
   * \defgroup get xmi remote memory access get transfer interface
   *
   * Some brief documentation on get stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Input parameter structure for simple get transfers
   */
  typedef struct
  {
    xmi_rma_t            rma;       /**< Common rma parameters */
    struct
    {
      size_t             bytes;     /**< Data transfer size in bytes */
    } get;                          /**< Parameters specific to get */
  } xmi_get_simple_t;

  /**
   * \brief Input parameter structure for typed get transfers
   */
  typedef struct
  {
    xmi_rma_t            rma;         /**< Common rma parameters */
    struct
    {
      size_t             bytes;     /**< Data transfer size in bytes */
      xmi_type_t         local;     /**< Data type of local buffer */
      xmi_type_t         remote;    /**< Data type of remote buffer */
    } get;                          /**< Parameters specific to get */
  } xmi_get_typed_t;

  /**
   * \brief One-sided get operation for simple contiguous data transfer
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Simple get input parameters
   */
  xmi_result_t XMI_Get (xmi_context_t      context,
                        xmi_get_simple_t * parameters);

  /**
   * \brief One-sided get operation for typed non-contiguous data transfer
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters Typed get input parameters
   */
  xmi_result_t XMI_Get_typed (xmi_context_t     context,
                              xmi_get_typed_t * parameters);

  /** \} */ /* end of "get" group */

  /*****************************************************************************/
  /**
   * \defgroup rmw xmi remote memory access read-modify-write interface
   *
   * Some brief documentation on rmw stuff ...
   * \{
   */
  /*****************************************************************************/

  /** \brief Atomic rmw data type */
  typedef enum
  {
    XMI_RMW_KIND_UINT32      = 0x0001, /**< 32-bit unsigned integer operation */
    XMI_RMW_KIND_UINT64      = 0x0002  /**< 64-bit unsigned integer operation */
  } xmi_rmw_kind_t;

  /** \brief Atomic rmw assignment type */
  typedef enum
  {
    XMI_RMW_ASSIGNMENT_SET   = 0x0010, /**< =  operation */
    XMI_RMW_ASSIGNMENT_ADD   = 0x0020, /**< += operation */
    XMI_RMW_ASSIGNMENT_OR    = 0x0040  /**< |= operation */
  } xmi_rmw_assignment_t;

  /** \brief Atomic rmw comparison type */
  typedef enum
  {
    XMI_RMW_COMPARISON_NOOP  = 0x0100, /**< No comparison operation */
    XMI_RMW_COMPARISON_EQUAL = 0x0200  /**< Equality comparison operation */
  } xmi_rmw_comparison_t;

  typedef struct
  {
    xmi_rma_t               rma;       /**< Common rma parameters */
    struct
    {
      xmi_rmw_comparison_t  compare;   /**< read-modify-write comparison type */
      xmi_rmw_assignment_t  assign;    /**< read-modify-write assignment type */
      xmi_rmw_kind_t        kind;      /**< read-modify-write variable type */
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
      };
    } rmw;                             /**< Parameters specific to rmw */
  } xmi_rmw_t;

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
   * \par XMI_RMW_KIND_UINT32 | XMI_RMW_COMPARISON_NOOP | XMI_RMW_ASSIGNMENT_ADD
   *      "32-bit unsigned integer fetch-and-add operation"
   * \code
   * uint32_t *result, *remote, value, test;
   * *result = *remote; *remote += value;
   * \endcode
   *
   * \par XMI_RMW_KIND_UINT32 | XMI_RMW_COMPARISON_NOOP | XMI_RMW_ASSIGNMENT_OR
   *      "32-bit unsigned integer fetch-and-or operation"
   * \code
   * uint32_t *result, *remote, value, test;
   * *result = *remote; *remote |= value;
   * \endcode
   *
   * \par XMI_RMW_KIND_UINT64 | XMI_RMW_COMPARISON_NOOP | XMI_RMW_ASSIGNMENT_SET
   *      "64-bit unsigned integer swap (fetch-and-set) operation"
   * \code
   * uint64_t *result, *remote, value, test;
   * *result = *remote; *remote = value;
   * \endcode
   *
   * \par XMI_RMW_KIND_UINT64 | XMI_RMW_COMPARISON_EQUAL | XMI_RMW_ASSIGNMENT_SET
   *      "64-bit unsigned integer compare-and-swap operation"
   * \code
   * uint64_t *result, *remote, value, test;
   * *result = *remote; (*remote == test) ? *remote = value;
   * \endcode
   *
   * \param[in] context    XMI communication context
   * \param[in] parameters read-modify-write input parameters
   */
  xmi_result_t XMI_Rmw (xmi_context_t context, xmi_rmw_t * parameters);

  /** \} */ /* end of "rmw" group */

  /*****************************************************************************/
  /**
   * \defgroup rdma xmi remote memory access rdma interfaces
   *
   * Some brief documentation on rdma stuff ...
   * \{
   */
  /*****************************************************************************/

  typedef void * xmi_memregion_t; /**< ??? */

  /**
   * \brief Default "global" memory region
   *
   * The global memory region may be used in the one-sided operations to make use
   * of the system-managed memory region support.
   *
   * User-managed memory regions may result in higher performance for one-sided
   * operations due to system memory region caching, internal memory region
   * exchange operations, and other implementation-specific management features.
   */
  extern xmi_memregion_t XMI_MEMREGION_GLOBAL;

  /**
   * \brief Register a local memory region for one sided operations
   *
   * The local memregion may be transfered, via a send message, to a remote task
   * to allow the remote task to perform one-sided operations with this local
   * task
   *
   * \todo Define, exactly, what the sematics are if a NULL pointer is passed
   *       as the memregion to initialize.
   *
   * \param[in]  context   XMI application context
   * \param[in]  address   Virtual address of memory region
   * \param[in]  bytes     Number of bytes to register
   * \param[out] memregion Memory region object. Can be NULL.
   */
  xmi_result_t XMI_Memregion_register (xmi_context_t     context,
                                       void            * address,
                                       size_t            bytes,
                                       xmi_memregion_t * memregion);

  /**
   * \brief Deregister a local memory region for one sided operations
   *
   * It is illegal to deregister the "global" memory region.
   *
   * \param[in] context   XMI application context
   * \param[in] memregion Memory region object
   */
  xmi_result_t XMI_Memregion_deregister (xmi_context_t   context,
                                         xmi_memregion_t memregion);

  /**
   * \brief Query the attributes of a memory region
   *
   * \todo Should the communication context be an output parameter?
   *
   * \param[in]  context   XMI application context
   * \param[in]  memregion Memory region object
   * \param[out] address   Base virtual address
   * \param[out] bytes     Number of contiguous bytes from the base address
   * \param[out] task      XMI task that registered the memory region
   */
  xmi_result_t XMI_Memregion_query (xmi_context_t      context,
                                    xmi_memregion_t    memregion,
                                    void            ** address,
                                    size_t           * bytes,
                                    size_t           * task);


  /**
   * \brief Input parameter structure for simple rput transfers
   */
  typedef struct
  {
    xmi_rma_t              rma;       /**< Common rma parameters */
    struct
    {
      xmi_event_function   remote_fn; /**< Remote completion event - all local
                                         data has been received by remote task */
      xmi_memregion_t      local_mr;  /**< Local buffer memory region */
      xmi_memregion_t      remote_mr; /**< Remote buffer memory region */
    } rput;                           /**< Parameters specific to rput */
  } xmi_rput_simple_t;

  /**
   * \brief Input parameter structure for typed put transfers
   */
  typedef struct
  {
    xmi_rma_t            rma;         /**< Common rma parameters */
    struct
    {
      xmi_event_function   remote_fn; /**< Remote completion event - all local
                                         data has been received by remote task */
      xmi_type_t           local;     /**< Data type of local buffer */
      xmi_type_t           remote;    /**< Data type of remote buffer */
      xmi_memregion_t      local_mr;  /**< Local buffer memory region */
      xmi_memregion_t      remote_mr; /**< Remote buffer memory region */
    } rput;                           /**< Parameters specific to rput */
  } xmi_rput_typed_t;


  /**
   * \brief Simple put operation for one-sided contiguous data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   */
  xmi_result_t XMI_Rput (xmi_context_t context, xmi_rput_simple_t * parameters);

  /**
   * \brief Put operation for data type specific one-sided data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   */
  xmi_result_t XMI_Rput_typed (xmi_context_t context, xmi_rput_typed_t * parameters);

  /**
   * \brief Input parameter structure for simple rput transfers
   */
  typedef struct
  {
    xmi_rma_t              rma;       /**< Common rma parameters */
    struct
    {
      xmi_memregion_t      local_mr;  /**< Local buffer memory region */
      xmi_memregion_t      remote_mr; /**< Remote buffer memory region */
    } rget;                           /**< Parameters specific to rget */
  } xmi_rget_simple_t;

  /**
   * \brief Input parameter structure for typed put transfers
   */
  typedef struct
  {
    xmi_rma_t            rma;         /**< Common rma parameters */
    struct
    {
      xmi_type_t           local;     /**< Data type of local buffer */
      xmi_type_t           remote;    /**< Data type of remote buffer */
      xmi_memregion_t      local_mr;  /**< Local buffer memory region */
      xmi_memregion_t      remote_mr; /**< Remote buffer memory region */
    } rget;                           /**< Parameters specific to rget */
  } xmi_rget_typed_t;

  /**
   * \brief Simple get operation for one-sided contiguous data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   */
  xmi_result_t XMI_Rget (xmi_context_t context, xmi_rget_simple_t * parameters);

  /**
   * \brief Get operation for data type specific one-sided data transfer.
   *
   * \param[in] context    XMI application context
   * \param[in] parameters Input parameters structure
   */
  xmi_result_t XMI_Rget_typed (xmi_context_t context, xmi_rget_typed_t * parameters);

  /** \} */ /* end of "rdma" group */
  /** \} */ /* end of "rma" group */



  /*****************************************************************************/
  /**
   * \defgroup dynamictasks xmi dynamic task interface
   *
   * Some brief documentation on dynamic task stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Clean up local resources to a task in preparation for
   *        task shutdown or checkpoint
   *
   *        It is the user of this API's responsibility to ensure
   *        that all communication has been quiesced to and from
   *        the destination via a fence call and synchronization
   *
   * \param[in] context    XMI communication context
   * \param[in] dest       Array of destination tasks to close connections to
   * \param[in] count      Number of tasks in the array dest
   */

  xmi_result_t XMI_Purge_totask (xmi_context_t   context,
                                 size_t        * dest,
                                 size_t          count);

  /**
   * \brief Setup local resources to a task in preparation for
   *        task restart or creation
   *
   * \param[in] context    XMI communication context
   * \param[in] dest       Array of destination tasks to resume connections to
   * \param[in] count      Number of tasks in the array dest
   */
  xmi_result_t XMI_Resume_totask (xmi_context_t   context,
                                  size_t        * dest,
                                  size_t          count);

  /** \} */ /* end of "dynamic tasks" group */


  /*****************************************************************************/
  /**
   * \defgroup sync xmi memory synchronization and data fence interface
   *
   * Some brief documentation on sync stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Begin a memory synchronization region
   *
   * A fence region is defined as an area of program control on the local task
   * bounded by the XMI_Fence_begin() and XMI_Fence_end() functions.
   *
   * \warning It is considered \b illegal to invoke a fence operation outside of
   *          a fence region.
   *
   * \warning It is considered \b illegal to begin a fence region inside an
   *          existing fence region. Fence regions can not be nested.
   *
   * \param[in] context XMI communication context
   */
  xmi_result_t XMI_Fence_begin (xmi_context_t context);

  /**
   * \brief End a memory synchronization region
   *
   * A fence region is defined as an area of program control on the local task
   * bounded by the XMI_Fence_begin() and XMI_Fence_end() functions.
   *
   * \warning It is considered \b illegal to invoke a fence operation outside of
   *          a fence region.
   *
   * \warning It is considered \b illegal to end a fence region outside of an
   *          existing fence region.
   *
   * \param[in] context XMI communication context
   */
  xmi_result_t XMI_Fence_end (xmi_context_t context);


  /**
   * \brief Syncronize all transfers between all tasks.
   *
   * \param[in] context XMI communication context
   * \param[in] done_fn Event callback to invoke when the fence is complete
   * \param[in] cookie  Event callback argument
   */
  xmi_result_t XMI_Fence_all (xmi_context_t        context,
                              xmi_event_function   done_fn,
                              void               * cookie);

  /**
   * \brief Syncronize all transfers between two tasks.
   *
   * \param[in] context XMI communication context
   * \param[in] done_fn Event callback to invoke when the fence is complete
   * \param[in] cookie  Event callback argument
   * \param[in] task    Remote task to synchronize
   */
  xmi_result_t XMI_Fence_task (xmi_context_t        context,
                               xmi_event_function   done_fn,
                               void               * cookie,
                               size_t               task);

  /** \} */ /* end of "sync" group */

/* we should probably put these in like xmi_util.h
   these values need to change later                                          */
#define XMI_BROADCAST_ALGORITHM_DEFAULT                                      0
#define XMI_ALLREDUCE_ALGORITHM_DEFAULT                                      0
#define XMI_REDUCE_ALGORITHM_DEFAULT                                         0
#define XMI_ALLGATHER_ALGORITHM_DEFAULT                                      0
#define XMI_ALLGATHERV_ALGORITHM_DEFAULT                                     0
#define XMI_ALLGATHERV_INT_ALGORITHM_DEFAULT                                 0
#define XMI_SCATTER_ALGORITHM_DEFAULT                                        0
#define XMI_SCATTERV_ALGORITHM_DEFAULT                                       0
#define XMI_SCATTERV_INT_ALGORITHM_DEFAULT                                   0
#define XMI_BARRIER_ALGORITHM_DEFAULT                                        0
#define XMI_ALLTOALL_ALGORITHM_DEFAULT                                       0
#define XMI_ALLTOALLV_ALGORITHM_DEFAULT                                      0
#define XMI_ALLTOALLV_INT_ALGORITHM_DEFAULT                                  0
#define XMI_SCAN_ALGORITHM_DEFAULT                                           0
#define XMI_AMBROADCAST_ALGORITHM_DEFAULT                                    0
#define XMI_AMSCATTER_ALGORITHM_DEFAULT                                      0
#define XMI_AMGATHER_ALGORITHM_DEFAULT                                       0
#define XMI_AMREDUCE_ALGORITHM_DEFAULT                                       0

  /**
   * \brief Macro to set a collective algorithm pointed by 'algorithm' to
   *        default protocol for a given transfer type.
   *
   * \param[in]      coll_type  Type of xmi transfer operation
   * \param[in/out]  algorithm  variable to set the default protocol to.
   */  
#define XMI_COLLECTIVE_ALGORITHM_DEFAULT(coll_type, algorithm)          \
  do                                                                    \
      {                                                                 \
        switch(coll_type)                                               \
            {                                                           \
                case XMI_XFER_BROADCAST:                                \
                  algorithm = XMI_BROADCAST_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_ALLREDUCE:                                \
                  algorithm = XMI_ALLREDUCE_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_REDUCE:                                   \
                  algorithm = XMI_REDUCE_ALGORITHM_DEFAULT;             \
                  break;                                                \
                case XMI_XFER_ALLGATHER:                                \
                  algorithm = XMI_ALLGATHER_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_ALLGATHERV:                               \
                  algorithm = XMI_ALLGATHERV_ALGORITHM_DEFAULT;         \
                  break;                                                \
                case XMI_XFER_ALLGATHERV_INT:                           \
                  algorithm = XMI_ALLGATHERV_INT_ALGORITHM_DEFAULT;     \
                  break;                                                \
                case XMI_XFER_SCATTERV:                                 \
                  algorithm = XMI_SCATTERV_ALGORITHM_DEFAULT;           \
                  break;                                                \
                case XMI_XFER_SCATTER:                                  \
                  algorithm = XMI_SCATTER_ALGORITHM_DEFAULT;            \
                  break;                                                \
                case XMI_XFER_SCATTER_INT:                              \
                  algorithm = XMI_SCATTER_INT_ALGORITHM_DEFAULT;        \
                  break;                                                \
                case XMI_XFER_BARRIER:                                  \
                  algorithm = XMI_BARRIER_ALGORITHM_DEFAULT;            \
                  break;                                                \
                case XMI_XFER_ALLTOALL:                                 \
                  algorithm = XMI_ALLTOALL_ALGORITHM_DEFAULT;           \
                  break;                                                \
                case XMI_XFER_ALLTOALLV:                                \
                  algorithm = XMI_ALLTOALLV_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_ALLTOALLV_INT:                            \
                  algorithm = XMI_ALLTOALLV_INT_ALGORITHM_DEFAULT;      \
                  break;                                                \
                case XMI_XFER_SCAN:                                     \
                  algorithm = XMI_SCAN_ALGORITHM_DEFAULT;               \
                  break;                                                \
                case XMI_XFER_AMBROADCAST:                              \
                  algorithm = XMI_AMBROADCAST_ALGORITHM_DEFAULT;        \
                  break;                                                \
                case XMI_XFER_AMSCATTER:                                \
                  algorithm = XMI_AMSCATTER_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_AMGATHER:                                 \
                  algorithm = XMI_AMGATHER_ALGORITHM_DEFAULT;           \
                  break;                                                \
                case XMI_XFER_AMREDUCE:                                 \
                  algorithm = XMI_AMREDUCE_ALGORITHM_DEFAULT;           \
                  break;                                                \
                default:                                                \
                  printf("XMI: invalid xmi_xfer_type\n");               \
                  break;                                                \
            }                                                           \
      }                                                                 \
  while (0);                                                            \
                                                                        \
  /* ************************************************************************* */
  /* ********* Transfer Types, used by geometry and xfer routines ************ */
  /* ************************************************************************* */
  typedef enum
  {
    XMI_XFER_BROADCAST = 0,
    XMI_XFER_ALLREDUCE,
    XMI_XFER_REDUCE,
    XMI_XFER_ALLGATHER,
    XMI_XFER_ALLGATHERV,
    XMI_XFER_ALLGATHERV_INT,
    XMI_XFER_SCATTER,
    XMI_XFER_SCATTERV,
    XMI_XFER_SCATTERV_INT,
    XMI_XFER_BARRIER,
    XMI_XFER_ALLTOALL,
    XMI_XFER_ALLTOALLV,
    XMI_XFER_ALLTOALLV_INT,
    XMI_XFER_SCAN,
    XMI_XFER_AMBROADCAST,
    XMI_XFER_AMSCATTER,
    XMI_XFER_AMGATHER,
    XMI_XFER_AMREDUCE,
    XMI_XFER_COUNT
  }xmi_xfer_type_t;

  /* ************************************************************************* */
  /* **************     Geometry (like groups/communicators)  **************** */
  /* ************************************************************************* */
  /**
   * \brief Initialize the geometry
   *
   * \param[out] geometry        Opaque geometry object to initialize
   * \param[in]  id              Unique identifier for this geometry
   *                             which globally represents this geometry
   * \param[in]  rank_slices     Array of node slices participating in the geometry
   *                             User must keep the array of slices in memory for the
   *                             duration of the geometry's existence
   * \param[in]  slice_count     Number of nodes participating in the geometry
   */
  xmi_result_t XMI_Geometry_initialize (xmi_context_t               context,
                                        xmi_geometry_t            * geometry,
                                        unsigned                    id,
                                        xmi_geometry_range_t      * rank_slices,
                                        unsigned                    slice_count);

  /**
   * \brief Initialize the geometry
   *
   * \param[in]  context         xmi context
   * \param[in]  world_geometry  world geometry object
   */
  xmi_result_t XMI_Geometry_world (xmi_context_t               context,
                                   xmi_geometry_t            * world_geometry);

  /**
   * \brief determines the number of algorithms available for a given op
   *        in the two different lists (always work list,
   *        under-cetain conditions list).
   *
   * \param[in]     context       xmi context.
   * \param[in]     geometry      An input geometry to be analyzed.
   * \param[in]     coll_type     type of collective op.
   * \param[in/out] lists_lengths array of 2 numbers representing all valid
   algorithms and optimized algorithms.
   * \retval        XMI_SUCCESS   number of algorithms is determined.
   * \retval        ?????         There is an error with input parameters
   */
  xmi_result_t XMI_Geometry_algorithms_num (xmi_context_t context,
                                            xmi_geometry_t geometry,
                                            xmi_xfer_type_t coll_type,
                                            int *lists_lengths);

  /**
   * \brief fills in the attributes for a given algorithm.
   *
   * \param[in]     context        xmi context.
   * \param[in]     geometry       An input geometry to be analyzed.
   * \param[in]     coll_type      type of collective op.
   * \param[in]     algorithm      algorithm's id
   * \param[in]     algorithm_type tells whether this an "always works"
   *                               or "works under-condition"
   * \param[out]    mdata          metadata to be filled in if algorithm is
   *                               applicable.
   * \retval        XMI_SUCCESS    algorithm is applicable to geometry.
   * \retval        ?????          Error in input arguments or not applicable.
   */

  xmi_result_t XMI_Geometry_algorithm_info (xmi_context_t context,
                                            xmi_geometry_t geometry,
                                            xmi_xfer_type_t type,
                                            xmi_algorithm_t algorithm,
                                            int algorithm_type,
                                            xmi_metadata_t *mdata);

  /**
   * \brief Analyze a protocol to ensure that it will run on the specified geometry
   *
   * \param[in]     geometry   An input geometry to be analyzed.
   * \param[in/out] algorithm  Storage for a list of algorithm id's
   * \param[in/out] num        number of algorithms in the list in/requested, out/actual
   * \retval        XMI_SUCCESS The protocol will run on the current geometry
   * \retval        ?????      The protocol does not support the current geometry
   */
  xmi_result_t XMI_Geometry_algorithm (xmi_context_t              context,
                                       xmi_xfer_type_t            colltype,
                                       xmi_geometry_t             geometry,
                                       xmi_algorithm_t           *algorithm,
                                       int                       *num);

  /**
   * \brief Free any memory allocated inside of a geometry.
   * \param[in] geometry The geometry object to free
   * \retval XMI_SUCCESS Memory free didn't fail
   */
  xmi_result_t XMI_Geometry_finalize(xmi_context_t   context,
                                     xmi_geometry_t  geometry);

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
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
    char                     * sndbuf;
    xmi_type_t               * stype;
    size_t                   * stypecounts;
    size_t                   * sdispls;
    char                     * rcvbuf;
    xmi_type_t               * rtype;
    size_t                   * rtypecounts;
    size_t                   * rdispls;
  }xmi_alltoallv_t;

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
    xmi_xfer_type_t           xfer_type;
    xmi_event_function        cb_done;
    void                    * cookie;
    xmi_geometry_t            geometry;
    xmi_algorithm_t           algorithm;
    char                    * sndbuf;
    xmi_type_t              * stype;
    int                     * stypecounts;
    int                     * sdispls;
    char                    * rcvbuf;
    xmi_type_t              * rtype;
    int                     * rtypecounts;
    int                     * rdispls;
  }xmi_alltoallv_int_t;


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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    xmi_algorithm_t             algorithm;
    char                      * sndbuf;
    xmi_type_t                * stype;
    size_t                      stypecount;
    char                      * rcvbuf;
    xmi_type_t                * rtype;
    size_t                      rtypecount;
  }xmi_alltoall_t;

  /**
   * \brief Create and post a non-blocking reduce operation.
   * The reduce operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         Rank of the reduce root node.
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
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
    size_t                     root;
    char                     * sndbuf;
    xmi_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    xmi_type_t               * rtype;
    size_t                     rtypecount;
    xmi_dt                     dt;
    xmi_op                     op;
  }xmi_reduce_t;

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
    xmi_xfer_type_t           xfer_type;
    xmi_event_function        cb_done;
    void                    * cookie;
    xmi_geometry_t            geometry;
    xmi_algorithm_t           algorithm;
    char                    * sndbuf;
    xmi_type_t              * stype;
    size_t                    stypecount;
    char                    * rcvbuf;
    xmi_type_t              * rtype;
    size_t                    rtypecount;
    size_t                  * rcounts;
    xmi_dt                    dt;
    xmi_op                    op;
  }xmi_reduce_scatter_t;

  /**
   * \brief Create and post a non-blocking broadcast operation.
   *
   * The broadcast operation ...
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         Rank of the node performing the broadcast.
   * \param[in]  buf          Source buffer to broadcast on root, dest buffer on non-root
   * \param[in]  type         data type layout, may be different on root/destinations
   * \param[in]  count        Single type replication count
   *
   * \retval     0            Success
   *
   */
  typedef struct
  {
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    xmi_algorithm_t             algorithm;
    size_t                      root;
    char                      * buf;
    xmi_type_t                * type;
    size_t                      typecount;
  }xmi_broadcast_t;


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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    xmi_algorithm_t             algorithm;
    char                      * sndbuf;
    xmi_type_t                * stype;
    size_t                      stypecount;
    char                      * rcvbuf;
    xmi_type_t                * rtype;
    size_t                      rtypecount;
  }xmi_allgather_t;


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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    xmi_algorithm_t             algorithm;
    size_t                      root;
    char                      * sndbuf;
    xmi_type_t                * stype;
    size_t                      stypecount;
    char                      * rcvbuf;
    xmi_type_t                * rtype;
    size_t                      rtypecount;
  }xmi_gather_t;

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
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
    size_t                     root;
    char                     * sndbuf;
    xmi_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    xmi_type_t               * rtype;
    size_t                   * rtypecounts;
    size_t                   * rdispls;
  }xmi_gatherv_t;

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
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
    size_t                     root;
    char                     * sndbuf;
    xmi_type_t               * stype;
    int                        stypecount;
    char                     * rcvbuf;
    xmi_type_t               * rtype;
    int                      * rtypecounts;
    int                      * rdispls;
  }xmi_gatherv_int_t;


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
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
    char                     * sndbuf;
    xmi_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    xmi_type_t               * rtype;
    size_t                   * rtypecounts;
    size_t                   * rdispls;
  }xmi_allgatherv_t;

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
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
    char                     * sndbuf;
    xmi_type_t               * stype;
    int                        stypecount;
    char                     * rcvbuf;
    xmi_type_t               * rtype;
    int                      * rtypecounts;
    int                      * rdispls;
  }xmi_allgatherv_int_t;


  /**
   * \brief Create and post a non-blocking scatter
   *
   * The scatter
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
   * \param[in]  root         Rank of the reduce root node.
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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    xmi_algorithm_t             algorithm;
    size_t                      root;
    char                      * sndbuf;
    xmi_type_t                * stype;
    size_t                      stypecount;
    char                      * rcvbuf;
    xmi_type_t                * rtype;
    size_t                      rtypecount;
  }xmi_scatter_t;

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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    xmi_algorithm_t             algorithm;
    size_t                      root;
    char                      * sndbuf;
    xmi_type_t                * stype;
    size_t                    * stypecounts;
    size_t                    * sdispls;
    char                      * rcvbuf;
    xmi_type_t                * rtype;
    size_t                      rtypecount;
  }xmi_scatterv_t;

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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    xmi_algorithm_t             algorithm;
    size_t                      root;
    char                      * sndbuf;
    xmi_type_t                * stype;
    int                       * stypecounts;
    int                       * sdispls;
    char                      * rcvbuf;
    xmi_type_t                * rtype;
    int                         rtypecount;
  }xmi_scatterv_int_t;


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
   * \todo discuss collapsing XMI_dt into type type.
   */
  typedef struct
  {
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
    char                     * sndbuf;
    xmi_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    xmi_type_t               * rtype;
    size_t                     rtypecount;
    xmi_dt                     dt;
    xmi_op                     op;
  }xmi_allreduce_t;


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
   * \todo discuss collapsing XMI_dt into type type.
   */
  typedef struct
  {
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
    char                     * sndbuf;
    xmi_type_t               * stype;
    size_t                     stypecount;
    char                     * rcvbuf;
    xmi_type_t               * rtype;
    size_t                     rtypecount;
    xmi_dt                     dt;
    xmi_op                     op;
    int                        exclusive;
  }xmi_scan_t;

  /**
   * \brief Create and post a non-blocking barrier operation.
   * The barrier operation ...
   * \param      geometry     Geometry to use for this collective operation.
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \retval  0            Success
   *
   * \see XMI_Barrier_register
   *
   * \todo doxygen
   */
  typedef struct
  {
    xmi_xfer_type_t            xfer_type;
    xmi_event_function         cb_done;
    void                     * cookie;
    xmi_geometry_t             geometry;
    xmi_algorithm_t            algorithm;
  }xmi_barrier_t;


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
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    xmi_algorithm_t             algorithm;
    void                      * user_header;
    size_t                      headerlen;
    void                      * sndbuf;
    xmi_type_t                * stype;
    size_t                      stypecount;
  }xmi_ambroadcast_t;
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
  typedef void (*xmi_dispatch_ambroadcast_fn) (xmi_context_t         context,
                                               size_t                root,
                                               xmi_geometry_t        geometry,
                                               const size_t          sndlen,
                                               void                * user_header,
                                               const size_t          headerlen,
                                               void               ** rcvbuf,
                                               xmi_type_t          * rtype,
                                               size_t              * rtypecount,
                                               xmi_event_function  * const cb_info,
                                               void               ** cookie);


  /**
   * \brief Create and post a non-blocking active message scatter operation.
   * The Active Message scatter operation ...
   *
   * This is slightly more complicated than an AMBroadcast, because it allows
   * different headers and data buffers to be sent to everyone in the team.
   *
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    void                      * headers;
    size_t                      headerlen;
    void                      * sndbuf;
    xmi_type_t                * stype;
    size_t                      stypecount;
  }xmi_amscatter_t;
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
  typedef void (*xmi_dispatch_amscatter_fn) (size_t               root,
                                             xmi_geometry_t       geometry,
                                             const unsigned       sndlen,
                                             void               * user_header,
                                             const size_t         headerlen,
                                             void              ** rcvbuf,
                                             xmi_type_t         * rtype,
                                             size_t             * rtypecount,
                                             xmi_event_function * const cb_info);

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
   * \param[in]  cb_done      Callback to invoke when message is complete.
   * \param[in]  geometry     Geometry to use for this collective operation.
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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    void                      * headers;
    size_t                      headerlen;
    void                      * rcvbuf;
    xmi_type_t                * rtype;
    size_t                      rtypecount;
  }xmi_amgather_t;
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
  typedef void (*xmi_dispatch_amgather_fn) (size_t               root,
                                            xmi_geometry_t       geometry_id,
                                            const unsigned       sndlen,
                                            void               * user_header,
                                            const size_t         headerlen,
                                            void              ** sndbuf,
                                            xmi_type_t         * stype,
                                            size_t             * stypecount,
                                            xmi_event_function * const cb_info);

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
   * \param[in]  registration Protocol registration.
   * \param[in]  request      Opaque memory to maintain internal message state.
   * \param[in]  cb_done      Callback to invoke when message is complete.
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
    xmi_xfer_type_t             xfer_type;
    xmi_event_function          cb_done;
    void                      * cookie;
    xmi_geometry_t              geometry;
    void                      * user_header;
    size_t                      headerlen;
    void                      * rcvbuf;
    xmi_type_t                * rtype;
    size_t                      rtypecount;
    xmi_dt                      dt;
    xmi_op                      op;
  }xmi_amreduce_t;

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
  typedef void (*xmi_dispatch_amreduce_fn) (size_t               root,
                                            xmi_geometry_t       geometry_id,
                                            const unsigned       sndlen,
                                            xmi_dt               dt,
                                            xmi_op               op,
                                            void               * user_header,
                                            const size_t         headerlen,
                                            void              ** sndbuf,
                                            xmi_type_t         * stype,
                                            size_t             * stypecount,
                                            xmi_event_function * const cb_info);

  typedef union
  {
    xmi_xfer_type_t        xfer_type;
    xmi_allreduce_t        xfer_allreduce;
    xmi_broadcast_t        xfer_broadcast;
    xmi_reduce_t           xfer_reduce;
    xmi_allgather_t        xfer_allgather;
    xmi_allgatherv_t       xfer_allgatherv;
    xmi_allgatherv_int_t   xfer_allgatherv_int;
    xmi_scatter_t          xfer_scatter;
    xmi_scatterv_t         xfer_scatterv;
    xmi_scatterv_int_t     xfer_scatterv_int;
    xmi_scatter_t          xfer_gather;
    xmi_scatter_t          xfer_gatherv;
    xmi_scatterv_t         xfer_gatherv_int;
    xmi_alltoall_t         xfer_alltoall;
    xmi_alltoallv_t        xfer_alltoallv;
    xmi_alltoallv_int_t    xfer_alltoallv_int;
    xmi_ambroadcast_t      xfer_ambroadcast;
    xmi_amscatter_t        xfer_amscatter;
    xmi_amgather_t         xfer_amgather;
    xmi_amreduce_t         xfer_amreduce;
    xmi_scan_t             xfer_scan;
    xmi_barrier_t          xfer_barrier;
  }xmi_xfer_t;

  xmi_result_t XMI_Collective (xmi_context_t context, xmi_xfer_t *cmd);

  typedef xmi_quad_t xmi_pipeworkqueue_t[4];
  typedef xmi_quad_t xmi_pipeworkqueue_ext_t[2];

  /**
   * \brief Configure for Shared Circular Buffer variety.
   *
   * Only one consumer and producer are allowed.
   * Creates a circular buffer of specified size in shared memory.
   * Buffer size must be power-of-two.
   *
   * \param[out] wq	Opaque memory for PipeWorkQueue
   * \param[in] bufsize	Size of buffer to allocate
   */
  void XMI_PipeWorkQueue_config_circ(xmi_pipeworkqueue_t *wq, size_t bufsize);

  /**
   * \brief Configure for User-supplied Circular Buffer variety.
   *
   * Only one consumer and producer are allowed.
   * Uses the provided buffer as a circular buffer of specified size.
   * Buffer size must be power-of-two.
   * Assumes the caller has placed buffer and (this) in appropriate memory
   * for desired use - i.e. all in shared memory if to be used beyond this process.
   *
   * NOTE: details need to be worked out. The buffer actually needs to include the
   * WQ header, so the caller must somehow know how much to allocate memory -
   * and how to ensure desired alignment.
   *
   * \param[out] wq		Opaque memory for PipeWorkQueue
   * \param[in] buffer		Buffer to use
   * \param[in] bufsize	Size of buffer
   */
  void XMI_PipeWorkQueue_config_circ_usr(xmi_pipeworkqueue_t *wq, char *buffer, size_t bufsize);

  /**
   * \brief Configure for Memory (flat buffer) variety.
   *
   * Only one consumer and producer are allowed. Still supports pipelining.
   * Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
   * Assumes the caller has placed buffer and (this) in appropriate memory
   * for desired use - i.e. all in shared memory if to be used beyond this process.
   *
   * \param[out] wq		Opaque memory for PipeWorkQueue
   * \param[in] buffer		Buffer to use
   * \param[in] bufsize	Size of buffer
   * \param[in] bufinit	Amount of data initially in buffer
   */
  void XMI_PipeWorkQueue_config_flat(xmi_pipeworkqueue_t *wq, char *buffer, size_t bufsize, size_t bufinit);

  /**
   * \brief PROPOSAL: Configure for Non-Contig Memory (flat buffer) variety.
   *
   * Only one consumer and producer are allowed. Still supports pipelining.
   * Sets up a flat buffer of specified maximum size with an arbitrary "initial fill".
   * Assumes the caller has placed buffer and (this) in appropriate memory
   * for desired use - i.e. all in shared memory if to be used beyond this process.
   *
   * This is typically only used for the application buffer, either input or output,
   * and so would not normally have both producer and consumer (only one or the other).
   * The interface is the same as for contiguous data except that "bytesAvailable" will
   * only return the number of *contiguous* bytes available. The user must consume those
   * bytes before it can see the next contiguous chunk.
   *
   * \param[out] wq            Opaque memory for PipeWorkQueue
   * \param[in] buffer         Buffer to use
   * \param[in] type           Memory layout of a buffer unit
   * \param[in] typecount      Number of repetitions of buffer units
   * \param[in] typeinit       Number of units initially in buffer
   */
  void XMI_PipeWorkQueue_config_noncontig(xmi_pipeworkqueue_t *wq, char *buffer, xmi_type_t *type, size_t typecount, size_t typeinit);

  /**
   * \brief Export
   *
   * Produces information about the PipeWorkQueue into the opaque buffer "export".
   * This info is suitable for sharing with other processes such that those processes
   * can then construct a PipeWorkQueue which accesses the same data stream.
   *
   * This only has value in the case of a flat buffer PipeWorkQueue and a platform
   * that supports direct mapping of memory from other processes. Circular buffer
   * PipeWorkQueues use shared memory and are inherently inter-process in nature.
   *
   * The exporting process is the only one that can produce to the PipeWorkQueue.
   * All importing processes are restricted to consuming. There is no feedback
   * from the consumers to the producer - no flow control.
   *
   * \param[in] wq             Opaque memory for PipeWorkQueue
   * \param[out] export        Opaque memory to export into
   * \return	success of the export operation
   */
  xmi_result_t XMI_PipeWorkQueue_export(xmi_pipeworkqueue_t *wq, xmi_pipeworkqueue_ext_t *exp);

  /**
   * \brief Import
   *
   * Takes the results of an export of a PipeWorkQueue on a different process and
   * constructs a new PipeWorkQueue which the local process may use to access the
   * data stream.
   *
   * The resulting PipeWorkQueue may consume data, but that is a local-only operation.
   * The producer has no knowledge of data consumed. There can be only one producer.
   * There may be multiple consumers, but the producer does not know about them.
   * An importing processes cannot be the producer.
   *
   * TODO: can this work for circular buffers? does it need to, since those are
   * normally shared memory and thus already permit inter-process communication.
   *
   * \param[in] import        Opaque memory into which an export was done.
   * \param[out] wq           Opaque memory for new PipeWorkQueue
   * \return	success of the import operation
   */
  xmi_result_t XMI_PipeWorkQueue_import(xmi_pipeworkqueue_ext_t *import, xmi_pipeworkqueue_t *wq);

  /**
   * \brief Clone constructor.
   *
   * Used to create a second local memory wrapper object of the same
   * shared memory resource.
   *
   * \see WorkQueue(WorkQueue &)
   *
   * \param[out] wq	Opaque memory for new PipeWorkQueue
   * \param[in] obj	old object, to be cloned
   */
  void XMI_PipeWorkQueue_clone(xmi_pipeworkqueue_t *wq, xmi_pipeworkqueue_t *obj);

  /**
   * \brief Destructor
   *
   * \param[out] wq	Opaque memory for PipeWorkQueue
   */
  void XMI_PipeWorkQueue_destroy(xmi_pipeworkqueue_t *wq);

  /**
   * \brief Reset this pipe work queue.
   *
   * All PipeWorkQueues must be reset() at least once after configure and before using.
   * (TODO: should this simply be part of configure?)
   *
   * Sets the number of bytes produced and the number of bytes
   * consumed to zero (or to "bufinit" as appropriate).
   *
   * This is typically required by circular PipeWorkQueues that are re-used.
   * Flat PipeWorkQueues are usually configured new for each instance
   * and thus do not require resetting. Circular PipeWorkQueues should be
   * reset by only one entity, and at a point when it is known that no other
   * entity is still using it (it must be idle). For example, in a multisend pipeline
   * consisting of:
   *
   *        [barrier] ; local-reduce -(A)-&gt; global-allreduce -(B)-&gt; local-broadcast
   *
   * the PipeWorkQueue "B" would be reset by the root of the reduce when starting the
   * local-reduce operation (when it is known that any prior instances have completed).
   *
   * One reason that a reset may be needed is to preserve buffer alignment. Another is
   * to prevent problems when, say, a consumer requires a full packet of data. In this
   * case, a circular PipeWorkQueue may have been left in a state from the previous
   * operation where the final chunk of data has left less than a packet length before
   * the wrap point. This would create a hang because the consumer would never see a full
   * packet until it consumes those bytes and passes the wrap point.
   *
   * Since resets are performed by the protocol code, it understands the context and
   * whether the PipeWorkQueue represents a flat (application) buffer or an intermediate
   * (circular) one.
   *
   * \param[out] wq	Opaque memory for PipeWorkQueue
   */
  void XMI_PipeWorkQueue_reset(xmi_pipeworkqueue_t *wq);

  /**
   * \brief Dump shared memory work queue statistics to stderr.
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \param[in] prefix Optional character string to prefix.
   */
  void XMI_PipeWorkQueue_dump(xmi_pipeworkqueue_t *wq, const char *prefix);

  /**
   * \brief register a wakeup for the consumer side of the PipeWorkQueue
   *
   * The 'vec' parameter is typically obtained from some platform authority,
   * which is the same used by the PipeWorkQueue to perform the wakeup.
   * A consumer wishing to be awoken when data is available would call the
   * system to get their 'vec' value, and pass it to this method to register
   * for wakeups. When the produceBytes method is called, it will use this
   * consumer wakeup 'vec' to tell the system to wake up the consumer process or thread.
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \param[in] vec	Opaque wakeup vector parameter
   */
  void XMI_PipeWorkQueue_setConsumerWakeup(xmi_pipeworkqueue_t *wq, void *vec);

  /**
   * \brief register a wakeup for the producer side of the PipeWorkQueue
   *
   * See setConsumerWakeup() for details.
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \param[in] vec	Opaque wakeup vector parameter
   */
  void XMI_PipeWorkQueue_setProducerWakeup(xmi_pipeworkqueue_t *wq, void *vec);

  /**
   * \brief Return the number of contiguous bytes that can be produced into this work queue.
   *
   * Bytes must be produced into the memory location returned by bufferToProduce() and then
   * this work queue \b must be updated with produceBytes().
   *
   * \see bufferToProduce
   * \see produceBytes
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return Number of bytes that may be produced.
   */
  size_t XMI_PipeWorkQueue_bytesAvailableToProduce(xmi_pipeworkqueue_t *wq);

  /**
   * \brief Return the number of contiguous bytes that can be consumed from this work queue.
   *
   * Bytes must be consumed into the memory location returned by bufferToConsume() and then
   * this work queue \b must be updated with consumeBytes().
   *
   * \see bufferToConsume
   * \see consumeBytes
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \param[in] consumer Consumer id for work queues with multiple consumers
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return Number of bytes that may be consumed.
   */
  size_t XMI_PipeWorkQueue_bytesAvailableToConsume(xmi_pipeworkqueue_t *wq);

  /**
   * \brief raw accessor for total number of bytes produced since reset()
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	number of bytes produced
   */
  size_t XMI_PipeWorkQueue_getBytesProduced(xmi_pipeworkqueue_t *wq);

  /**
   * \brief raw accessor for total number of bytes consumed since reset()
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	number of bytes consumed
   */
  size_t XMI_PipeWorkQueue_getBytesConsumed(xmi_pipeworkqueue_t *wq);

  /**
   * \brief current position for producing into buffer
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	location in buffer to produce into
   */
  char *XMI_PipeWorkQueue_bufferToProduce(xmi_pipeworkqueue_t *wq);

  /**
   * \brief notify workqueue that bytes have been produced
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	number of bytes that were produced
   */
  void XMI_PipeWorkQueue_produceBytes(xmi_pipeworkqueue_t *wq, size_t bytes);

  /**
   * \brief current position for consuming from buffer
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	location in buffer to consume from
   */
  char *XMI_PipeWorkQueue_bufferToConsume(xmi_pipeworkqueue_t *wq);

  /**
   * \brief notify workqueue that bytes have been consumed
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	number of bytes that were consumed
   */
  void XMI_PipeWorkQueue_consumeBytes(xmi_pipeworkqueue_t *wq, size_t bytes);

  /**
   * \brief is workqueue ready for action
   *
   * \param[in] wq	Opaque memory for PipeWorkQueue
   * \return	boolean indicate workqueue readiness
   */
  int XMI_PipeWorkQueue_available(xmi_pipeworkqueue_t *wq);


  /** \brief The various types a Topology can be */
  typedef enum {
    XMI_EMPTY_TOPOLOGY = 0, /**< topology represents no (zero) ranks    */
    XMI_SINGLE_TOPOLOGY,    /**< topology is for one rank               */
    XMI_RANGE_TOPOLOGY,     /**< topology is a simple range of ranks    */
    XMI_LIST_TOPOLOGY,      /**< topology is an unordered list of ranks */
    XMI_COORD_TOPOLOGY,     /**< topology is a rectangular segment
                               represented by coordinates           */
    XMI_TOPOLOGY_COUNT
  } xmi_topology_type_t;

  typedef xmi_quad_t xmi_topology_t[2];

  /**
   * \brief default constructor (XMI_EMPTY_TOPOLOGY)
   *
   * \param[out] topo	Opaque memory for topology
   */
  void XMI_Topology_create(xmi_topology_t *topo);

  /**
   * \brief rectangular segment with torus (XMI_COORD_TOPOLOGY)
   *
   * Assumes no torus links if no 'tl' param.
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] ll	lower-left coordinate
   * \param[in] ur	upper-right coordinate
   * \param[in] tl	optional, torus links flags
   */
  void XMI_Topology_create_rect(xmi_topology_t *topo,
                                xmi_coord_t *ll, xmi_coord_t *ur, unsigned char *tl);

  /**
   * \brief single rank constructor (XMI_SINGLE_TOPOLOGY)
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] rank	The rank
   */
  void XMI_Topology_create_rank(xmi_topology_t *topo, size_t rank);

  /**
   * \brief rank range constructor (XMI_RANGE_TOPOLOGY)
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] rank0	first rank in range
   * \param[in] rankn	last rank in range
   */
  void XMI_Topology_create_range(xmi_topology_t *topo, size_t rank0, size_t rankn);

  /**
   * \brief rank list constructor (XMI_LIST_TOPOLOGY)
   *
   * caller must not free ranks[]!
   *
   * \param[out] topo	Opaque memory for topology
   * \param[in] ranks	array of ranks
   * \param[in] nranks	size of array
   *
   * \todo create destructor to free list, or establish rules
   */
  void XMI_Topology_create_list(xmi_topology_t *topo, size_t *ranks, size_t nranks);

  /**
   * \brief destructor
   *
   * For XMI_LIST_TOPOLOGY, would free the ranks list array... ?
   *
   * \param[out] topo	Opaque memory for topology
   */
  void XMI_Topology_destroy(xmi_topology_t *topo);

  /**
   * \brief accessor for size of a Topology object
   *
   * \param[in] topo	Opaque memory for topology
   * \return	size of XMI::Topology
   */
  unsigned XMI_Topology_size_of(xmi_topology_t *topo);

  /**
   * \brief number of ranks in topology
   * \param[in] topo	Opaque memory for topology
   * \return	number of ranks
   */
  size_t XMI_Topology_size(xmi_topology_t *topo);

  /**
   * \brief type of topology
   * \param[out] topo	Opaque memory for topology
   * \return	topology type
   */
  xmi_topology_type_t xmi_topology_type(xmi_topology_t *topo);

  /**
   * \brief Nth rank in topology
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] ix	Which rank to select
   * \return	Nth rank or (size_t)-1 if does not exist
   */
  size_t XMI_Topology_index2Rank(xmi_topology_t *topo, size_t ix);

  /**
   * \brief determine index of rank in topology
   *
   * This is the inverse function to rank(ix) above.
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] rank	Which rank to get index for
   * \return	index of rank (rank(ix) == rank) or (size_t)-1
   */
  size_t XMI_Topology_rank2Index(xmi_topology_t *topo, size_t rank);

  /**
   * \brief return range
   *
   * \param[in] topo	Opaque memory for topology
   * \param[out] first	Where to put first rank in range
   * \param[out] last	Where to put last rank in range
   * \return	XMI_SUCCESS, or XMI_UNIMPL if not a range topology
   */
  xmi_result_t XMI_Topology_rankRange(xmi_topology_t *topo, size_t *first, size_t *last);

  /**
   * \brief return rank list
   *
   * \param[in] topo	Opaque memory for topology
   * \param[out] list	pointer to list stored here
   * \return	XMI_SUCCESS, or XMI_UNIMPL if not a list topology
   */
  xmi_result_t XMI_Topology_rankList(xmi_topology_t *topo, size_t **list);

  /**
   * \brief return rectangular segment coordinates
   *
   * This method copies data to callers buffers. It is safer
   * as the caller cannot directly modify the topology.
   *
   * \param[in] topo	Opaque memory for topology
   * \param[out] ll	lower-left coord pointer storage
   * \param[out] ur	upper-right coord pointer storage
   * \param[out] tl	optional, torus links flags
   * \return	XMI_SUCCESS, or XMI_UNIMPL if not a coord topology
   */
  xmi_result_t XMI_Topology_rectSeg(xmi_topology_t *topo,
                                    xmi_coord_t *ll, xmi_coord_t *ur,
                                    unsigned char *tl);

  /**
   * \brief does topology consist entirely of ranks local to self
   *
   * \param[in] topo	Opaque memory for topology
   * \return boolean indicating locality of ranks
   */
  int XMI_Topology_isLocalToMe(xmi_topology_t *topo);

  /**
   * \brief is topology a rectangular segment
   * \param[in] topo	Opaque memory for topology
   * \return	boolean indicating rect seg topo
   */
  int XMI_Topology_isRectSeg(xmi_topology_t *topo);

  /**
   * \brief extract Nth dimensions from coord topology
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] n	Which dim to extract
   * \param[out] c0	lower value for dim range
   * \param[out] cn	upper value for dim range
   * \param[out] tl	optional, torus link flag
   */
  void XMI_Topology_getNthDims(xmi_topology_t *topo, unsigned n,
                               unsigned *c0, unsigned *cn, unsigned char *tl);

  /**
   * \brief is rank in topology
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] rank	Rank to test
   * \return	boolean indicating rank is in topology
   */
  int XMI_Topology_isRankMember(xmi_topology_t *topo, size_t rank);

  /**
   * \brief is coordinate in topology
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] c0	Coord to test
   * \return	boolean indicating coord is a member of topology
   */
  int XMI_Topology_isCoordMember(xmi_topology_t *topo, xmi_coord_t *c0);

  /**
   * \brief create topology of ranks local to self
   *
   * \param[out] _new	Where to build topology
   * \param[in] topo	Opaque memory for topology
   */
  void XMI_Topology_sub_LocalToMe(xmi_topology_t *_new, xmi_topology_t *topo);

  /**
   * \brief create topology from all Nth ranks globally
   *
   * \param[out] _new	Where to build topology
   * \param[in] topo	Opaque memory for topology
   * \param[in] n	Which local rank to select on each node
   */
  void XMI_Topology_sub_NthGlobal(xmi_topology_t *_new, xmi_topology_t *topo, int n);

  /**
   * \brief reduce dimensions of topology (cube -> plane, etc)
   *
   * The 'fmt' param is a pattern indicating which dimensions
   * to squash, and what coord to squash into. A dim in 'fmt'
   * having "-1" will be preserved, while all others will be squashed
   * into a dimension of size 1 having the value specified.
   *
   * \param[out] _new	where to build new topology
   * \param[in] topo	Opaque memory for topology
   * \param[in] fmt	how to reduce dimensions
   */
  void XMI_Topology_sub_ReduceDims(xmi_topology_t *_new, xmi_topology_t *topo, xmi_coord_t *fmt);

  /**
   * \brief Return list of ranks representing contents of topology
   *
   * This always returns a list regardless of topology type.
   * Caller must allocate space for list, and determine an
   * appropriate size for that space. Note, there might be a
   * number larger than 'max' returned in 'nranks', but there
   * are never more than 'max' ranks put into the array.
   * If the caller sees that 'nranks' exceeds 'max' then it
   * should assume it did not get the whole list, and could
   * allocate a larger array and try again.
   *
   * \param[in] topo	Opaque memory for topology
   * \param[in] max	size of caller-allocated array
   * \param[out] ranks	array where rank list is placed
   * \param[out] nranks	actual number of ranks put into array
   */
  void XMI_Topology_getRankList(xmi_topology_t *topo, size_t max, size_t *ranks, size_t *nranks);

  /**
   * \brief check if rank range or list can be converted to rectangle
   *
   * Since a rectangular segment is consider the optimal state, no
   * other analysis is done. A XMI_SINGLE_TOPOLOGY cannot be optimized,
   * either. Optimization levels:
   *
   * 	XMI_SINGLE_TOPOLOGY (most)
   * 	XMI_COORD_TOPOLOGY
   * 	XMI_RANGE_TOPOLOGY
   * 	XMI_LIST_TOPOLOGY (least)
   *
   * \param[in,out] topo	Opaque memory for topology
   * \return	'true' if topology was changed
   */
  int XMI_Topology_analyze(xmi_topology_t *topo);

  /**
   * \brief check if topology can be converted to type
   *
   * Does not differentiate between invalid conversions and
   * 'null' conversions (same type).
   *
   * \param[in,out] topo	Opaque memory for topology
   * \param[in] new_type	Topology type to try and convert into
   * \return	'true' if topology was changed
   */
  int XMI_Topology_convert(xmi_topology_t *topo, xmi_topology_type_t new_type);

  /**
   * \brief produce the intersection of two topologies
   *
   * produces: _new = this ./\. other
   *
   * \param[out] _new	New topology created there
   * \param[in] topo	Opaque memory for topology
   * \param[in] other	The other topology
   */
  void XMI_Topology_intersect(xmi_topology_t *_new, xmi_topology_t *topo, xmi_topology_t *other);

  /**
   * \brief produce the difference of two topologies
   *
   * produces: _new = this .-. other
   *
   * \param[out] _new	New topology created there
   * \param[in] topo	Opaque memory for topology
   * \param[in] other	The other topology
   */
  void XMI_Topology_subtract(xmi_topology_t *_new, xmi_topology_t *topo, xmi_topology_t *other);

/**
 * \file sys/xmi_multisend.h
 * \brief Multisend interface.
 *
 * A multisend operation allows many message passing transactions to
 * be performed in the same call to amortize software overheads.  It
 * has two flavors
 *
 *   - multicast, which sends the same buffer to a list of processors
 *     and also supports depost-bit torus line broadcasts
 *
 *   - manytomany, which sends different offsets from the same buffer
 *     to a list of processors
 *
 * As reductions have a single source buffer and destination buffer,
 * we have extended the multicast call to support reductions.
 *
 * Each multisend operation carries a connection id to identify the
 * data on the reciever.
 *
 * In a multisend operation the cores are typically involved at the
 * end-points. Moreover the processor lists can be created on the fly.
 */

  /**
   * \brief Determine role information for a multisend protocol
   *
   * Roles are numbered 0..(N-1). Role 0 is the first role in the data-chain.
   * Role 0 performs the root task(s) of a broadcast operation, for example.
   * Role (N-1) (may also be 0 if numRoles is 1) is the last role in the data-chain.
   * It performs the root task(s) of a reduce operation, as an example. Roles between
   * 0 and (N-1) are intermediate roles of non-specific task. Typically, all roles
   * pass in "data" and "results", even though some roles may ignore them.
   * Roles also specify how to assign roles when there are more processes than roles.
   * The "R" (replication role) indicates which role to replicate for any additional
   * processes. Beyond that, each process is assigned role "N" (an invalid role).
   * Role information (numRoles and ReplRole) is determined from the factory that
   * was registered. So a typical procedure to generate a multicombine would be:
   *
   *	factory->getRoles(&numRoles, &replRole);
   *	// analyze and select role(s)
   *	factory->generate(..., roles, ...);
   *
   * Role information is static (?) so the analysis of roles might be done at
   * registration/init time and carried forward. If that is not the case, we may
   * need to add additional params or perhaps even split this out by multi* type.
   * For example, if there is not a 1:1 relationship between factory (registration)
   * and message/device, then some other way must be found to determine the roles
   * before calling factory->generate(). This might require passing some or all of
   * the generate() parameters into the getRoles() call (i.e. pass in the param struct).
   * Or, perhaps generate() returns the role information and then roles must be set
   * after generate(), but this creates problems not only with optimizations but
   * also because generate() often starts the message and roles must be known by that time.
   * Perhaps, generate() passes generic role info, such as "I am #2 of 4" and let
   * generate setup roles according to some standard algorithm. But that may not
   * allow for special treatment based on locality of data or other work assigned
   * to that core.
   *
   * Example 1: numRoles = 2 and replRole = 0 (e.g. a Reduce)
   *	All data participants use (replicate) role "0". The root uses role "1".
   *	If a participant is the root but is not supplying data, it would use
   *	the data_participants topology to indicate that.
   *
   * Example 2: numRoles = 2 and replRole = -1 (e.g. Tree device, injection/reception)
   *	One participant, closest to the input data, uses role "0". Another
   *	participant (closest to the results) uses role "1". If there is only one
   *	(local) participant, i.e. SMP mode, then the role is "0|1" (both roles).
   *
   * Example 3: numRoles = 1 and replRole = -1 (e.g. one-sided bcast)
   *	(a.k.a multicast)
   *	Origin uses role "0" and sets results participants to the destinations.
   *	Destinations all get data via recv callback setup at registration time.
   *
   * It is expected that the caller of a multisend has some knowledge of the type
   * of multisend being performed. This is required in order to even setup
   * topologies, let alone roles. There is much context required to make a
   * multisend call, so adding the roles as a dependency of the context should
   * not be overly restrictive.
   *
   * \param[in] registration	The protocol to be analysed
   * \param[out] numRoles	The number of different roles supported/required
   * \param[out] replRole	The role to replicate for additional participants,
   *				or -1 if no additional roles are used.
   * \return	success or failure
   */
  xmi_result_t XMI_Multisend_getroles(xmi_context_t  context,
                                      size_t         dispatch,
                                      int           *numRoles,
                                      int           *replRole);

  /**
   * \brief Recv callback for Multicast
   *
   * Note, certain flavors of Multicast do not use a Receive Callback and
   * constructing or registering with a non-NULL cb_recv will result in error.
   *
   * Does this accept zero-byte (no data, no metadata) operations?
   *
   * \param[in] msginfo		Metadata
   * \param[in] msgcount	Count of metadata
   * \param[in] connection_id  Stream ID of data
   * \param[in] root		Sending rank
   * \param[in] sndlen		Length of data sent
   * \param[in] clientdata	Opaque arg
   * \param[out] rcvlen		Length of data to receive
   * \param[out] rcvbuf		Where to put recv data
   * \param[out] cb_done	Completion callback to invoke when data received
   * \return	XMI_Request opaque memory for message
   */
  typedef void (*xmi_dispatch_multicast_fn)(const xmi_quad_t        *msginfo,
                                            unsigned              msgcount,
                                            unsigned              connection_id,
                                            size_t                root,
                                            size_t                sndlen,
                                            void                 *clientdata,
                                            size_t               *rcvlen,
                                            xmi_pipeworkqueue_t **rcvbuf,
                                            xmi_event_function   *cb_done);

  /**
   * \brief The new structure to pass parameters for the multisend multicast operation.
   *
   * The XMI_Multicast_t object is re-useable immediately, but objects referred to
   * (src, etc) cannot be re-used until cb_done.
   */
  typedef struct {
    void                *request; 	       /**< space available in request, bytes */
    xmi_callback_t       cb_done;          /**< Completion callback */
    unsigned             connection_id;    /**< A connection is a distinct stream of
                                              traffic. The connection id identifies the
                                              connection */
    unsigned             roles;            /**< bitmap of roles to perform */
    size_t               bytes;            /**< size of the message*/
    xmi_pipeworkqueue_t *src;              /**< source buffer */
    xmi_topology_t      *src_participants; /**< root */
    xmi_pipeworkqueue_t *dst;              /**< dest buffer (ignored for one-sided) */
    xmi_topology_t      *dst_participants; /**< destinations to multicast to*/
    const xmi_quad_t    *msginfo;	       /**< A extra info field to be sent with the message.
                                                  This might include information about
                                                  the data being sent, for one-sided. */
    unsigned            msgcount;          /**< info count*/
  } xmi_multicast_t;
  xmi_result_t XMI_Multicast(xmi_context_t context,xmi_multicast_t *mcastinfo);


  /**  Deprecated Multicast:  To be deleted soon!!! */
  /**********************************************************************/
  typedef enum
  {
    XMI_UNDEFINED_CONSISTENCY = -1,
    XMI_RELAXED_CONSISTENCY,
    XMI_MATCH_CONSISTENCY,
    XMI_WEAK_CONSISTENCY,
    XMI_CONSISTENCY_COUNT
  }xmi_consistency_t;

#define  LINE_BCAST_MASK    (XMI_LINE_BCAST_XP|XMI_LINE_BCAST_XM|	\
                             XMI_LINE_BCAST_YP|XMI_LINE_BCAST_YM|	\
                             XMI_LINE_BCAST_ZP|XMI_LINE_BCAST_ZM)
  typedef enum
  {
    XMI_PT_TO_PT_SUBTASK           =  0,      //Send a pt-to-point message
    XMI_LINE_BCAST_XP              =  0x20,   //Bcast along x+
    XMI_LINE_BCAST_XM              =  0x10,   //Bcast along x-
    XMI_LINE_BCAST_YP              =  0x08,   //Bcast along y+
    XMI_LINE_BCAST_YM              =  0x04,   //Bcast along y-
    XMI_LINE_BCAST_ZP              =  0x02,   //Bcast along z+
    XMI_LINE_BCAST_ZM              =  0x01,   //Bcast along z-
    XMI_COMBINE_SUBTASK            =  0x0100,   //Combine the incoming message
    //with the local state
    XMI_GI_BARRIER                 =  0x0200,
    XMI_LOCKBOX_BARRIER            =  0x0300,
    XMI_TREE_BARRIER               =  0x0400,
    XMI_TREE_BCAST                 =  0x0500,
    XMI_TREE_ALLREDUCE             =  0x0600,
    XMI_REDUCE_RECV_STORE          =  0x0700,
    XMI_REDUCE_RECV_NOSTORE        =  0x0800,
    XMI_BCAST_RECV_STORE           =  0x0900,
    XMI_BCAST_RECV_NOSTORE         =  0x0a00,
    XMI_LOCALCOPY                  =  0x0b00,
    XMI_UNDEFINED_SUBTASK          =  (~LINE_BCAST_MASK),
  } xmi_subtask_t;


  typedef xmi_quad_t * (*xmi_olddispatch_multicast_fn) (const xmi_quad_t   * info,
                                                        unsigned             count,
                                                        unsigned             peer,
                                                        unsigned             sndlen,
                                                        unsigned             conn_id,
                                                        void               * arg,
                                                        unsigned           * rcvlen,
                                                        char              ** rcvbuf,
                                                        unsigned           * pipewidth,
                                                        xmi_callback_t     * cb_done);
  typedef struct xmi_oldmulticast_t
  {
    xmi_quad_t        * request;
    xmi_callback_t      cb_done;
    unsigned            connection_id;
    unsigned            bytes;
    const char        * src;
    unsigned            nranks;
    unsigned          * ranks;
    xmi_subtask_t     * opcodes;
    const xmi_quad_t  * msginfo;
    unsigned            count;
    unsigned            flags;
    xmi_op              op;
    xmi_dt              dt;

  }xmi_oldmulticast_t;

  typedef struct xmi_oldmulticast_recv_t
  {
    xmi_quad_t        * request;
    xmi_callback_t      cb_done;
    unsigned            connection_id;
    unsigned            bytes;
    char              * rcvbuf;
    unsigned            pipelineWidth;
    xmi_subtask_t       opcode;
    xmi_op              op;
    xmi_dt              dt;
  }xmi_oldmulticast_recv_t;


  typedef xmi_quad_t * (*xmi_olddispatch_manytomany_fn) (unsigned          conn_id,
                                                         void            * arg,
                                                         char           ** rcvbuf,
                                                         size_t       ** rcvdispls,
                                                         size_t       ** rcvlens,
                                                         size_t       **rcvcounters,
                                                         size_t        * nranks,
                                                         xmi_callback_t  * cb_done);

  
  /**********************************************************************/



  /**
   * \brief Sub-structure used to represent a vectored buffer for many-to-many.
   *
   * This is used for both send and receive parameters.
   *
   * This structure is likely to change as decisions are made about how
   * much of the vector to embed inside PipeWorkQueue and if Topology
   * contains permutation information.
   */
  typedef struct {
    xmi_pipeworkqueue_t *buffer;	    /**< Memory used for data (buffer) */
    xmi_topology_t      *participants;  /**< Ranks that are vectored in buffer */
    size_t              *lengths;       /**< Array of lengths in buffer for each rank */
    size_t              *offsets;       /**< Array of offsets in buffer for each rank */
    size_t               num_vecs;      /**< The number of entries in "lengths" and
                                           "offsets". May be a flag: either "1" or
                                           participants->size(). */
  } xmi_manytomanybuf_t;

  /**
   * \brief Callback for Manytomany Receive operations
   *
   * Note, certain flavors of ManyToMany do not use a Receive Callback and
   * constructing or registering with a non-NULL cb_recv will result in error.
   *
   * All senders of metadata for a given connection_id (instance) must send
   * the same, or at least compatible, metadata such that all would return the
   * same receive parameters (**recv). Note, the recv callback will be invoked
   * only once per connection_id. The first sender message to arrive will invoke
   * the callback and get recv params for ALL other senders in the instance.
   *
   * The myIndex parameter is the receiving rank's index in the recv arrays
   * (lengths and offsets) and is used by the manytomany as an optimization
   * for handling reception data and completion.
   *
   * \param[in] arg		Client Data
   * \param[in] conn_id		Instance ID
   * \param[in] metadata	Pointer to metadata, if any, in message header.
   * \param[in] metacount	Number of xmi_quad_ts of metadata.
   * \param[out] recv		Receive parameters for this connection (instance)
   * \param[out] myIndex	Index of Recv Rank in the receive parameters.
   * \param[out] cb_done	Completion callback when message complete
   * \return	Request object opaque storage for message.
   */
  typedef void (*xmi_dispatch_manytomany_fn)(void                 *arg,
                                             unsigned              conn_id,
                                             xmi_quad_t           *metadata,
                                             unsigned              metacount,
                                             xmi_manytomanybuf_t **recv,
                                             size_t               *myIndex,
                                             xmi_callback_t       *cb_done);

  /**
   * \brief Structure of parameters used to initiate a ManyToMany
   *
   * The rankIndex parameter is transmitted to the receiver for use by cb_recv
   * for indexing into the recv parameter arrays (lengths and offsets).
   */
  typedef struct {
    void                *request; 	     /**< space available in request, bytes */
    xmi_callback_t       cb_done;	     /**< User's completion callback */
    unsigned             connection_id;  /**< differentiate data streams */
    unsigned             roles;          /**< bitmap of roles to perform */
    size_t              *rankIndex;	     /**< Index of send in recv parameters */
    size_t               num_index;      /**< Number of entries in "rankIndex".
                                            should be multiple of send.participants->size()?
                                         */
    xmi_manytomanybuf_t  send;           /**< send data parameters */
    const xmi_quad_t    *metadata;	     /**< A extra info field to be sent with the message.
                                                This might include information about
                                                the data being sent, for one-sided. */
    unsigned             metacount;	     /**< metadata count*/
  } xmi_manytomany_t;

  /**
   * \brief Initiate a ManyToMany
   *
   * \param[in] m2minfo	Paramters for ManyToMany operation to be performed
   * \return	XMI_SUCCESS or error code
   */
  xmi_result_t XMI_Manytomany(xmi_context_t context,xmi_manytomany_t *m2minfo);

  /******************************************************************************
   *       Multisync Personalized synchronization/coordination
   ******************************************************************************/

  /**
   * \brief Recv callback for Multisync.
   *
   * Not normally used.
   *
   * Note, certain flavors of Multisync do not use a Receive Callback and
   * constructing or registering with a non-NULL cb_recv will result in error.
   *
   * \param[in] clientdata	Opaque arg
   * \param[in] msginfo		Metadata
   * \param[in] msgcount	Number of xmi_quad_ts in msginfo
   * \param[in] conn_id		Instance ID
   * \return	XMI_Request opaque memory for message
   */
  typedef void (*xmi_dispatch_multisync_fn)(void       *clientdata,
                                            xmi_quad_t *msginfo,
                                            unsigned    msgcount,
                                            unsigned    conn_id);
  /**
   * \brief structure defining interface to Multisync
   */
  typedef struct {
    void              *request;	        /**< space available in request, bytes */
    xmi_callback_t     cb_done;		/**< User's completion callback */
    unsigned           connection_id;	/**< (remove?) differentiate data streams */
    unsigned           roles;		/**< bitmap of roles to perform */
    xmi_topology_t    *participants;	/**< Ranks involved in synchronization */
  } xmi_multisync_t;
  /**
   * \brief Barriers and the like.
   *
   * All participants make this call. So, there is no "send" or "recv"
   * distinction needed.
   *
   * \param[in] msyncinfo	Struct of all params needed to perform operation
   * \return	XMI_SUCCESS or error codes
   */
  xmi_result_t XMI_Multisync(xmi_context_t context,xmi_multisync_t *msyncinfo);


  /******************************************************************************
   *       Multicombine Personalized reduction
   ******************************************************************************/
  /**
   * \brief structure defining interface to Multicombine
   *
   * The recv callback, and associated metadata parameters, are not valid for all
   * multicombines. Depending on the kind of multicombine being registered, it may
   * require that the recv callback be either NULL or valid. If the recv callback
   * is NULL then the metadata parameters should also be NULL (0) when inoking
   * the multicombine.
   *
   * data and results parameters may not always be required, depending on role (and other?).
   * For example, if a call the a multicombine specifies a single role of, say, "injection",
   * then the results parameters are not needed. Details of this are specified by the
   * type of multicombine being registered/used.
   */
  typedef struct {
    void                *request;	              /**< space available in request, bytes */
    xmi_callback_t       cb_done;	              /**< User's completion callback */
    unsigned             roles;		      /**< bitmap of roles to perform */
    xmi_pipeworkqueue_t *data;		      /**< Data source */
    xmi_topology_t      *data_participants;	      /**< Ranks contributing data */
    xmi_pipeworkqueue_t *results;	              /**< Results destination */
    xmi_topology_t      *results_participants;    /**< Ranks receiving results */
    xmi_op               optor;		      /**< Operation to perform on data */
    xmi_dt               dtype;		      /**< Datatype of elements */
    size_t               count;		      /**< Number of elements */
  } xmi_multicombine_t;

  /**
   * \brief Allreduce, Reduce, etc. (may include some specialized Broadcasts, too)
   *
   * All participants make this call. So, there is no "send" or "recv"
   * distinction needed. Send and/or recv are determined by calling rank's
   * membership in respective Topologies, as well as requirements of underlying
   * hardware.
   *
   * All participants == {data_participants .U. results_participants}.
   *
   * \param[in] mcombineinfo	Struct of all params needed to perform operation
   * \return	XMI_SUCCESS or error codes
   */
  xmi_result_t XMI_Multicombine(xmi_context_t context,xmi_multicombine_t *mcombineinfo);

  /*****************************************************************************/
  /**
   * \defgroup datatype xmi non-contiguous datatype interface
   *
   * Some brief documentation on active message stuff ...
   * \{
   */
  /*****************************************************************************/

#define XMI_BYTE NULL

  /**
   * \brief Create a new type for noncontiguous transfers
   *
   * \todo provide example code
   *
   * \param[out] type Type identifier to be created
   */
  xmi_result_t XMI_Type_create (xmi_type_t * type);

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
  xmi_result_t XMI_Type_add_simple (xmi_type_t type,
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
  xmi_result_t XMI_Type_add_typed (xmi_type_t type,
                                   xmi_type_t subtype,
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
  xmi_result_t XMI_Type_complete (xmi_type_t type);

  /**
   * \brief Get the byte size of a completed type
   *
   * \param[in] type Type identifier to get size from
   */
  xmi_result_t XMI_Type_sizeof (xmi_type_t type);

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
  xmi_result_t XMI_Type_destroy (xmi_type_t type);

  /*
   * \brief Pack data from a non-contiguous buffer to a contiguous buffer
   *
   * \param[in] src_type   source data type
   * \param[in] src_offset starting offset of source data type
   * \param[in] src_addr   starting address of source buffer
   * \param[in] dst_addr   starting address of destination buffer
   * \param[in] dst_size   destination buffer size
   */
  xmi_result_t XMI_Type_pack_data (xmi_type_t src_type,
                                   size_t     src_offset,
                                   void     * src_addr,
                                   void     * dst_addr,
                                   size_t     dst_size);

  /*
   * \brief Unpack data from a contiguous buffer to a non-contiguous buffer
   *
   * \param[in] dst_type   destination data type
   * \param[in] dst_offset starting offset of destination data type
   * \param[in] dst_addr   starting address of destination buffer
   * \param[in] src_addr   starting address of source buffer
   * \param[in] src_size   source buffer size
   */
  xmi_result_t XMI_Type_unpack_data (xmi_type_t dst_type,
                                     size_t     dst_offset,
                                     void     * dst_addr,
                                     void     * src_addr,
                                     size_t     src_size);
  /** \} */ /* end of "datatype" group */

  extern int xmi_dt_shift[XMI_DT_COUNT];

  typedef union
  {
    xmi_dispatch_p2p_fn         p2p;
    xmi_dispatch_ambroadcast_fn ambroadcast;
    xmi_dispatch_amscatter_fn   amscatter;
    xmi_dispatch_amreduce_fn    amreduce;
    xmi_dispatch_multicast_fn   multicast;
    xmi_dispatch_manytomany_fn  manytomany;
    xmi_dispatch_multisync_fn   multisync;
  }xmi_dispatch_callback_fn;

  /*****************************************************************************/
  /**
   * \defgroup dispatch xmi dispatch interface
   *
   * Some brief documentation on dispatch stuff ...
   * \{
   */
  /*****************************************************************************/
  /**
   * \brief Initialize the dispatch functions for a dispatch id.
   *
   * This is a local, non-collective operation. There is no communication
   * between tasks.
   *
   * \param[in] context    XMI communication context
   * \param[in] dispatch   Dispatch identifier to initialize
   * \param[in] fn         Dispatch receive function
   * \param[in] cookie     Dispatch function cookie
   * \param[in] options    Dispatch registration assertions
   *
   */
  xmi_result_t XMI_Dispatch_set (xmi_context_t              context,
                                 size_t                     dispatch,
                                 xmi_dispatch_callback_fn   fn,
                                 void                     * cookie,
                                 xmi_send_hint_t            options);
  /** \} */ /* end of "dispatch" group */

#include "xmi_ext.h"


  /*****************************************************************************/
  /**
   * \defgroup configuration xmi configuration interface
   *
   * Some brief documentation on configuration stuff ...
   * \{
   */
  /*****************************************************************************/

  typedef unsigned int  xmi_intr_mask_t;  /**< ??? */
  typedef unsigned int  xmi_bool_t;       /**< ??? */
  typedef char *        xmi_user_key_t;   /**< ??? */
  typedef char *        xmi_user_value_t; /**< ??? */

  typedef struct
  {
    xmi_user_key_t    key;   /**< The configuration key   */
    xmi_user_value_t  value; /**< The configuration value */
  } xmi_user_config_t;

  /**
   * This enum contains ALL possible attributes for all hardware
   */
  typedef enum {
    /* Attribute            Init / Query / Update                                                                              */
    XMI_PROTOCOL_NAME=1, /**< IQ  : char *            : name of the protocol                                                   */
    XMI_RELIABILITY,     /**< IQ  : xmi_bool_t        : guaranteed reliability                                                 */
    XMI_ATTRIBUTES,      /**<  Q  : xmi_attribute_t[] : attributes on the platform terminated with NULL                        */
    XMI_TASK_ID,         /**<  Q  : size_t            : ID of this task                                                        */
    XMI_NUM_TASKS,       /**<  Q  : size_t            : total number of tasks                                                  */
    XMI_RECV_INTR_MASK,  /**<  QU : xmi_intr_mask_t   : receive interrupt mask                                                 */
    XMI_CHECK_PARAMS,    /**<  QU : xmi_bool_t        : check parameters                                                       */
    XMI_USER_KEYS,       /**<  Q  : xmi_user_key_t[]  : user-defined keys terminated with NULL                                 */
    XMI_USER_CONFIG,     /**<  QU : xmi_user_config_t : user-defined configuration key and value are shallow-copied for update */
  } xmi_attribute_name_t;

  typedef union
  {
    void              * pointer;
    char              * string;
    xmi_bool_t          boolean;
    xmi_attribute_name_t   * attributes;
    size_t              intval;
    xmi_intr_mask_t     interrupts;
    xmi_user_key_t    * keys;
    xmi_user_config_t * configurations;
  } xmi_attribute_value_t;

#define XMI_EXT_ATTR 1000 /**< starting value for extended attributes */

  /**
   * \brief General purpose configuration structure.
   */
  typedef struct
  {
    xmi_attribute_name_t  name;  /**< Attribute type */
    xmi_attribute_value_t value; /**< Attribute value */
  } xmi_configuration_t;


  /**
   * \brief Query the value of an attribute
   *
   * \param [in]     context        The XMI context
   * \param [in,out] configuration  The configuration attribute of interest
   *
   * \note
   * \returns
   *   XMI_SUCCESS
   *   XMI_ERR_CONTEXT
   *   XMI_ERR_ATTRIBUTE
   *   XMI_ERR_VALUE
   */
  xmi_result_t XMI_Configuration_query (xmi_context_t         context,
                                        xmi_configuration_t * configuration);

  /**
   * \brief Update the value of an attribute
   *
   * \param [in] context       The XMI context
   * \param [in] configuration The configuration attribute to update
   *
   * \note
   * \returns
   *   XMI_SUCCESS
   *   XMI_ERR_CONTEXT
   *   XMI_ERR_ATTRIBUTE
   *   XMI_ERR_VALUE
   */
  xmi_result_t XMI_Configuration_update (xmi_context_t         context,
                                         xmi_configuration_t * configuration);

  /**
   * \brief Provides the detailed description of the most recent xmi result.
   *
   * The "most recent xmi result" is specific to each thread.
   *
   * \note  XMI implementations may provide translated (i18n) text.
   *
   * \param[in] string Character array to write the descriptive text
   * \param[in] length Length of the character array
   *
   * \return Number of characters written into the array
   */
  size_t XMI_Error_text (char * string, size_t length);


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
  double XMI_Wtime();

  /**
   * \brief  Computes the smallest clock resolution theoretically possible
   * \note   This has the same definition as MPI_Wtick
   * \return The duration of a single timebase clock increment in seconds
   */
  double XMI_Wtick();

  /**
   * \brief  Returns the number of "cycles" elapsed on the calling processor.
   * \return Number of "cycles" since an arbitrary time in the past.
   *
   * "Cycles" could be any quickly and continuously increasing counter
   * if true cycles are unavailable.
   */
  unsigned long long XMI_Wtimebase();

  /** \} */ /* end of "Time" group */


  /*****************************************************************************/
  /**
   * \defgroup context xmi multi-context messaging interface
   *
   * Some brief documentation on context stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Initialize the XMI runtime for a client program
   *
   * A client program is any program that invokes any XMI function. This
   * includes applications, libraries, and other middleware. Some example
   * clienti names may include: "MPI", "UPC", "OpenSHMEM", and "ARMCI"
   *
   * A communication context must be created before any data transfer functions
   * may be invoked.
   *
   * \param[in]  name   XMI client unique name
   * \param[out] client XMI client handle
   */
  xmi_result_t XMI_Client_initialize (char         * name,
                                      xmi_client_t * client);

  /**
   * \brief Finalize the XMI runtime for a client program
   *
   * \warning It is \b illegal to invoke any XMI functions using the client
   *          handle from any thread after the finalize function.
   *
   * \param[in] client XMI client handle
   */
  xmi_result_t XMI_Client_finalize (xmi_client_t client);


  /**
   * \brief Create a new independent communication context for a client
   *
   * The context configuration attributes may include:
   * - Unique context name for this application, which is platform specific
   * - Context optimizations, such as shared memory, collective acceleration, etc
   *
   * Context creation is a local operation and does not involve communication or
   * syncronization with other tasks.
   *
   * \param[in]  client        Client handle
   * \param[in]  configuration List of configurable attributes and values
   * \param[in]  count         Number of configurations, may be zero
   * \param[out] context       XMI communication context
   */
  xmi_result_t XMI_Context_create (xmi_client_t           client,
                                   xmi_configuration_t    configuration[],
                                   size_t                 count,
                                   xmi_context_t        * context);

  /**
   * \brief Destroy an independent communication context
   *
   * \warning It is \b illegal to invoke any XMI functions using the
   *          communication context from any thread after the context is
   *          destroyed.
   *
   * \param[in] context XMI communication context
   */
  xmi_result_t XMI_Context_destroy (xmi_context_t context);

  /**
   * \brief Atomically post work to a context
   *
   * It is \b not required that the target context is locked, or otherwise
   * reserved, by an external atomic operation to ensure thread safety. The XMI
   * runtime will internally perform an atomic operation in order to post the
   * work to the context.
   *
   * The callback function will be invoked in the thread that advances the
   * communication context. There is no implicit completion notification provided
   * to the \em posting thread when the xmi client returns from the callback
   * function.  If the xmi client desires a completion notification in the
   * posting thread it must explicitly program such notifications, via the
   * XMI_Context_post() interface, from the target thread back to the origin
   * thread
   *
   * \note This is similar to the various DCMF_Handoff() interface(s)
   *
   * \param[in] context XMI communication context
   * \param[in] work_fn Event callback function to post to the context
   * \param[in] cookie  Opaque data pointer to pass to the event function
   */
  xmi_result_t XMI_Context_post (xmi_context_t        context,
                                 xmi_event_function   work_fn,
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
   * \see XMI_Context_lock()
   * \see XMI_Context_trylock()
   *
   * \todo Define return code, event bitmask ?
   *
   * \param[in] context XMI communication context
   * \param[in] maximum Maximum number of internal poll iterations
   */
  xmi_result_t XMI_Context_advance (xmi_context_t context, size_t maximum);

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
   *       always advanced together by any xmi client thread.  It is the
   *       responsibility of the xmi client to atomically lock the context set,
   *       perhaps by using the XMI_Context_lock() function on a designated
   *       \em leader context, and to manage the xmi client threads to ensure
   *       that only one thread ever advances the set of contexts.
   *
   * \todo Define return code, event bitmask ?
   * \todo Rename function to something better
   *
   * \see XMI_Context_lock()
   * \see XMI_Context_trylock()
   *
   * \param[in] context Array of XMI communication contexts
   * \param[in] count   Number of communication contexts
   * \param[in] maximum Maximum number of internal poll iterations on each context
   */
  xmi_result_t XMI_Context_multiadvance (xmi_context_t context[],
                                         size_t        count,
                                         size_t        maximum);

  /**
   * \brief Acquire an atomic lock on a communication context
   *
   * \warning This function will block until the lock is aquired.
   *
   * \param[in] context XMI communication context
   */
  xmi_result_t XMI_Context_lock (xmi_context_t context);

  /**
   * \brief Attempt to acquire an atomic lock on a communication context
   *
   * May return \c EAGAIN if the lock was not acquired.
   *
   * \param[in] context XMI communication context
   */
  xmi_result_t XMI_Context_trylock (xmi_context_t context);

  /**
   * \brief Release an atomic lock on a communication context
   *
   * \param[in] context XMI communication context
   */
  xmi_result_t XMI_Context_unlock (xmi_context_t context);

  /** \} */ /* end of "context" group */

#ifdef __cplusplus
};
#endif

#endif /* __xmi__h__ */
