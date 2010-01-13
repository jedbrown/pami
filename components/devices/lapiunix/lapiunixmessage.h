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

#include "sys/xmi.h"
#include "util/common.h"
#include "util/queue/Queue.h"
#include <lapi.h>
#include <list>

namespace XMI
{
  namespace Device
  {
    #define XMI_TSP_AMSEND_REG_MAX        100
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
                         xmi_event_function  done_fn,
                         void               *cookie):
        _context(context),
        _done_fn(done_fn),
        _cookie(cookie)
        {
          _p2p_msg._dispatch_id=dispatch_id;
        };
      size_t              _context;
      xmi_event_function  _done_fn;
      void               *_cookie;
      int                 _freeme;
      LAPIP2PMessage _p2p_msg;
    protected:
    private:
    };


    class LAPIMcastSendReq
    {
    public:
      int                   _count;
      int                   _total;
      xmi_callback_t        _user_done;
    };



    class LAPIMcastMessage
    {
    public:
      xmi_quad_t     _info[2];
      int            _info_count;
      xmi_context_t  _context;
      size_t         _dispatch_id;
      int            _peer;
      int            _size;
      unsigned       _conn;
      int            _num;
      xmi_callback_t _cb_done;
      inline void *buffer() { return ((char *)this + sizeof (*this)); }
      inline int  totalsize () { return _size + sizeof (*this); }
    }__attribute__((__aligned__(16)));;

    class LAPIMcastRecvMessage
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

    class LAPIMcastRecvReq
    {
    public:
      LAPIMcastRecvMessage              _mcast;
      int                               _found;
      std::list<LAPIMcastRecvMessage*> *_mcastrecvQ;
    };

    class LAPIM2MHeader
    {
    public:
      size_t      _dispatch_id;
      unsigned    _peer;
      unsigned    _size;
      unsigned    _conn;
      inline void *buffer() { return ((char *)this + sizeof (LAPIM2MHeader)); }
      inline int  totalsize () { return _size + sizeof (LAPIM2MHeader); }
    };



    class LAPIM2MMessage
    {
    public:
      xmi_context_t       _context;
      unsigned            _conn;
      xmi_event_function  _user_done_fn;
      void               *_cookie;
      int                 _num;
      int                 _numdone;
      LAPIM2MHeader      *_send_headers;
    };

    template <class T_Counter>
    class LAPIM2MRecvMessage
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
      std::list<LAPIM2MRecvMessage*> *_m2mrecvQ;
    };



  };
};

#endif // __components_devices_lapi_lapibasemessage_h__
