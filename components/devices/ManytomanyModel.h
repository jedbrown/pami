/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/ManytomanyModel.h
 * \brief ???
 */

#ifndef __components_devices_ManytomanyModel_h__
#define __components_devices_ManytomanyModel_h__

#include <pami.h>
#include "algorithms/ccmi.h"  //for PAMI_Callback_t...probably move this at some point
#include "M2MPipeWorkQueue.h"
#include "Topology.h"

/**
 * \brief Manytomany interfaces.
 *
 * A multi* operation allows many message passing transactions to
 * be performed in the same call to amortize software overheads.
 *
 * A multi* Manytomany sends different offsets from the same buffer
 * to a list of processors
 *
 * Each multi* operation carries a connection id to identify the
 * data on the reciever.
 *
 * In a multi* operation the cores are typically involved at the
 * end-points. Moreover the processor lists can be created on the fly.
 */


/**
 * \brief Sub-structure used to represent a vectored buffer for many-to-many.
 *
 * This is used for both send and receive parameters.
 *
 */
typedef struct
{
  PAMI::M2MPipeWorkQueue *buffer;       /**< M2M PWQ used for data                    */
  PAMI::Topology         *participants; /**< Tasks that are vectored in buffer        */
} pami_manytomanybuf_t;

/**
 * \brief Structure of parameters used to initiate a ManyToMany
 *
 * The taskIndex parameter may be transmitted to the receiver for use by cb_recv
 * to optimized indexing into the recv parameter arrays (lengths and offsets).
 * It may be NULL, in which case the receiver will lookup each sender rank in the
 * recv topology.  It may be a single value if the index is the same on all 
 * destinations (alltoall).  It may be 1 value per destination if a more complicated 
 * manytomany is being done. 
 */
typedef struct
{
  size_t               client;        /**< client to operate within */
  size_t               context;       /**< primary context to operate within */
  pami_callback_t      cb_done;       /**< User's send completion callback */
  unsigned             connection_id; /**< differentiate data streams */
  unsigned             roles;         /**< bitmap of roles to perform */
  size_t              *taskIndex;     /**< Hint - index of send in recv parameters.
                                           May be NULL */
  size_t               num_index;     /**< Number of entries in "taskIndex".
                                           Should be 0, 1 or send.participants->size() */
  pami_manytomanybuf_t send;          /**< send data parameters */
  const pami_quad_t   *msginfo;       /**< A extra info field to be sent with the message.
                                           This might include information about
                                           the data being sent. */
  unsigned             msgcount;      /**< info count*/
} pami_manytomany_t;

/**
 * \brief Receive callback for Manytomany
 *
 * ManyToMany must register a Receive Callback to receive data independently
 * of calling manytomany() to send data.
 *
 * All senders of metadata for a given connection_id (instance) must send
 * the same, or at least compatible, metadata such that all would return the
 * same receive parameters (**recv). Note, the recv callback will be invoked
 * only once per connection_id. The first sender message to arrive will invoke
 * the callback and get recv params for ALL other senders in the instance.
 *
 * The myIndex parameter is the receiving task's index in the recv arrays
 * (lengths and offsets) and is used by the manytomany as an optimization
 * for handling local data reception.
 *
 * \param[in]  arg       Client Data
 * \param[in]  conn_id   Instance ID
 * \param[in]  msginfo   Pointer to metadata, if any, in message header.
 * \param[in]  msgcount  Number of pami_quad_ts of metadata.
 * \param[out] recv      Receive parameters for this connection (instance)
 * \param[out] myIndex   Hint - index of Recv Task in the receive parameters.
 * \param[out] cb_done   Receive completion callback
 *
 * \return  Request object opaque storage for message.
 */
typedef void (*pami_dispatch_manytomany_fn)(void                  *arg,
                                            unsigned               conn_id,
                                            pami_quad_t           *msginfo,
                                            unsigned               msgcount,
                                            pami_manytomanybuf_t **recv,
                                            size_t                *myIndex,
                                            pami_callback_t       *cb_done);




namespace PAMI
{
  namespace Device
  {
    namespace Interface
    {
      template <class T_Model, class T_Device, unsigned T_StateBytes>
      class ManytomanyModel
      {
        public:
          /// \param[in] device                Manytomany device reference
          ManytomanyModel (T_Device & device, pami_result_t &status)
          {
            COMPILE_TIME_ASSERT(T_Model::sizeof_msg == T_StateBytes);
            status = PAMI_SUCCESS;
          };
          ~ManytomanyModel () {};
          inline pami_result_t postManytomany(uint8_t (&state)[T_StateBytes],
                                              pami_manytomany_t *m2minfo,
                                              void              *devinfo = NULL);

      };

      template <class T_Model, class T_Device, unsigned T_StateBytes>
      pami_result_t ManytomanyModel<T_Model, T_Device, T_StateBytes>::postManytomany(uint8_t (&state)[T_StateBytes],
                                                                                     pami_manytomany_t *m2minfo,
                                                                                     void              *devinfo)

      {
        return static_cast<T_Model*>(this)->postManytomany_impl(state, m2minfo, devinfo);
      }


      ///
      /// \brief Add an active message dispatch to the multicast model.
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      ///
      /// \param T_Model   Manytomany model template class
      ///
      /// \see Manytomany::Model
      ///
      /** \todo We probably need client and context passed to the model */
      template <class T_Model, class T_Device, unsigned T_StateBytes>
      class AMManytomanyModel : public ManytomanyModel<T_Model, T_Device, T_StateBytes>
      {
        public:
          AMManytomanyModel (T_Device &device, pami_result_t &status) :
              ManytomanyModel<T_Model, T_Device, T_StateBytes> (device, status)
          {
          };
          ~AMManytomanyModel ()
          {
          };
          inline pami_result_t
          registerManytomanyRecvFunction(int dispatch_id,
                                         pami_dispatch_manytomany_fn recv_func,
                                         void  *async_arg);
      }; // class AMManytomanyModel
      template <class T_Model, class T_Device, unsigned T_StateBytes>
      pami_result_t AMManytomanyModel<T_Model, T_Device, T_StateBytes>::registerManytomanyRecvFunction (int                          dispatch_id,
          pami_dispatch_manytomany_fn  recv_func,
          void                        *async_arg)
      {
        return static_cast<T_Model*>(this)->registerManytomanyRecvFunction_impl (dispatch_id, recv_func, async_arg);
      }
    };
  };
};
#endif // __components_device_manytomanymodel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
