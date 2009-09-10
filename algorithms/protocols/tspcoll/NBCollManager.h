#ifndef __tspcoll_nbcollmanager_h__
#define __tspcoll_nbcollmanager_h__


#include "NBCollFactory.h"

namespace TSPColl
{

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

  
  template <class T_Mcast>
  void
  TSPColl::NBCollManager<T_Mcast>::multisend_reg (NBTag tag,T_Mcast *mcast_iface)
  {
    switch (tag)
        {
            case BarrierTag:
            {
              TSPColl::Barrier<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case AllgatherTag:
            {
              TSPColl::Allgather<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case AllgathervTag:
            {
              TSPColl::Allgatherv<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case BcastTag:
            {
              TSPColl::BinomBcast<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case BcastTag2:
            {
              TSPColl::ScBcast<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case ShortAllreduceTag:
            {
              TSPColl::Allreduce::Short<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case LongAllreduceTag:
            {
              TSPColl::Allreduce::Long<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case ScatterTag:
            {
              TSPColl::Scatter<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case ScattervTag:
            {
              TSPColl::Scatterv<T_Mcast>::amsend_reg(mcast_iface);
              break;
            }
            case GatherTag:
            case GathervTag:
            default:
            {
              assert (0);
            }
        }
  }



/* ************************************************************************ */
/*                              vector                                      */
/* ************************************************************************ */

#define MAXOF(a,b) (((a)>(b))?(a):(b))
  template <class T>
  T & TSPColl::Vector<T>::operator[](int idx)
  {
    assert (idx >= 0);
    if (idx>=_max)
        {
          int oldmax = _max;
          _max = MAXOF((2*_max+1),idx+1);
          _v = (T*) realloc (_v, _max * sizeof(T*));
          if (_v == NULL) CCMI_FATALERROR(-1, "Memory error");
          memset (_v + oldmax, 0, sizeof(T) * (_max - oldmax));
        }
    if (idx>=_size) _size = idx+1;
    return _v[idx];
  }

//TSPColl::NBCollManager * TSPColl::NBCollManager::_instance = NULL;

/* ************************************************************************ */
/*          NBColl life cycle manager: singleton initializer                */
/* ************************************************************************ */
  template <class T_Mcast>
  void TSPColl::NBCollManager<T_Mcast>::initialize (void)
  {
    _instance = (NBCollManager<T_Mcast> *)malloc (sizeof(NBCollManager));
    assert (_instance);
    new (_instance) NBCollManager<T_Mcast>();
    NBCollFactory<T_Mcast>::initialize();
  }

  template <class T_Mcast>
  TSPColl::NBCollManager<T_Mcast> * TSPColl::NBCollManager<T_Mcast>::instance()
  {
    if (_instance==NULL) initialize();
    return _instance;
  }

/* ************************************************************************ */
/*           NBColl life cycle manager: constructor                         */
/* ************************************************************************ */
  template <class T_Mcast>
  TSPColl::NBCollManager<T_Mcast>::NBCollManager (void)
  {
    for (int i=0; i<MAXTAG; i++)
        {
          _taglist [i] = (Vector<NBColl<T_Mcast> *> *) malloc (sizeof(Vector<NBColl<T_Mcast> *>));
          new (_taglist[i]) Vector<NBColl<T_Mcast> *> ();
        }
  }

/* ************************************************************************ */
/*              find an instance                                            */
/* ************************************************************************ */
  template <class T_Mcast>
  TSPColl::NBColl<T_Mcast> * 
  TSPColl::NBCollManager<T_Mcast>::find (NBTag tag, int id)
  {
    assert (0 <= tag && tag < MAXTAG);
    return (*_taglist[tag])[id];
  }

/* ************************************************************************ */
/*            reserve an instance or create a new one                       */
/* ************************************************************************ */
  template <class T_Mcast>
  TSPColl::NBColl<T_Mcast> * 
  TSPColl::NBCollManager<T_Mcast>::allocate (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * comm, NBTag tag)
  {
    assert (0 <= tag && tag < MAXTAG);
    int nextID = _taglist[tag]->size();
    NBColl<T_Mcast> * retval = NBCollFactory<T_Mcast>::create (comm, tag, nextID);
    (*_taglist[tag])[nextID] = retval;
    return retval;
  }
};










#endif
