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
#include <list>

namespace PAMI
{
  namespace Device
  {
    class CAUMcastMessage
    {
    public:
      unsigned toimpl;
    };

    class CAUMsyncMessage : MatchQueueElem
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

    class CAUMcombineMessage
    {
    public:
      unsigned toimpl;
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
        _topo(topo)
        {}
      int             _cau_id;
      int             _geometry_id;
      uint64_t        _seqno;
      MatchQueue      _ue;
      MatchQueue      _posted;
      PAMI::Topology *_topo;
    };



  };
};

#endif // __components_devices_cau_caubasemessage_h__
