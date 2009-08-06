/**
 * \file xmi_send.h
 * \brief messaging interface
 */
#ifndef __xmi_send_h__
#define __xmi_send_h__

#include "xmi.h"

/* ************************************************************************* */
/* ************************************************************************* */
/*                                                                           */
/*                                XMI_Send ()                                */
/*                                                                           */
/* ************************************************************************* */
/* ************************************************************************* */

#if 0
typedef void (*xmi_callback_fn) (void * arg, xmi_error_t error);

typedef struct xmi_callback
{
  xmi_callback_fn   fn;  /**< Callback function pointer */
  void            * arg; /**< Callback function parameter */
} xmi_callback_t;

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
  xmi_callback_t    callback;    /**< Callback to invoke when the recv is complete */
} xmi_recv_t;

/**
 * \brief Dispatch function signature for receives.
 *
 * \param[in]  clientdata Registered clientdata
 * \param[in]  rank       Global rank of the sender
 * \param[in]  metadata   Metadata from the origin rank for this transfer
 * \param[in]  bytes      Number of incoming bytes for this transfer
 * \param[in]  source     System source buffer address to be copied into application buffer, may be \c NULL
 * \param[out] recv       Async receive information
 */
typedef void (*xmi_recv_fn) (void                 * clientdata,
                             size_t                 rank,
                             const xmi_metadata_t * metadata,
                             size_t                 bytes,
                             const void           * source,
                             xmi_recv_t           * recv);

typedef enum
{
  XMI_OPTION_RDMA = 0x01,
  XMI_OPTION_SMP  = 0x02
} xmi_option_t;


/**
 * \brief Initialize the dispatch functions for a dispatch id.
 *
 * This is a local, non-collective operation. There is no communication
 * between ranks.
 *
 * \param[in] context    XMI application context
 * \param[in] dispatch   Dispatch identifier to initialize
 * \param[in] fn         Dispatch receive function
 * \param[in] clientdata Dispatch function clientdata
 * \param[in] options    Dispatch registration options - bitmask?
 *
 */
xmi_result_t XMI_Dispatch_set (xmi_context_t     * context,
                               xmi_dispatch_t      dispatch,
                               xmi_recv_fn         fn,
                               void              * clientdata,
                               xmi_option_t        options);

typedef struct xmi_send_packet
{
  xmi_callback_t    remote;    /**< Transfer event callback, Source data buffer has been received */
  size_t            rank;      /**< Global rank of the remote process */
  void            * source;    /**< Source data buffer address */
  size_t            bytes;     /**< Number of bytes to transfer from the source data buffer */
  xmi_dispatch_t    dispatch;  /**< Dispatch identifier for this transfer */
  xmi_metadata_t    metadata;  /**< Application metadata information */
  xmi_hint_t        hints;     /**< Send hints which may be ignored by the runtime. */
} xmi_send_packet_t;

typedef struct xmi_send
{
  xmi_callback_t     local;    /**< Transfer event callback, Source data buffer has been sent and may be reused */
  xmi_send_packet_t  send;     /**< Common send parameters */
} xmi_send_t;

/**
 * Send a single packet. The source buffer is immediately available for reuse
 * upon the return of this function.
 *
 * \param[in]  context    XMI application context
 * \param[in]  parameters XMI latency send parameters
 */
xmi_result_t XMI_Send_packet (xmi_context_t     * context,
                              xmi_send_packet_t * parameters);

/**
 * \brief Send a source data buffer to a remote rank.
 *
 * \example xmi_send.c
 *
 * \param[in]  context    XMI application context
 * \param[in]  parameters XMI send parameters
 */
xmi_result_t XMI_Send (xmi_context_t * context,
                       xmi_send_t    * parameters);
#endif

#endif /* __xmi_send_h__ */
