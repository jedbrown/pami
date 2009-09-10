#ifndef __tspcoll_nbcoll_h__
#define __tspcoll_nbcoll_h__

#include "components/geometry/common/commongeometry.h"

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
    NBColl (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * comm, NBTag tag, int instID,
	    void (*cb_complete)(void *), void *arg);

  public:
    virtual void  kick  (T_mcast *mcast_iface) {};
    virtual bool isdone () const { return false; } /* check completion */
    int instID () const { return _instID; }
    int tag    () const { return _tag;    }
    virtual void setComplete (void (*cb_complete)(void *), void *arg);

  protected:
    XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * _comm;
    NBTag          _tag;
    int            _instID;
    void        (* _cb_complete) (void *);
    void         * _arg;
  };

  

/* ************************************************************************ */
/*                 non-blocking collective constructor                      */
/* ************************************************************************ */
  template <class T_Mcast>
  TSPColl::NBColl<T_Mcast>::NBColl (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * comm, NBTag tag, int instID,
                                    void (*cb_complete)(void *), void *arg):
    _comm (comm), _tag (tag), _instID (instID),
    _cb_complete (cb_complete), _arg(arg)
  {
  }

/* ************************************************************************ */
/* ************************************************************************ */
  template <class T_Mcast>
  void TSPColl::NBColl<T_Mcast>::setComplete (void (*cb_complete)(void *), void *arg)
  {
    _cb_complete = cb_complete;
    _arg = arg;
  }
};
#endif
