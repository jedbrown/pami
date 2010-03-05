/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

#include "sys/xmi.h"
#include "util/common.h"
#include "util/queue/Queue.h"
#include "PipeWorkQueue.h"
#include <mpi.h>

//#define EMULATE_NONDETERMINISTIC_DEVICE
//#define EMULATE_UNRELIABLE_DEVICE
#define EMULATE_UNRELIABLE_DEVICE_FREQUENCY 10

#undef TRACE_DEVICE
#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
#define MAX_MULTISYNCS 1000

    typedef enum
    {
      P2P_PACKET_TAG=0,
      P2P_PACKET_DATA_TAG,
      OLD_MULTICAST_TAG,
      OLD_M2M_TAG,
      MULTICAST_TAG,
      MULTISYNC_TAG,
      LAST_TAG=MULTISYNC_TAG+MAX_MULTISYNCS
    }MPITag;



    class MPIMessage
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
        TRACE_DEVICE((stderr,"%s dispatch_id %d\n",__PRETTY_FUNCTION__, dispatch_id));
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


    class OldMPIMcastMessage
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

    class OldMPIMcastRecvMessage
    {
    public:
      size_t              _dispatch_id;
      unsigned            _conn;
      xmi_event_function  _done_fn;
      void               *_cookie;
      char               *_buf;
      size_t              _size;
      size_t              _pwidth;
      unsigned            _nranks;
      unsigned            _hint;
      xmi_op              _op;
      xmi_dt              _dtype;
      size_t              _counter;
    };

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

    template <class T_Counter>
    class MPIM2MRecvMessage
    {
    public:
      size_t              _dispatch_id;
      unsigned            _conn;
      xmi_event_function  _done_fn;
      void               *_cookie;
      int                 _num;
      char               *_buf;
      T_Counter          *_sizes;
      T_Counter          *_offsets;
      unsigned            _nranks;
    };

    class MPIM2MHeader
    {
    public:
      size_t      _dispatch_id;
      unsigned    _size;
      unsigned    _conn;
      inline void *buffer() { return ((char *)this + sizeof (MPIM2MHeader)); }
      inline int  totalsize () { return _size + sizeof (MPIM2MHeader); }
    };


    class MPIMsyncMessage
    {
    public:
      xmi_callback_t _cb_done;
      unsigned       _numphases;
      unsigned       _sendcomplete;
      unsigned       _phase;
      bool           _sendStarted;
      bool           _sendDone;
      bool           _recvDone;
      size_t         _dests[64];
      size_t         _srcs[64];
      MPI_Request    _reqs[64];
      struct _p2p_msg
      {
        unsigned       _connection_id;
      }_p2p_msg;
    };

    class MPIMcastMessage
    {
    public:
      xmi_callback_t      _cb_done;
      XMI::Topology      *_srcranks;
      XMI::Topology      *_dstranks;
      XMI::PipeWorkQueue *_srcpwq;
      XMI::PipeWorkQueue *_dstpwq;
      size_t              _root;
      size_t              _bytes;
      size_t              _currBytes;
      char               *_currBuf;
      xmi_task_t         *_ranks;
      size_t              _numRanks;
      size_t              _dests[64];
      size_t              _srcs[64];
      MPI_Request         _reqs[64];
      struct _p2p_msg
      {
        unsigned       _connection_id;
      }_p2p_msg;
    };

    class MPIMcastRecvMessage
    {
    public:


    };


    class MPIMcombineMessage
    {
    public:
      unsigned toimpl;
    };

  };
};
#undef TRACE_DEVICE

#endif // __components_devices_mpi_mpimessage_h__
