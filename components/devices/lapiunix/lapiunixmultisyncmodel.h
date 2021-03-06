/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixmultisyncmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmultisyncmodel_h__
#define __components_devices_lapiunix_lapiunixmultisyncmodel_h__

#include <pami.h>
#include "components/devices/MultisyncModel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class LAPIMultisyncModel :
      public Interface::MultisyncModel<LAPIMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>
    {
    public:
      static const size_t msync_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      LAPIMultisyncModel (T_Device &device, pami_result_t &status) :
        Interface::MultisyncModel<LAPIMultisyncModel<T_Device, T_Message>,T_Device,sizeof(T_Message)>(device,status),
        _device(device)
        {
          status = PAMI_SUCCESS;
        }

      pami_result_t postMultisync (uint8_t         (&state)[msync_model_state_bytes],
				   size_t           client,
				   size_t           context,
				   pami_multisync_t *msync)
        {
          pami_result_t      rc     = PAMI_SUCCESS;
          LAPIMsyncMessage *msg     = (LAPIMsyncMessage *) state;
          PAMI::Topology   *topo    = (PAMI::Topology *)msync->participants;
          pami_task_t      *ranks_h = NULL;
          pami_task_t      *ranks   = NULL;
          size_t           size    = topo->size();
          size_t          myidx    = topo->rank2Index(__global.mapping.task());
          topo->rankList(&ranks_h);
          ranks                    = ranks_h;

          msg->_cb_done            = msync->cb_done;
          msg->_r_flag             = 0;
          msg->_total              = size;
          msg->_p2p_msg._connection_id = msync->connection_id;
          // Kick off the communication
          lapi_xfer_t xfer_struct;
          xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
          xfer_struct.Am.flags     = 0;
          xfer_struct.Am.hdr_hdl   = (lapi_long_t)3L;
          xfer_struct.Am.uhdr      = (void *) &msg->_p2p_msg;
          xfer_struct.Am.uhdr_len  = sizeof(msg->_p2p_msg);
          xfer_struct.Am.udata     = NULL;
          xfer_struct.Am.udata_len = 0;
          _device.enqueue(msg);

          for(size_t i=0;i<size;i++)
              {
                xfer_struct.Am.tgt       = ranks[i];
                CheckLapiRC(lapi_xfer(_device._lapi_handle, &xfer_struct));
              }
          return rc;
        }

      T_Device                     &_device;
      size_t                        _dispatch_id;
      pami_olddispatch_multicast_fn  _cb_async_head;
      void                         *_async_arg;
    };
  };
};
#endif // __components_devices_mpi_mpimultisyncmodel_h__
