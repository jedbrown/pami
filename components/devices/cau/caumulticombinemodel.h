/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumulticombinemodel.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumulticombinemodel_h__
#define __components_devices_cau_caumulticombinemodel_h__

#include <pami.h>
#include "components/devices/MulticombineModel.h"
#include "components/devices/cau/caumessage.h"

#ifdef TRACE
#undef TRACE
#define TRACE(x) //fprintf x
#else
#define TRACE(x) //fprintf x
#endif

extern int pami_dt_sizes[];

namespace PAMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
    class CAUMulticombineModel :
      public Interface::MulticombineModel<CAUMulticombineModel<T_Device, T_Message>,
                                          T_Device,
                                          sizeof(T_Message)>
    {
      public:
      static const size_t multicombine_model_state_bytes = sizeof(T_Message);
      static const size_t sizeof_msg                     = sizeof(T_Message);

      // The CAU Multicast Handler
      // This is used to handle incoming broadcasts from a
      // multicombine.  There is no unexpected message handling because
      // all tasks are syncronized during the reduction phase of the multicombine
      // In this case, we just copy the packet into the message.
      // The next generic device advance will pick up and process the packet
      // This will only be called on tasks that are non-root (of the CAU network)
      // because the root multicasts the data
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
          CAUMulticombineModel            *mc    = (CAUMulticombineModel*) CAUDevice::getClientData(did);
          PAMI_GEOMETRY_CLASS             *g     = (PAMI_GEOMETRY_CLASS*)mc->_device.geometrymap(gid);
          CAUGeometryInfo                 *gi    = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);
          T_Message                       *msg   = (T_Message *)gi->_postedRed.find(seqno);
          lapi_return_info_t              *ri    = (lapi_return_info_t *) retinfo;
          memcpy(msg->_resultPkt, ri->udata_one_pkt_ptr,hdr->pktsize);
          msg->_resultPktBytes = hdr->pktsize;

          TRACE((stderr, "MCombine:  cau_mcast_handler:  seqno=%d rpb=%d\n",
                 seqno, msg->_resultPktBytes));

          msg->advanceNonRoot();
          
          return NULL;
        }

      // The reduction handler
      // This handler is only called on the root of the reduction (cau root)
      // This code first searches the posted queue for any CAU reduction messages
      // with the correct sequence number
      // If the message is found, the message has been posted and we copy the
      // incoming cau packet into the message, to be picked up and processed
      // in the next generic device advance.
      // If the message is not found, we allocate a message and push it into the
      // unexpected queue.      
      static void * cau_red_handler(lapi_handle_t  *hndl,
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
          CAUMulticombineModel            *mc    = (CAUMulticombineModel*) CAUDevice::getClientData(did);
          PAMI_GEOMETRY_CLASS             *g     = (PAMI_GEOMETRY_CLASS*)mc->_device.geometrymap(gid);
          CAUGeometryInfo                 *gi    = (CAUGeometryInfo*) g->getKey(PAMI::Geometry::GKEY_MCOMB_CLASSROUTEID);
          T_Message                       *msg   = (T_Message *)gi->_postedRed.find(seqno);
          lapi_return_info_t              *ri    = (lapi_return_info_t *) retinfo;
          
          PAMI_assert(ri->udata_one_pkt_ptr);
          if(msg == NULL)  // Not Found, insdert into ue queue
          {
            msg = (T_Message *)mc->_msg_allocator.allocateObject();
            new(msg) T_Message(&mc->_device,gi,did,gid); // Construct, but don't init this message
            gi->_ueRed.pushTail((MatchQueueElem*)msg);
          }
          // In either case, copy the packet and packet size into the message
          msg->_reducePktBytes = (unsigned)hdr->pktsize;
          memcpy(msg->_reducePkt, ri->udata_one_pkt_ptr, hdr->pktsize);

          TRACE((stderr, "MCombine:  cau_red_handler: h.psz=%d h.msz=%d sno=%d rpb=%d did=%d, gid=%d\n",
                 hdr->pktsize, hdr->msgsize, seqno, msg->_reducePktBytes, did, gid));

          msg->advanceRoot();
          
          // Lapi return parameters
          *comp_h       = NULL;
          ri->ret_flags = LAPI_SEND_REPLY;
          ri->ctl_flags = LAPI_BURY_MSG;
          return NULL;
        }

      // This function is executed by the generic device, and will execute
      // as long as do_reduce returns PAMI_EAGAIN.
      static pami_result_t do_reduce (pami_context_t context, void *cookie)
        {
          // Capture all message information needed to free
          // before the message storage may be freed as a result of
          // a message cb_done
          T_Message              *m      = (T_Message*) cookie;
          Generic::GenericThread *t      = m->_workfcn;
          T_Device               *device = (T_Device *)m->_device;

          // Advance the message
          pami_result_t           result = m->advance();

          // Free the message if not PAMI_EAGAIN.
          if(result == PAMI_SUCCESS)
            device->freeWork(t);
          return result;
        }


      // Multicombine model constructor.
      // This registers and caches any information that can be re-used, like dispatch id's
      CAUMulticombineModel (T_Device & device, pami_result_t &status) :
        Interface::MulticombineModel < CAUMulticombineModel<T_Device, T_Message>,T_Device,sizeof(T_Message)> (device, status),
        _device(device)
        {
          TRACE((stderr, "CAUMulticombineModel:  Registering Dispatch Handler:  %p %p\n", cau_red_handler, cau_mcast_handler));
          _dispatch_red_id   = _device.registerSyncDispatch(cau_red_handler, this);
          _dispatch_mcast_id = _device.registerSyncDispatch(cau_mcast_handler, this);
          status             = PAMI_SUCCESS;
        };

      
      // Posts a multicombine operation
      // We are starting a multicombine operation, first we check the unexpected queue
      // with our sequence number to determine if we have any early arrival CAU messages
      // This will only happen on the root task, as all other tasks are synchronized
      // but it should be OK to check for ue messages on the non-root tasks.
      // If the message is found, return the object to the pool
      // and construct a new message and copy over the early arrival data
      inline pami_result_t postMulticombine (uint8_t (&state)[multicombine_model_state_bytes],
                                             size_t               client,
                                             size_t               context,
                                             pami_multicombine_t *mcombine,
                                             void                *devinfo)
          {
            CAUGeometryInfo  *gi             = (CAUGeometryInfo *)devinfo;
            T_Message        *msg, *earlymsg = (T_Message*)gi->_ueRed.findAndDelete(gi->_seqnoRed);
            pami_result_t     res            = PAMI_SUCCESS;
            msg = new(state) T_Message(&_device,gi,
                                       _dispatch_red_id,
                                       _dispatch_mcast_id);
            msg->init(mcombine);
            if(earlymsg)
            {
              msg->_reducePktBytes = earlymsg->_reducePktBytes;
              memcpy(msg->_reducePkt, earlymsg->_reducePkt, msg->_reducePktBytes);
              _msg_allocator.returnObject(earlymsg);
            }
            gi->_postedRed.pushTail((MatchQueueElem*)msg);                        
            msg->_workfcn = _device.postWork(do_reduce, msg);
            return PAMI_SUCCESS;
          }
      public:
        T_Device                                    &_device;
        int                                          _dispatch_red_id;
        int                                          _dispatch_mcast_id;        
        PAMI::MemoryAllocator<sizeof(T_Message),16>  _msg_allocator;
    };
  };
};
#endif // __components_devices_cau_caumulticombinemodel_h__
