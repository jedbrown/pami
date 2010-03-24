/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/tspcoll/NBColl.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_NBColl_h__
#define __algorithms_protocols_tspcoll_NBColl_h__

#include "algorithms/geometry/Geometry.h"

namespace TSPColl
{
  typedef unsigned char *           __pgasrt_local_addr_t;
  /* *********************************************************** */
  /* *********************************************************** */
  enum NBTag
  {
    BarrierTag=0,
    AllgatherTag,
    AllgathervTag,
    ScatterTag,
    ScattervTag,
    GatherTag,
    GathervTag,
    BcastTag,
    BcastTag2,
    ShortAllreduceTag,
    LongAllreduceTag,
    MAXTAG
  };

  /* *********************************************************** */
  /*        a generic non-blocking transport collective          */
  /* *********************************************************** */
  template <class T_mcast>
  class NBColl
  {
  public:
    NBColl (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int instID,
            pami_event_function cb_complete, void *arg);

  public:
    virtual void  kick  (T_mcast *mcast_iface) {};
    virtual bool isdone () const { return false; } /* check completion */
    int instID () const { return _instID; }
    int tag    () const { return _tag;    }
    virtual void setComplete (pami_event_function cb_complete, void *arg);
// compiler gets cranky if this isn't here
    virtual ~NBColl() {}
  protected:
    PAMI_GEOMETRY_CLASS *_comm;
    NBTag               _tag;
    int                 _instID;
    pami_event_function  _cb_complete;
    void               *_arg;
  };



/* ************************************************************************ */
/*                 non-blocking collective constructor                      */
/* ************************************************************************ */
  template <class T_Mcast>
  TSPColl::NBColl<T_Mcast>::NBColl (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int instID,
                                    pami_event_function cb_complete, void *arg):
    _comm (comm), _tag (tag), _instID (instID),
    _cb_complete (cb_complete), _arg(arg)
  {
  }

/* ************************************************************************ */
/* ************************************************************************ */
  template <class T_Mcast>
  void TSPColl::NBColl<T_Mcast>::setComplete (pami_event_function cb_complete, void *arg)
  {
    _cb_complete = cb_complete;
    _arg = arg;
  }
};
#endif
