/**
 * \file sys/xmi_p2p.h
 * \brief messaging interface
 * \todo API style - all lowercase types? mixed-case? all uppercase?
 * \todo API style - abbreviated or explicit method/type/variable names ?
 * \todo API style - define parameter list convention. Most common parameters first?
 */
#ifndef __xmi_p2p_h__
#define __xmi_p2p_h__

#include <stdlib.h>
#include <stdint.h>
#include "xmi_types.h"
#include "xmi_misc.h"

#ifdef __cplusplus
extern "C"
{
#endif
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
    xmi_dispatch_t         dispatch; /**< Dispatch identifier */
    xmi_send_hint_t        hints;    /**< Hints for sending the message */
    size_t                 task;     /**< Destination task */
    void                 * cookie;   /**< Argument to \b all event callbacks */
    struct
    {
      size_t               bytes;    /**< Header buffer size in bytes */
      void               * addr;     /**< Header buffer address */
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
   */
  typedef void (*xmi_dispatch_p2p_fn) (
    xmi_context_t        context,      /**< IN: XMI context */
    void               * cookie,       /**< IN: dispatch cookie */
    size_t               task,         /**< IN: source task */
    void               * header_addr,  /**< IN: header address */
    size_t               header_size,  /**< IN: header size */
    void               * pipe_addr,    /**< IN: address of XMI pipe buffer */
    size_t               pipe_size,    /**< IN: size of XMI pipe buffer */
    xmi_recv_t         * recv);        /**< OUT: receive message structure */

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


#ifdef __cplusplus
};
#endif

/*
 * astyle info    http://astyle.sourceforge.net
 *
 * astyle options --style=gnu --indent=spaces=2 --indent-classes
 * astyle options --indent-switches --indent-namespaces --break-blocks
 * astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
 */

#endif /* __xmi_p2p_h__ */
