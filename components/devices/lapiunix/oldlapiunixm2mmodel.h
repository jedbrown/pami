/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

      static void __xmi_lapi_m2m_senddone_fn (lapi_handle_t * handle, void * param, lapi_sh_info_t * info)
        {
          LAPIM2MMessage * sreq = (LAPIM2MMessage *) param;
          sreq->_numdone++;
          if(sreq->_numdone == sreq->_num)
              {
                if (sreq->_user_done_fn)
                  sreq->_user_done_fn (NULL, sreq->_cookie, XMI_SUCCESS);
                free(sreq->_send_headers);
              }
        }

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
          m2m->_conn        = connid;
          m2m->_numdone     = 0;
          if(cb_done)
              {
                m2m->_user_done_fn = cb_done->function;
                m2m->_cookie  = cb_done->clientdata;
              }
          else
            m2m->_user_done_fn = NULL;

          m2m->_num = 0;
          for( i = 0; i < nranks; i++)
          {
            if( sizes[i] == 0 )
              continue;
            m2m->_num++;
          }

          if( m2m->_num == 0 )
          {
            if( m2m->_user_done_fn )
              m2m->_user_done_fn(NULL,m2m->_cookie,XMI_SUCCESS);
            free ( m2m );
            return ;
          }

          m2m->_send_headers = (LAPIM2MHeader*)malloc(m2m->_num*sizeof(LAPIM2MHeader));
          XMI_assert ( m2m->_send_headers != NULL );
          LAPIM2MHeader *hdr = (LAPIM2MHeader *) m2m->_send_headers;
          for( i = 0; i < nranks; i++)
              {
                lapi_xfer_t xfer_struct;
                int index = permutation[i];
                XMI_assert ( (unsigned)index < nranks );
                if( sizes[index] == 0 ) continue;
                hdr->_dispatch_id = _dispatch_id;
                hdr->_size        = sizes[index];
                hdr->_conn        = connid;
                hdr->_peer        = __global.mapping.task();
                int          done = 0;
                if (sizeof(LAPIM2MHeader) + hdr->_size < 128)
                    {
                      lapi_xfer_t xfer_struct;
                      xfer_struct.Am.Xfer_type = LAPI_AM_LW_XFER;
                      xfer_struct.Am.flags     = 0;
                      xfer_struct.Am.tgt       = ranks[index];
                      xfer_struct.Am.hdr_hdl   = (lapi_long_t)2L;
                      xfer_struct.Am.uhdr      = (void *) hdr;
                      xfer_struct.Am.uhdr_len  = sizeof(LAPIM2MHeader);
                      xfer_struct.Am.udata     = (void *) ((char*)buf+offsets[index]);
                      xfer_struct.Am.udata_len = sizes[index];
                      CheckLapiRC(lapi_xfer(_device._lapi_handle, &xfer_struct));
                      __xmi_lapi_m2m_senddone_fn(&_device._lapi_handle, m2m, NULL);
                    }
                else
                    {
                      xfer_struct.Am.Xfer_type = LAPI_AM_XFER;
                      xfer_struct.Am.flags     = 0;
                      xfer_struct.Am.tgt       = ranks[index];
                      xfer_struct.Am.hdr_hdl   = (lapi_long_t)2L;
                      xfer_struct.Am.uhdr      = (void *) hdr;
                      xfer_struct.Am.uhdr_len  = sizeof(LAPIM2MHeader);
                      xfer_struct.Am.udata     = (void *) ((char*)buf+offsets[index]);
                      xfer_struct.Am.udata_len = sizes[index];
                      xfer_struct.Am.shdlr     = (scompl_hndlr_t*) __xmi_lapi_m2m_senddone_fn;
                      xfer_struct.Am.sinfo     = (void *) m2m;
                      xfer_struct.Am.org_cntr  = NULL;
                      xfer_struct.Am.cmpl_cntr = NULL;
                      xfer_struct.Am.tgt_cntr  = NULL;
                      CheckLapiRC(lapi_xfer(_device._lapi_handle, &xfer_struct));
                    }
                hdr++;
              }
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
          msg->_buf      = buf;
          msg->_sizes    = sizes;
          msg->_offsets  = offsets;
          msg->_nranks   = nranks;
          msg->_m2mrecvQ = &_device._m2mrecvQ;
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
