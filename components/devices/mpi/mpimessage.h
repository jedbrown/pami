/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpimessage.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpimessage_h__
#define __components_devices_mpi_mpimessage_h__

#ifndef TRACE_ADAPTOR
  #define TRACE_ADAPTOR(x) //fprintf x
#endif

#include "sys/xmi.h"
#include "util/common.h"
#include "util/queue/Queue.h"
#include <mpi.h>
#include "components/devices/mpi/mpimessage_old.h"
#include "common/mpi/PipeWorkQueue.h"
#include "common/mpi/Topology.h"

//#define EMULATE_NONDETERMINISTIC_DEVICE
//#define EMULATE_UNRELIABLE_DEVICE
#define EMULATE_UNRELIABLE_DEVICE_FREQUENCY 10

namespace XMI
{
  namespace Device
  {
    class MPICommMcastMessage
    {
    public:
      inline MPIMessage (xmi_client_t client, size_t       context,
                         size_t              dispatch_id,
                         xmi_event_function  done_fn,
                         void               *cookie):
        _client(client),
        _context(context),
        _done_fn(done_fn),
        _cookie(cookie)
        {
          _p2p_msg._dispatch_id=dispatch_id;
        };
      xmi_client_t       _client;
      size_t       _context;
      xmi_event_function  _done_fn;
      void               *_cookie;
      int                 _freeme;
      MPI_Request         _request;
      xmi_task_t          _target_task;

      struct _p2p_msg
      {
        size_t              _dispatch_id;
        int                 _metadatasize;
        int                 _payloadsize0;
        int                 _payloadsize1;
        char                _metadata[128];
        char                _payload[224];
      }_p2p_msg;
    protected:
    private:
    };


    class MPIMcastMessage
    {
    public:
      xmi_client_t  _client;
      size_t  _context;
      size_t         _dispatch_id;
      xmi_quad_t     _info[2];
      int            _info_count;
      int            _size;
      unsigned       _conn;
      MPI_Request    *_req;
      int            _num;
      xmi_callback_t _cb_done;
      inline void *buffer() { return ((char *)this + sizeof (*this)); }
      inline int  totalsize () { return _size + sizeof (*this); }
    };

      size_t              _dispatch_id;
      size_t              _bytesAvailable;
      size_t              _bytesComplete; 
      char               *_dataBuffer;

    class MPIM2MMessage
    {
    public:
      xmi_client_t       _client;
      size_t       _context;
      size_t              _dispatch_id;
      unsigned            _conn;
      xmi_event_function  _done_fn;
      void               *_cookie;
      int                 _num;
      MPI_Request        *_reqs;
      int                 _totalsize;
      char               *_bufs;
    };

      const void complete()
      {
        if(_cb_done.function)
          (_cb_done.function)(_context,_cb_done.clientdata, XMI_SUCCESS);
      }

    };

  };
};

#endif // __components_devices_mpi_mpibasemessage_h__


