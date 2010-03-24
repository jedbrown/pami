/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/lapiunix/lapiunixmessage.h
 * \brief ???
 */

#ifndef __components_devices_lapiunix_lapiunixmessage_h__
#define __components_devices_lapiunix_lapiunixmessage_h__

#include "sys/pami.h"
#include "util/common.h"
#include "util/queue/Queue.h"
#include <lapi.h>
#include <list>

namespace PAMI
{
  namespace Device
  {
    #define PAMI_TSP_AMSEND_REG_MAX        100
    #define DEV_HEADER_SIZE               64
    #define DEV_PAYLOAD_SIZE              64


    class LAPIP2PMessage
    {
    public:
      size_t              _dispatch_id;
      int                 _metadatasize;
      int                 _payloadsize0;
      int                 _payloadsize1;
      char                _metadata[DEV_HEADER_SIZE];
      char                _payload[DEV_PAYLOAD_SIZE];
    };


    class LAPIMessage
    {
    public:
      inline LAPIMessage (size_t       context,
                         size_t              dispatch_id,
                         pami_event_function  done_fn,
                         void               *cookie):
        _context(context),
        _done_fn(done_fn),
        _cookie(cookie)
        {
          _p2p_msg._dispatch_id=dispatch_id;
        };
      size_t              _context;
      pami_event_function  _done_fn;
      void               *_cookie;
      int                 _freeme;
      LAPIP2PMessage _p2p_msg;
    protected:
    private:
    };


    class OldLAPIMcastSendReq
    {
    public:
      int                   _count;
      int                   _total;
      pami_callback_t        _user_done;
    };



    class OldLAPIMcastMessage
    {
    public:
      pami_quad_t     _info[2];
      int            _info_count;
      pami_context_t  _context;
      size_t         _dispatch_id;
      int            _peer;
      int            _size;
      unsigned       _conn;
      int            _num;
      pami_callback_t _cb_done;
      inline void *buffer() { return ((char *)this + sizeof (*this)); }
      inline int  totalsize () { return _size + sizeof (*this); }
    }__attribute__((__aligned__(16)));;

    class OldLAPIMcastRecvMessage
    {
    public:
      size_t              _dispatch_id;
      unsigned            _conn;
      pami_event_function  _done_fn;
      void               *_cookie;
      char               *_buf;
      size_t              _size;
      size_t              _pwidth;
      unsigned            _nranks;
      unsigned            _hint;
      pami_op              _op;
      pami_dt              _dtype;
      size_t              _counter;
    };

    class OldLAPIMcastRecvReq
    {
    public:
      OldLAPIMcastRecvMessage              _mcast;
      int                               _found;
      std::list<OldLAPIMcastRecvMessage*> *_mcastrecvQ;
    };

    class OldLAPIM2MHeader
    {
    public:
      size_t      _dispatch_id;
      unsigned    _peer;
      unsigned    _size;
      unsigned    _conn;
      inline void *buffer() { return ((char *)this + sizeof (OldLAPIM2MHeader)); }
      inline int  totalsize () { return _size + sizeof (OldLAPIM2MHeader); }
    };



    class OldLAPIM2MMessage
    {
    public:
      pami_context_t       _context;
      unsigned            _conn;
      pami_event_function  _user_done_fn;
      void               *_cookie;
      int                 _num;
      int                 _numdone;
      OldLAPIM2MHeader      *_send_headers;
    };

    template <class T_Counter>
    class OldLAPIM2MRecvMessage
    {
    public:
      size_t              _dispatch_id;
      unsigned            _conn;
      pami_event_function  _done_fn;
      void               *_cookie;
      int                 _num;
      char               *_buf;
      T_Counter          *_sizes;
      T_Counter          *_offsets;
      unsigned            _nranks;
      std::list<OldLAPIM2MRecvMessage*> *_m2mrecvQ;
    };



    class LAPIMcastMessage
    {
    public:
      unsigned toimpl;
    };

    class LAPIMsyncMessage
    {
    public:
      pami_callback_t _cb_done;
      size_t         _r_flag;
      size_t         _total;
      struct _p2p_msg
      {
        unsigned       _connection_id;
      }_p2p_msg;
    };

    class LAPIMcombineMessage
    {
    public:
      unsigned toimpl;
    };

    class LAPIM2MMessage
    {
    public:
      unsigned toimpl;
    };

  };
};

#endif // __components_devices_lapi_lapibasemessage_h__
