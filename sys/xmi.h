/**
 * \file xmi.h
 * \brief messaging interface
 */
#ifndef __xmi_h__
#define __xmi_h__

/**
 * \todo Define common return type.
 *
 * \todo Determine if need support for a separate "configure" function, or if
 *       configuration is only done during xmi context initialization
 *
 * \todo API style - all lowercase types? mixed-case? all uppercase?
 * \todo API style - abbreviated or explicit method/type/variable names ?
 * \todo API style - use MPI convention to name function pointer typedefs
 *       and function pointer parameters. see -> https://svn.mpi-forum.org/trac/mpi-forum-web/ticket/7
 *
 * \todo Define parameter list convention. Most common parameters first?
 *
 * \todo Define critical section interface
 *
 * \todo How to target specific devices? As it is below, BGQ would need to
 *       advance two xmi contexts - one for the torus and another for shmem.
 */

/**
 *
 * 1. Set the device/fabric/network for pt2pt (dma, shmem, etc)
 * 2. Set the sync and async recv callbacks and clientdata
 */
xmi_result_t XMI_Initialize (xmi_context_t * context);

xmi_result_t XMI_Finalize (xmi_context_t * context);


/**
 * \brief Singular advance of the progress engine.
 *
 * May complete zero, one, or more outbound transfers. May
 * invoke dispatch handlers for incoming transfers.
 *
 * If interrupt mode is enabled it is not neccesary to explicitly
 * advance the progress engine.
 *
 * \todo Define return code - any events occured?
 */
xmi_result_t XMI_Advance (xmi_context_t * context);




/* ************************************************************************* */
/* ************************************************************************* */
/*                                                                           */
/*                                XMI_Send ()                                */
/*                                                                           */
/* ************************************************************************* */
/* ************************************************************************* */

typedef struct xmi_metadata
{
  void     * source;    /**< Are there alignment requirements for metadata? */
  size_t     bytes;
  uint64_t   options;   /**< bitmask? Options may include for whether to checksum the metadata or not... */
} xmi_metadata_t;


typedef struct xmi_recv
{
  void            * destination; /**< Destination recv buffer address */
  size_t            bytes;       /**< Number of bytes to receive */
  xmi_callback_fn   callback;    /**< Callback function to invoke when the recv is complete */
  void            * clientdata;  /**< Callback function clientdata */
} xmi_recv_t;


typedef void (*xmi_callback_fn) (void * clientdata);

typedef struct xmi_callback
{
  xmi_callback_fn   fn;  /**< Callback function pointer */
  void            * arg; /**< Callback function parameter */
} xmi_callback_t;

/**
 * \brief Dispatch function signature for latency optimized receives.
 *
 * \param[in]  clientdata Registered clientdata
 * \param[in]  metadata   Metadata from the origin rank for this transfer
 * \param[in]  bytes      Number of incoming bytes for this transfer
 * \param[in]  source     System source buffer address to be copied into application buffer
 */
typedef void (*xmi_recv_fn) (void                 * clientdata,
                             const xmi_metadata_t * metadata,
                             size_t                 bytes,
                             const void           * source);


/**
 * \brief Dispatch function signature for asynchronous receives.
 *
 * \param[in]  clientdata Registered clientdata
 * \param[in]  metadata   Metadata from the origin rank for this transfer
 * \param[in]  bytes      Number of incoming bytes for this transfer
 * \param[out] recv       Async receive information
 */
typedef void (*xmi_recv_async_fn) (void                 * clientdata,
                                   const xmi_metadata_t * metadata,
                                   size_t                 bytes,
                                   xmi_recv_t           * recv);

typedef struct xmi_send
{
  xmi_callback_t    callback;  /**< Transfer completion callback */
  size_t            rank;      /**< Global rank of the remote process */
  void            * source;    /**< Source data buffer address */
  size_t            bytes;     /**< Number of bytes to transfer from the source data buffer */
  xmi_metadata_t    metadata;  /**< Application metadata information */
} xmi_send_t;

/**
 * \param[in]  context    XMI context
 * \param[in]  parameters XMI send parameters
 */
xmi_result_t XMI_Send (xmi_context_t * context,
                       xmi_send_t    * parameters);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */


/**
 * \brief Retrieve the configuration structure for an XMI context
 *
 * Specifies XMI attributes such as interrupt mode (default is enabled on Power
 * and disabled on Blue Gene). Configuration structure is used as input to the
 * XMI_Initialize() function.
 *
 * This function may be invoked before XMI_Initialize()
 *
 * \todo Define configuration attriutes (interrupt mode, send/recv buffer space, etc)
 *
 * \param[in]     context       Pointer to an initialized XMI context, If
 *                              \c NULL then the default configuration is returned
 * \param[in,out] configuration Pointer to the configuration structure to update
 */
xmi_result_t XMI_Configuration (xmi_context_t       * context,
                                xmi_configuration_t * configuration);

xmi_result_t XMI_Configure (...);



#endif /* __xmi_h__ */
