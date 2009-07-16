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

#ifndef __tspcoll_nbcoll_h__
#define __tspcoll_nbcoll_h__

#include "interface/MultiSendOld.h"

namespace TSPColl
{
  class Communicator;
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
  class NBColl
  {
  public:
    NBColl (Communicator * comm, NBTag tag, int instID,
	    void (*cb_complete)(void *), void *arg);

  public:
    virtual void  kick  (CCMI::MultiSend::MulticastInterface *mcast_iface) {};
    virtual bool isdone () const { return false; } /* check completion */
    int instID () const { return _instID; }
    int tag    () const { return _tag;    }
    virtual void setComplete (void (*cb_complete)(void *), void *arg);

  protected:
    Communicator * _comm;
    NBTag          _tag;
    int            _instID;
    void        (* _cb_complete) (void *);
    void         * _arg;
  };

  /* *********************************************************** */
  /*    A factory for generating non-blocking coll. instances    */
  /* *********************************************************** */
  class NBCollFactory
  {
  public:
    static NBColl * create (Communicator * comm, NBTag tag, int id);
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
  class NBCollManager
  {
  public:
    /* ---------------- */
    /* external API     */
    /* ---------------- */

    static void initialize ();
    static NBCollManager * instance();

    NBColl * find (NBTag tag, int id); /* find an existing instance */
    NBColl * allocate (Communicator *, NBTag tag);
    void     multisend_reg (NBTag tag,CCMI::MultiSend::MulticastInterface *mcast_iface);

  private:
    /* ------------ */
    /* data members */
    /* ------------ */

    Vector<NBColl *>        * _taglist[MAXTAG];
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
