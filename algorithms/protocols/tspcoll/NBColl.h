/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */
#include "platform.h"

#ifndef __tspcoll_nbcoll_h__
#define __tspcoll_nbcoll_h__
namespace TSPColl
{
//  class XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS>;
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

  /* *********************************************************** */
  /*    A factory for generating non-blocking coll. instances    */
  /* *********************************************************** */
  template <class T_Mcast>
  class NBCollFactory
  {
  public:
    static NBColl<T_Mcast> * create (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * comm, NBTag tag, int id);
    static void initialize();
  };

  /* *********************************************************** */
  /*           simple vector class                               */
  /* *********************************************************** */

  template<class T> class Vector
    {
    public:
      void * operator new (size_t, void * addr) { return addr; }
      Vector() { _size = _max = 0; _v = NULL; }
      T & operator[](int idx);
      int max() const { return _max; }
      int size() const { return _size; }

    private:
      T    * _v;
      int    _max;
      int    _size;
    };

  /* *********************************************************** */
  /*   Managing non-blocking collectives at runtime.             */
  /* The manager is a singleton.                                 */
  /* *********************************************************** */
  template <class T_Mcast>
  class NBCollManager
  {
  public:
    /* ---------------- */
    /* external API     */
    /* ---------------- */

    static void initialize ();
    static NBCollManager * instance();

    NBColl<T_Mcast> * find (NBTag tag, int id); /* find an existing instance */
    NBColl<T_Mcast> * allocate (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> *, NBTag tag);
    void     multisend_reg (NBTag tag,T_Mcast *mcast_iface);

  private:
    /* ------------ */
    /* data members */
    /* ------------ */

    Vector<NBColl<T_Mcast> *>        * _taglist[MAXTAG];
    static NBCollManager    * _instance; 

  private:
    /* ------------ */
    /* constructors */
    /* ------------ */

    NBCollManager (void);
    void * operator new (size_t, void * addr) { return addr; }
  };
};










#endif
