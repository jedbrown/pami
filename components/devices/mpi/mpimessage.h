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
          _msg._dispatch_id=dispatch_id;
        };
      xmi_context_t       _context;
      xmi_event_function  _done_fn;
      void               *_cookie;
      MPI_Request         _request;
      struct _msg
      {
        size_t              _dispatch_id;
        int                 _metadatasize;
        int                 _payloadsize;
        char                _metadata[128];
        char                _payload[224];
      }_msg;
    protected:
    private:
    };
  };
};

#endif // __components_devices_mpi_mpibasemessage_h__
