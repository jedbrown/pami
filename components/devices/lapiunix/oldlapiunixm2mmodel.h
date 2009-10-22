/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/oldlapiunixm2mmodel.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_oldlapiunixm2mmodel_h__
#define __components_devices_lapiunix_oldlapiunixm2mmodel_h__

#include "sys/xmi.h"
#include "components/devices/OldM2MModel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include <lapi.h>
#include "util/common.h"
namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message, class T_Counter>
    class LAPIOldm2mModel : public Interface::Oldm2mModel<LAPIOldm2mModel<T_Device, T_Message, T_Counter>, T_Device, T_Message, T_Counter>
    {
    public:
      LAPIOldm2mModel (T_Device & device) :
        Interface::Oldm2mModel<LAPIOldm2mModel<T_Device, T_Message, T_Counter>, T_Device, T_Message, T_Counter> (device),
        _device(device)
        {
          _dispatch_id = _device.initM2M();
        };

      inline void setCallback (xmi_olddispatch_manytomany_fn cb_recv, void *arg)
        {
          _device.registerM2MRecvFunction (_dispatch_id, cb_recv, arg);
        }

      inline void  send_impl  (XMI_Request_t         * request,
                               const xmi_callback_t  * cb_done,
                               unsigned                connid,
                               unsigned                rcvindex,
                               const char            * buf,
                               T_Counter              * sizes,
                               T_Counter              * offsets,
                               T_Counter              * counters,
                               T_Counter              * ranks,
                               T_Counter              * permutation,
                               unsigned                nranks)
        {

          unsigned i;
          LAPIM2MMessage * m2m = (LAPIM2MMessage *)malloc(sizeof(LAPIM2MMessage));
          XMI_assert( m2m != NULL );

          m2m->_context     = NULL;
          m2m->_dispatch_id = _dispatch_id;
          m2m->_conn        = connid;

          if(cb_done)
              {
                m2m->_done_fn = cb_done->function;
                m2m->_cookie  = cb_done->clientdata;
              }
          else
            m2m->_done_fn = NULL;

          m2m->_num = 0;
          m2m->_totalsize = 0;

          for( i = 0; i < nranks; i++)
          {
            if( sizes[i] == 0 )
              continue;
            m2m->_num++;
            m2m->_totalsize += (sizes[i] + sizeof(LAPIM2MHeader));
          }

          if( m2m->_num == 0 )
          {
            if( m2m->_done_fn )
              m2m->_done_fn(NULL, m2m->_cookie,XMI_SUCCESS);
            free ( m2m );
            return ;
          }

//          m2m->_reqs = (LAPI_Request *)malloc( m2m->_num * sizeof(LAPI_Request));
//          XMI_assert ( m2m->_reqs != NULL );
          m2m->_bufs = ( char *)malloc( m2m->_totalsize );
          XMI_assert ( m2m->_bufs != NULL );

          LAPIM2MHeader   * hdr = (LAPIM2MHeader *) m2m->_bufs;
//          LAPI_Request    * req = m2m->_reqs;
          for( i = 0; i < nranks; i++)
          {
            int index = permutation[i];
            XMI_assert ( (unsigned)index < nranks );
            if( sizes[index] == 0 ) continue;
            hdr->_dispatch_id = _dispatch_id;
            hdr->_size        = sizes[index];
            hdr->_conn        = connid;
            memcpy (hdr->buffer(), buf+offsets[index], sizes[index]);
            int rc = -1;
//            rc = LAPI_Isend (hdr,
//                            hdr->totalsize(),
//                            LAPI_CHAR,
//                            ranks[index],
//                            3,
//                            LAPI_COMM_WORLD,
//                            req);
            XMI_assert (rc == LAPI_SUCCESS);
            hdr = (LAPIM2MHeader *)((char *)hdr + hdr->totalsize());
//            req++;
          }
          _device.enqueue(m2m);
          return;
        }

      inline void postRecv_impl (XMI_Request_t          * request,
                                 const xmi_callback_t   * cb_done,
                                 unsigned                 connid,
                                 char                   * buf,
                                 T_Counter              * sizes,
                                 T_Counter              * offsets,
                                 T_Counter              * counters,
                                 unsigned                 nranks,
                                 unsigned                 myindex)
        {
          LAPIM2MRecvMessage<T_Counter> *msg = (LAPIM2MRecvMessage<T_Counter>*)malloc(sizeof(*msg));
          msg->_dispatch_id = _dispatch_id;
          msg->_conn        = connid;
          msg->_done_fn     = cb_done->function;
          msg->_cookie      = cb_done->clientdata;
          msg->_num         = 0;
          for( unsigned i = 0; i < nranks; i++)
          {
            if( sizes[i] == 0 ) continue;
            msg->_num++;
          }

          if( msg->_num == 0 )
          {
            if( msg->_done_fn )
              (*msg->_done_fn)(NULL, msg->_cookie,XMI_SUCCESS);
            free(msg);
          }
          msg->_buf     = buf;
          msg->_sizes   = sizes;
          msg->_offsets = offsets;
          msg->_nranks  = nranks;
          _device.enqueue(msg);
          return;
        }
      T_Device                     &_device;
      size_t                        _dispatch_id;
      xmi_olddispatch_manytomany_fn _cb_async_head;
      void                         *_async_arg;

    };
  };
};
#endif // __components_devices_lapi_lapioldm2mmodel_h__
