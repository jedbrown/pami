/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumessage.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumessage_h__
#define __components_devices_cau_caumessage_h__

#include <pami.h>
#include "util/common.h"
#include "util/queue/MatchQueue.h"
#include "common/lapiunix/lapifunc.h"
#include "TypeDefs.h"
#include "components/memory/MemoryAllocator.h"
#include "algorithms/protocols/allreduce/ReduceFunctions.h"
#include <list>

namespace PAMI
{
  namespace Device
  {
    class CAUMcastSendMessage
    {
    public:
      CAUMcastSendMessage(size_t                client,
                          size_t                context,
                          size_t                dispatch,
                          pami_event_function   fn,
                          void                 *cookie,
                          unsigned              connection_id,
                          unsigned              roles,
                          size_t                bytes,
                          PipeWorkQueue        *src_pwq,
                          Topology             *src_participants,
                          PipeWorkQueue        *dst_pwq,
                          Topology             *dst_participants,
                          const pami_quad_t    *msginfo,
                          unsigned              msgcount,
                          void                 *device,
                          void                 *devinfo):
        _client(client),
        _context(context),
        _dispatch(dispatch),
        _fn(fn),
        _cookie(cookie),
        _connection_id(connection_id),
        _roles(roles),
        _bytes(bytes),
        _src_pwq(src_pwq),
        _src_participants(src_participants),
        _dst_pwq(dst_pwq),
        _dst_participants(dst_participants),
        _msginfo(msginfo),
        _msgcount(msgcount),
        _xfer_msghdr(NULL),
        _device(device),
        _devinfo(devinfo)
        {

        }
      void *allocateHeader(size_t sz)
        {
          _xfer_msghdr = malloc(sz);
          return _xfer_msghdr;
        }

      void freeHeader()
        {
          free(_xfer_msghdr);
        }
      size_t                _client;
      size_t                _context;
      size_t                _dispatch;
      pami_event_function   _fn;
      void                 *_cookie;
      unsigned              _connection_id;
      unsigned              _roles;
      size_t                _bytes;
      PipeWorkQueue        *_src_pwq;
      Topology             *_src_participants;
      PipeWorkQueue        *_dst_pwq;
      Topology             *_dst_participants;
      const pami_quad_t    *_msginfo;
      unsigned              _msgcount;
      void                 *_xfer_msghdr;
      void                 *_device;
      void                 *_devinfo;
    };

    class CAUMcastRecvMessage
    {
    public:
      CAUMcastRecvMessage(pami_callback_t  cb_done,
                          void            *target_buf,
                          size_t           buflen,
                          PipeWorkQueue   *pwq,
                          pami_context_t   context):
        _cb_done(cb_done),
        _side_buf(NULL),
        _buflen(buflen),
        _pwq(pwq),
        _bytesProduced(0),
        _bytesCopied(0),
        _context(context),
        _thread(NULL)
        {
        }


      CAUMcastRecvMessage(pami_callback_t  cb_done,
                          void            *target_buf,
                          size_t           buflen,
                          PipeWorkQueue   *pwq,
                          pami_context_t   context,
                          int              alloc):
        _cb_done(cb_done),
        _side_buf(malloc(buflen)),
        _buflen(buflen),
        _bytesProduced(0),
        _bytesCopied(0),
        _pwq(pwq),
        _thread(NULL)
        {
        }
      pami_callback_t            _cb_done;
      void                      *_side_buf;
      size_t                     _buflen;
      size_t                     _bytesProduced;
      size_t                     _bytesCopied;
      PipeWorkQueue             *_pwq;
      pami_context_t             _context;
      Generic::GenericThread    *_thread;
    };

    class CAUMsyncMessage : public MatchQueueElem
    {
    public:
      CAUMsyncMessage(double               init_val,
                      pami_context_t       context,
                      pami_event_function  done_fn,
                      void                *user_cookie,
                      int                  key,
                      void                *toFree):
        MatchQueueElem(key),
        _reduce_val(init_val),
        _user_done_fn(done_fn),
        _user_cookie(user_cookie),
        _toFree(toFree),
        _context(context)
        {
        }
      double               _reduce_val;
      pami_event_function  _user_done_fn;
      void                *_user_cookie;
      void                *_toFree;
      pami_context_t       _context;
      int                  _xfer_data[3];

    };

    class CAUMcombineMessage : public MatchQueueElem
    {
    public:
      CAUMcombineMessage(size_t               count,
                         size_t               sizeoftype,
                         PipeWorkQueue       *rcvpwq,
                         PipeWorkQueue       *sndpwq,
                         cau_reduce_op_t      red,
                         pami_context_t       context,
                         pami_event_function  done_fn,
                         void                *user_cookie,
                         int                  key,                         
                         void                *toFree):
        MatchQueueElem(key),
        _user_done_fn(done_fn),
        _user_cookie(user_cookie),
        _toFree(toFree),
        _count(count),
        _sizeoftype(sizeoftype),
        _bytesReduced(0),
        _bytesToReduce(count*sizeoftype),
        _bytesBroadcasted(0),
        _bytesToBroadcast(_bytesToReduce),
        _currentBytes(0),
        _rcvpwq(rcvpwq),
        _sndpwq(sndpwq),
        _red(red),
        _context(context),
        _devinfo(NULL),
        _device(NULL),
        _workfcn(NULL)
        {
        }
      pami_event_function  _user_done_fn;
      void                *_user_cookie;
      void                *_toFree;
      pami_context_t       _context;
      size_t               _count;
      size_t               _sizeoftype; 
      size_t               _bytesReduced;
      size_t               _bytesToReduce;
      size_t               _bytesBroadcasted;
      size_t               _bytesToBroadcast;
      size_t               _currentBytes;
      PipeWorkQueue       *_rcvpwq;
      PipeWorkQueue       *_sndpwq;
      cau_reduce_op_t      _red;
      void                *_ue_buf;
      size_t               _ue_bytes;
      void                *_ue_hdr;
      size_t               _ue_hdr_bytes;
      void                *_devinfo;
      void                *_device;
      coremath             _math_func;
      bool                 _reduceOnly;
      Generic::GenericThread *_workfcn;
      struct               xfer_header
      {
        unsigned           dispatch_id:16;
        unsigned           geometry_id:16;
        unsigned           seqno      :32;
        unsigned           pktsize    :7;
        unsigned           msgsize    :25;
      } _xfer_header __attribute__((__packed__));
      struct               xfer_header_b
      {
        unsigned           dispatch_id:16;
        unsigned           geometry_id:16;
        unsigned           seqno      :32;
        unsigned           pktsize    :7;
        unsigned           msgsize    :25;
      } _xfer_header_b __attribute__((__packed__));


    };

    class CAUM2MMessage
    {
    public:
      unsigned toimpl;
    };



    class CAUGeometryInfo
    {
    public:
      CAUGeometryInfo(int             cau_id,
                      int             geometry_id,
                      PAMI::Topology *topo):
        _cau_id(cau_id),
        _geometry_id(geometry_id),
        _seqno(0),
        _ue(),
        _posted(),
        _seqnoRed(0),
        _ueRed(),
        _postedRed(),
        _postedBcast(),
        _topo(topo)
        {}
      int             _cau_id;
      int             _geometry_id;
      uint64_t        _seqno;
      MatchQueue      _ue;
      MatchQueue      _posted;

      uint64_t        _seqnoRed;
      MatchQueue      _ueRed;
      MatchQueue      _postedRed;
      MatchQueue      _postedBcast;

      PAMI::Topology *_topo;
    };



  };
};

#endif // __components_devices_cau_caubasemessage_h__
