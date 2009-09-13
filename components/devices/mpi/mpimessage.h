#ifndef __components_devices_mpi_mpimessage_h__
#define __components_devices_mpi_mpimessage_h__

#include "sys/xmi.h"
#include "util/common.h"
#include "util/queue/Queue.h"
#include <mpi.h>

namespace XMI
{
  namespace Device
  {
    class MPIMessage
    {
    public:
      inline MPIMessage (xmi_context_t       context,
                         size_t              dispatch_id,
                         xmi_event_function  done_fn,
                         void               *cookie):
      _context(context),
        _done_fn(done_fn),
        _cookie(cookie)
        {
          _p2p_msg._dispatch_id=dispatch_id;
        };
      xmi_context_t       _context;
      xmi_event_function  _done_fn;
      void               *_cookie;
      MPI_Request         _request;
      struct _p2p_msg
      {
        size_t              _dispatch_id;
        int                 _metadatasize;
        int                 _payloadsize;
        char                _metadata[128];
        char                _payload[224];
      }_p2p_msg;
    protected:
    private:
    };

    class MPIMcastMessage
    {
    public:
      xmi_context_t  _context;
      size_t         _dispatch_id;
      xmi_quad_t     _info[2];
      int            _info_count;
      int            _size;
      int            _conn;
      MPI_Request    *_req;
      int            _num;
      xmi_callback_t _cb_done;
      inline void *buffer() { return ((char *)this + sizeof (*this)); } 
      inline int  totalsize () { return _size + sizeof (*this); } 
    };
  };
};

#endif // __components_devices_mpi_mpibasemessage_h__
