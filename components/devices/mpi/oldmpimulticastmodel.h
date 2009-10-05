/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/oldmpimulticastmodel.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpioldmulticastmodel_h__
#define __components_devices_mpi_mpioldmulticastmodel_h__

#include "sys/xmi.h"
#include "components/devices/OldMulticastModel.h"
#include "components/devices/mpi/mpimessage.h"
#include <mpi.h>
#include "util/common.h"
namespace XMI
{
  namespace Device
  {
    template <class T_Device, class T_Message>
      class MPIOldmulticastModel : public Interface::OldmulticastModel<MPIOldmulticastModel<T_Device, T_Message>, T_Device, T_Message>
      {
      public:
	MPIOldmulticastModel (T_Device & device) :
        Interface::OldmulticastModel<MPIOldmulticastModel<T_Device, T_Message>, T_Device, T_Message> (device),
	  _device(device)
	  {
            _dispatch_id = _device.initMcast();
          };

	inline void setCallback (xmi_olddispatch_multicast_fn cb_recv, void *arg)
        {
          _device.registerMcastRecvFunction (_dispatch_id, cb_recv, arg);
        }

	inline unsigned  send   (XMI_Request_t             * request,
				 const xmi_callback_t      * cb_done,
				 xmi_consistency_t           consistency,
				 const xmi_quad_t          * info,
				 unsigned                    info_count,
				 unsigned                    connection_id,
				 const char                * buf,
				 unsigned                    size,
				 unsigned                  * hints,
				 unsigned                  * ranks,
				 unsigned                    nranks,
				 xmi_op                      op    = XMI_UNDEFINED_OP,
				 xmi_dt                      dtype = XMI_UNDEFINED_DT )
        {

	  MPIMcastMessage *hdr = (MPIMcastMessage *) malloc (sizeof (MPIMcastMessage) + size);
          XMI_assert( hdr != NULL );
          hdr->_dispatch_id = _dispatch_id;
          hdr->_info_count  = info_count;
          hdr->_size        = size;
          hdr->_conn        = connection_id;
          if( info )
	    {
	      memcpy (&hdr->_info[0], info, info_count * sizeof (xmi_quad_t));
	      if(info_count > 2)
		{
		  fprintf(stderr, "FIX:  The generic adaptor only supports up to 2 quads\n");
		  abort();
		}
	    }
          memcpy (hdr->buffer(), buf, size);

          int rc = -1;

          hdr->_req = (MPI_Request*)malloc(sizeof(MPI_Request)*nranks);
          hdr->_num = nranks;
          if(cb_done)
            hdr->_cb_done = *cb_done;
          else
            hdr->_cb_done.function = NULL;

          XMI_assert(hdr->_req != NULL);
          for(unsigned count = 0; count < nranks; count ++)
	    {
	      XMI_assert (hints[count] == XMI_PT_TO_PT_SUBTASK);

	      rc = MPI_Isend (hdr,
			      hdr->totalsize(),
			      MPI_CHAR,
			      ranks[count],
			      2,
			      MPI_COMM_WORLD,
			      &hdr->_req[count]);
	      XMI_assert (rc == MPI_SUCCESS);
	    }
	  _device.enqueue(hdr);
          return rc;
        }


	inline unsigned send (xmi_oldmulticast_t *mcastinfo)
        {
          return this->send((XMI_Request_t*)mcastinfo->request,
                            &mcastinfo->cb_done,
                            XMI_MATCH_CONSISTENCY,
                            mcastinfo->msginfo,
                            mcastinfo->count,
                            mcastinfo->connection_id,
                            mcastinfo->src,
                            mcastinfo->bytes,
                            (unsigned *)mcastinfo->opcodes,
                            mcastinfo->ranks,
                            mcastinfo->nranks);
        }


        inline unsigned postRecv (XMI_Request_t          * request,
                                  const xmi_callback_t   * cb_done,
                                  unsigned                 conn_id,
                                  char                   * buf,
                                  unsigned                 size,
                                  unsigned                 pwidth,
                                  unsigned                 hint   = XMI_UNDEFINED_SUBTASK,
                                  xmi_op                   op     = XMI_UNDEFINED_OP,
                                  xmi_dt                   dtype  = XMI_UNDEFINED_DT)
        {

          MPIMcastRecvMessage *msg = (MPIMcastRecvMessage*)malloc(sizeof(*msg));
          msg->_dispatch_id = _dispatch_id;
          msg->_conn     = conn_id;
          msg->_done_fn  = cb_done->function;
          msg->_cookie   = cb_done->clientdata;
          msg->_buf      = buf;
          msg->_size     = size;
          msg->_pwidth   = pwidth;
	  msg->_counter  = 0;
//          msg->_hint     = hint;
//          msg->_op       = op;
//          msg->_dtype    = dtype;
          _device.enqueue(msg);
	  return 0;
        }

	inline unsigned postRecv (xmi_oldmulticast_recv_t  *mrecv)
        {
          postRecv((XMI_Request_t*)mrecv->request,
                   &mrecv->cb_done,
                   mrecv->connection_id,
                   mrecv->rcvbuf,
                   mrecv->bytes,
                   mrecv->pipelineWidth,
                   mrecv->opcode,
                   mrecv->op,
                   mrecv->dt);
	  return 0;
        }


	T_Device                     &_device;
	size_t                        _dispatch_id;
	xmi_olddispatch_multicast_fn  _cb_async_head;
	void                         *_async_arg;

      };
  };
};
#endif // __components_devices_mpi_mpioldmulticastmodel_h__
