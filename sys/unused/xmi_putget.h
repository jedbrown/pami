/**
 * \file xmi_putget.h
 * \brief XMI remote memory access interface
 * \defgroup rma Remote Memory Access data transfer operations
 * \{
 */
#ifndef __xmi_putget_h__
#define __xmi_putget_h__

#include "xmi.h"
#include "xmi_am.h"

typedef void * xmi_memregion_t;

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
extern xmi_memregion_t xmi_global_memregion;

/**
 * \brief Register a local memory region for one sided operations
 *
 * The local memregion may be transfered, via a send message, to a remote task
 * to allow the remote task to perform one-sided operations with this local
 * task
 *
 * \param[in]  context   XMI application context
 * \param[in]  address   Virtual address of memory region
 * \param[in]  bytes     Number of bytes to register
 * \param[out] memregion Memory region object. Can be NULL.
 */
xmi_result_t XMI_Memory_register (xmi_context_t     context,
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
xmi_result_t XMI_Memory_deregister (xmi_context_t   context,
                                    xmi_memregion_t memregion);

/**
 * \brief Provide one or more contiguous segments to transfer.
 *
 * \see XMI_Put_iterate
 * \see XMI_Get_iterate
 *
 * \param[in]     context       XMI application context
 * \param[in]     cookie        Event callback argument
 * \param[in,out] local_offset  Array of byte offsets from the local buffer
 * \param[in,out] remote_offset Array of byte offsets from the remote buffer
 * \param[in,out] bytes         Array of bytes to transfer
 * \param[in,out] segments      Number of segments available to be initialized
 *                              and the number of segments actually initialized
 *
 * \retval  0 Iterate complete, do not invoke the iterate callback again for
 *            this transfer
 * \retval !0 Iterate is not complete, the iterate callback must be invoked
 *            again for this transfer
 */
typedef size_t (*xmi_iterate_fn) (xmi_context_t   context,
                                  void          * cookie,
                                  size_t        * local_offset,
                                  size_t        * remote_offset,
                                  size_t        * bytes,
                                  size_t        * segments);

/**
 * \brief ???
 */
typedef struct {
  xmi_iterate_fn  function; /**< Non-contiguous iterate function */
} xmi_rma_iterate_t;

/**
 * \brief ???
 */
typedef struct {
  size_t          bytes;    /**< Data transfer size in bytes */
} xmi_rma_simple_t;

/**
 * \brief ???
 */
typedef struct {
  size_t          bytes;    /**< Data transfer size in bytes */
  xmi_data_type_t local;    /**< Data type of local buffer */
  xmi_data_type_t remote;   /**< Data type of remote buffer */
} xmi_rma_typed_t;





/**
 * \defgroup put Put data transfer operations
 *
 * ???
 */

/**
 * \brief Input parameters for the XMI put functions
 * \ingroup put
 *
 * \see XMI_Put
 * \see XMI_Put_typed
 **/
typedef struct {
  size_t                 task;      /**< Destination task */
  void                 * local;     /**< Local buffer virtual address */
  void                 * remote;    /**< Remote buffer virtual address */
  xmi_event_callback_t   send_done; /**< All local data has been sent */
  xmi_event_callback_t   recv_done; /**< All local data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
    xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Put() */
    xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Put_typed() */
  };
} xmi_put_t;

xmi_result_t XMI_Put (xmi_context_t context, xmi_put_t * parameters);
xmi_result_t XMI_Put_typed (xmi_context_t context, xmi_put_t * parameters);

/**
 * \brief Input parameters for the XMI get functions
 * \ingroup get
 *
 * \see XMI_Get
 * \see XMI_Get_typed
 **/
typedef struct {
  size_t                 task;      /**< Destination task */
  void                 * local;     /**< Local buffer virtual address */
  void                 * remote;    /**< Remote buffer virtual address */
  xmi_event_callback_t   done;      /**< All local data has been sent */
  xmi_event_callback_t   recv_done; /**< All local data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
    xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Get() */
    xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Get_typed() */
  };
} xmi_get_t;

xmi_result_t XMI_Get (xmi_context_t context, xmi_put_t * parameters);
xmi_result_t XMI_Get_typed (xmi_context_t context, xmi_put_t * parameters);

/*************************************************************************/
/*
 *   RDMA interface starts
 */
/*************************************************************************/

/**
 * \brief Input parameters for the XMI put functions
 * \ingroup put
 *
 * \see XMI_RPut
 * \see XMI_RPut_iterate
 * \see XMI_RPut_typed
 **/
typedef struct {
  size_t                 task;      /**< Destination task */
  void                 * local_va;  /**< Local buffer virtual address */
  xmi_memregion_t        local_mr;  /**< Local buffer memory region */
  void                 * remote_va; /**< Remote buffer virtual address */
  xmi_memregion_t        remote_mr; /**< Remote buffer memory region */
  xmi_event_callback_t   send_done; /**< All local data has been sent */
  xmi_event_callback_t   recv_done; /**< All local data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
    xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_RPut() */
    xmi_rma_iterate_t    iterate;   /**< Required, and only valid for, XMI_RPut_iterate() */
    xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_RPut_typed() */
  };
} xmi_rput_t;

/**
 * \brief Simple put operation for one-sided contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup put
 */
xmi_result_t XMI_RPut (xmi_context_t context, xmi_rput_t * parameters);

/**
 * \brief Put operation for callback-driven one-sided non-contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup put
 */
xmi_result_t XMI_RPut_iterate (xmi_context_t context, xmi_rput_t * parameters);

/**
 * \brief Put operation for data type specific one-sided data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup put
 */
xmi_result_t XMI_RPut_typed (xmi_context_t context, xmi_rput_t * parameters);





/**
 * \defgroup get Get data transfer operations
 *
 * ???
 */

/**
 * \brief Input parameters for the XMI get functions
 * \ingroup get
 *
 * \see XMI_RGet
 * \see XMI_RGet_iterate
 * \see XMI_RGet_typed
 **/
typedef struct {
  size_t                 task;      /**< Destination task */
  void                 * local_va;  /**< Local buffer virtual address */
  xmi_memregion_t        local_mr;  /**< Local buffer memory region */
  void                 * remote_va; /**< Remote buffer virtual address */
  xmi_memregion_t        remote_mr; /**< Remote buffer memory region */
  xmi_event_callback_t   done;      /**< All remote data has been received */
  void                 * cookie;    /**< Argument to \b all event callbacks */
  xmi_send_hint_t        hints;     /**< Hints for sending the message */
  union {
    xmi_rma_simple_t     simple;    /**< Required, and only valid for, XMI_Get() */
    xmi_rma_iterate_t    iterate;   /**< Required, and only valid for, XMI_Get_iterate() */
    xmi_rma_typed_t      typed;     /**< Required, and only valid for, XMI_Get_typed() */
  };
} xmi_rget_t;


/**
 * \brief Simple get operation for one-sided contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup get
 */
xmi_result_t XMI_RGet (xmi_context_t context, xmi_rget_t * parameters);

/**
 * \brief Get operation for callback-driven one-sided non-contiguous data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup get
 */
xmi_result_t XMI_RGet_iterate (xmi_context_t context, xmi_rget_t * parameters);

/**
 * \brief Get operation for data type specific one-sided data transfer.
 *
 * \param[in] context    XMI application context
 * \param[in] parameters Input parameters structure
 *
 * \ingroup get
 */
xmi_result_t XMI_RGet_typed (xmi_context_t context, xmi_rget_t * parameters);

/**
 * \}
 * \addtogroup rma
 *
 * More documentation for rma stuff....
 */


#endif
