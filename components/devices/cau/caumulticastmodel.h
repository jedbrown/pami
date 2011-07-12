/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumulticastmodel_h__
#define __components_devices_cau_caumulticastmodel_h__

#include <pami.h>
#include "components/devices/MulticastModel.h"
#include "components/devices/cau/caumessage.h"

#ifdef TRACE
#undef TRACE
#define TRACE(x) //fprintf x
#else
#define TRACE(x) //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class CAUMulticastModel :
      public Interface::MulticastModel<CAUMulticastModel<T_Device, T_Message>,
                                       T_Device,
                                       sizeof(T_Message)>
    {
      public:
      static const size_t mcast_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg              = sizeof(T_Message);
      static const bool   is_active_message       = false;

      static void * cau_mcast_handler(lapi_handle_t  *hndl,
                                      void           *uhdr,
                                      uint           *uhdr_len,
                                      ulong          *retinfo,
                                      compl_hndlr_t **comp_h,
                                      void          **uinfo)
        {
          typename T_Message::Header      *hdr   = (typename T_Message::Header*)uhdr;
          int                              did   = hdr->dispatch_id;
          int                              gid   = hdr->geometry_id;
          int                              seqno = hdr->seqno;
          CAUMulticastModel               *mc    = (CAUMulticastModel*) CAUDevice::getClientData(did);
          PAMI_GEOMETRY_CLASS             *g     = (PAMI_GEOMETRY_CLASS*)mc->_device.geometrymap(gid);
          CAUGeometryInfo                 *gi    = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::GKEY_MCAST_CLASSROUTEID);
          T_Message                       *msg   = (T_Message *)gi->_postedBcast.find(seqno);
          lapi_return_info_t              *ri    = (lapi_return_info_t *) retinfo;
          PAMI_assert(ri->udata_one_pkt_ptr);

          TRACE((stderr, "Packet Arrival, seqno=%d msg=%p\n", seqno, msg));
          if(msg == NULL)  // Not Found, insdert into ue queue
          {
            msg = (T_Message*)gi->_ueBcast.find(seqno);
            if(msg == NULL)
            {
              TRACE((stderr, "   is a first packet seqno=%d msg=%p\n", seqno, msg));              
              mc->_device.allocMessage(&msg); 
              new(msg) T_Message(&mc->_device,gi,did,seqno); // Construct, but don't init this message
              gi->_ueBcast.pushTail((MatchQueueElem*)msg);
            }
          }
          // Copy message data into the packet
          // We can optimize this to avoid the copy, but care is needed to ensure pipelining is correct
          typename T_Message::IncomingPacket *ipacket =
            (typename T_Message::IncomingPacket *)mc->_device._pkt_allocator.allocateObject();
          msg->_packetQueue.enqueue(ipacket);
          memcpy(&ipacket->_data[0],ri->udata_one_pkt_ptr, hdr->pktsize);
          ipacket->_size=hdr->pktsize;

          msg->advanceNonRoot();

          // Lapi return parameters
          *comp_h       = NULL;
          ri->ret_flags = LAPI_LOCAL_STATE;
          ri->ctl_flags = LAPI_BURY_MSG;
          return NULL;
        }


      // This function is executed by the generic device, and will execute
      // as long as do_reduce returns PAMI_EAGAIN.
      static pami_result_t do_bcast (pami_context_t context, void *cookie)
        {
          // Capture all message information needed to free
          // before the message storage may be freed as a result of
          // a message cb_done
          T_Message              *m      = (T_Message*) cookie;
          Generic::GenericThread *t      = m->_workfcn;
          T_Device               *device = (T_Device *)m->_device;

          // Advance the message
          pami_result_t           result = m->advance();
          if(result == PAMI_SUCCESS)
            device->freeWork(t);

          return result;
        }
      
      CAUMulticastModel (T_Device & device, pami_result_t &status) :
        Interface::MulticastModel < CAUMulticastModel<T_Device, T_Message>,
                                    T_Device,
                                    sizeof(T_Message) > (device, status),
        _device(device)
          {
            TRACE((stderr, "CAUMulticastModel:  Registering Dispatch Handler:  %p\n",cau_mcast_handler));
            _dispatch_mcast_id = _device.registerSyncDispatch(cau_mcast_handler, this);
          };
      
        inline pami_result_t postMulticast_impl (uint8_t (&state)[mcast_model_state_bytes],
						 size_t            client,
						 size_t            context,
                                                 pami_multicast_t *mcast,
                                                 void             *devinfo)
          {
            CAUGeometryInfo  *gi             = (CAUGeometryInfo *)devinfo;
            T_Message        *msg, *earlymsg = (T_Message*)gi->_ueBcast.findAndDelete(gi->_seqnoBcast);
            msg = new(state) T_Message(&_device,gi,_dispatch_mcast_id);
            TRACE((stderr, "Posting Multicast, seqno=%d, earlymsg=%p\n", gi->_seqnoBcast, earlymsg));

            msg->init(mcast);
            if(earlymsg)
            {
              // Copy early arrival packet pointers to new message
              int i,sz = earlymsg->_packetQueue.size();
              TRACE((stderr, "Early Arrival found in post, %d packets\n", sz));
              for(i=0; i<sz; i++)
                msg->_packetQueue.enqueue(earlymsg->_packetQueue.dequeue());
              _device.freeMessage(earlymsg);
            }
            pami_result_t rc = msg->advance();
            if(rc == PAMI_EAGAIN)
            {
              gi->_postedBcast.pushTail((MatchQueueElem*)msg);
              msg->_isPosted= true;
              msg->_workfcn = _device.postWork(do_bcast, msg);
            }
            return PAMI_SUCCESS;
          }
        T_Device                                                   &_device;
        int                                                         _dispatch_mcast_id;        
    };
  };
};
#endif // __components_devices_cau_caumulticastmodel_h__
