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
#include "util/queue/MatchQueue.h"
#include "util/queue/Queue.h"
#include "components/devices/MulticastModel.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/MultisyncModel.h"
#include "math/math_coremath.h"
#include "algorithms/protocols/allreduce/ReduceFunctions.h"
#include "TypeDefs.h"

namespace PAMI
{
  namespace Device
  {
    class CAUGeometryInfo
    {
    public:
      CAUGeometryInfo(int cau_id,int geometry_id):
        _cau_id(cau_id),
        _geometry_id(geometry_id),
        _seqno(0),
        _ueBar(),
        _postedBar(),
        _seqnoRed(0),
        _ueRed(),
        _postedRed(),
        _seqnoBcast(0),
        _postedBcast(),
        _ueBcast()
        {}
      int             _cau_id;
      int             _geometry_id;
      unsigned        _seqno;
      MatchQueue      _ueBar;
      MatchQueue      _postedBar;

      unsigned        _seqnoRed;
      MatchQueue      _ueRed;
      MatchQueue      _postedRed;

      unsigned        _seqnoBcast;
      MatchQueue      _postedBcast;
      MatchQueue      _ueBcast;
      
//      PAMI::Topology *_topo;
    };



  };
};

#endif // __components_devices_cau_caubasemessage_h__
