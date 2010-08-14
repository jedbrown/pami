/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/MulticastModel.h
 * \brief ???
 */

#ifndef __components_devices_MulticastModel_h__
#define __components_devices_MulticastModel_h__

#include <sys/uio.h>
#include <pami.h>
#include "util/common.h"

/**
 * \brief Multisend interfaces.
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
   * \brief The new structure to pass parameters for the multisend multicast operation.
   *
   * The PAMI_Multicast_t object is re-useable immediately, but objects referred to
   * (src, etc) cannot be re-used until cb_done.
   *
   * client and context types should not be pami_client_t and pami_context_t,
   * the implementations need offsets/index and can't access the opaque types
   * to get that because of circular dependencies.
   */
  typedef struct
  {
    size_t               client;	   /**< client to operate within */
    size_t               context;	   /**< primary context to operate within */
    size_t               dispatch;         /**< Dispatch identifier */
    pami_callback_t       cb_done;          /**< Completion callback */
    unsigned             connection_id;    /**< A connection is a distinct stream of
                                              traffic. The connection id identifies the
                                              connection */
    unsigned             roles;            /**< bitmap of roles to perform */
    size_t               bytes;            /**< size of the message*/
    pami_pipeworkqueue_t *src;              /**< source buffer */
    pami_topology_t      *src_participants; /**< root */
    pami_pipeworkqueue_t *dst;              /**< dest buffer (ignored for one-sided) */
    pami_topology_t      *dst_participants; /**< destinations to multicast to*/
    const pami_quad_t    *msginfo;	       /**< A extra info field to be sent with the message.
                                                  This might include information about
                                                  the data being sent, for one-sided. */
    unsigned            msgcount;          /**< info count*/
  } pami_multicast_t;

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
   * \param[in] root        Sending task
   * \param[in] sndlen      Length of data sent
   * \param[in] clientdata  Opaque arg
   * \param[out] rcvlen     Length of data to receive
   * \param[out] rcvpwq     Where to put recv data
   * \param[out] cb_done    Completion callback to invoke when data received
   * \return   void
   */
  typedef void (*pami_dispatch_multicast_fn)(const pami_quad_t        *msginfo,
                                            unsigned              msgcount,
                                            unsigned              connection_id,
                                            size_t                root,
                                            size_t                sndlen,
                                            void                 *clientdata,
                                            size_t               *rcvlen,
                                            pami_pipeworkqueue_t **rcvpwq,
                                            pami_callback_t       *cb_done);



namespace PAMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model   Multicast model template class
      ///
      /// \see Multicast::Model
      ///
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      class MulticastModel
      {
      public:
        MulticastModel (T_Device &device, pami_result_t &status)
          {
            COMPILE_TIME_ASSERT(T_Model::sizeof_msg <= T_StateBytes);
            status = PAMI_SUCCESS;
          };
        ~MulticastModel ()
          {
          };
        inline pami_result_t postMulticast(uint8_t (&state)[T_StateBytes],
                                           pami_multicast_t *mcast,
                                           void             *devinfo=NULL);
      }; // class MulticastModel

      template <class T_Model,class T_Device,unsigned T_StateBytes>
      pami_result_t MulticastModel<T_Model,T_Device, T_StateBytes>::postMulticast(uint8_t (&state)[T_StateBytes],
                                                                                  pami_multicast_t *mcast,
                                                                                  void             *devinfo)
      {
        return static_cast<T_Model*>(this)->postMulticast_impl(state, mcast, devinfo);
      }

      ///
      /// \brief Add an active message dispatch to the multicast model.
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      ///
      /// \param T_Model   Multicast model template class
      ///
      /// \see Multicast::Model
      ///
      /** \todo We probably need client and context passed to the model */
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      class AMMulticastModel : public MulticastModel<T_Model,T_Device,T_StateBytes>
      {
      public:
        AMMulticastModel (T_Device &device, pami_result_t &status) :
          MulticastModel<T_Model,T_Device,T_StateBytes> (device, status)
          {
          };
        ~AMMulticastModel ()
          {
          };
        inline pami_result_t registerMcastRecvFunction (int                         dispatch_id,
                                                        pami_dispatch_multicast_fn  recv_func,
                                                        void                       *async_arg);
      }; // class AMMulticastModel
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      pami_result_t AMMulticastModel<T_Model,T_Device,
                                    T_StateBytes>::registerMcastRecvFunction (int                         dispatch_id,
                                                                              pami_dispatch_multicast_fn  recv_func,
                                                                              void                       *async_arg)
      {
        return static_cast<T_Model*>(this)->registerMcastRecvFunction_impl (dispatch_id,
                                                                            recv_func,
                                                                            async_arg);
      }
    }; // namespace Interface
  }; // namespace Device
}; // namespace PAMI
#endif // __components_devices_MulticastModel_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
