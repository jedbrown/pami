/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/tspcoll/NBCollManager.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_NBCollManager_h__
#define __algorithms_protocols_tspcoll_NBCollManager_h__


#include "algorithms/protocols/tspcoll/NBCollFactory.h"

namespace TSPColl
{

  /* *********************************************************** */
  /*           simple vector class                               */
  /* *********************************************************** */

  template<class T> class Vector
  {
  public:
    void * operator new (size_t, void * addr) { return addr; }
    Vector() {
      _size = _max = 0; _v = NULL;
    }
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
  template <class T_NI>
  class NBCollManager
  {
  public:
    /* ---------------- */
    /* external API     */
    /* ---------------- */

    NBCollManager(void);
    void initialize ();
    NBColl<T_NI> * find (NBTag tag, int id); /* find an existing instance */
    NBColl<T_NI> * allocate (PAMI_GEOMETRY_CLASS *, NBTag tag);
    void     multisend_reg (NBTag tag,T_NI *p2p_iface);

  private:
    /* ------------ */
    /* data members */
    /* ------------ */

    Vector<NBColl<T_NI> *>        * _taglist[MAXTAG];
    NBCollFactory<T_NI>             _factory;
  private:
  };


  template <class T_NI>
  void
  TSPColl::NBCollManager<T_NI>::multisend_reg (NBTag tag,T_NI *p2p_iface)
  {
    switch (tag)
        {
            case BarrierTag:
            {
              TSPColl::Barrier<T_NI>::amsend_reg(p2p_iface, this);
              break;
            }
            case AllgatherTag:
            {
              TSPColl::Allgather<T_NI>::amsend_reg(p2p_iface, this);
              break;
            }
            case AllgathervTag:
            {
              TSPColl::Allgatherv<T_NI>::amsend_reg(p2p_iface, this);
              break;
            }
            case BcastTag:
            {
              TSPColl::BinomBcast<T_NI>::amsend_reg(p2p_iface, this);
              break;
            }
            case BcastTag2:
            {
              TSPColl::ScBcast<T_NI>::amsend_reg(p2p_iface, this);
              break;
            }
            case ShortAllreduceTag:
            {
              TSPColl::Allreduce::Short<T_NI>::amsend_reg(p2p_iface, this);
              break;
            }
            case LongAllreduceTag:
            {
              TSPColl::Allreduce::Long<T_NI>::amsend_reg(p2p_iface, this);
              break;
            }
            case ScatterTag:
            {
              TSPColl::Scatter<T_NI>::amsend_reg(p2p_iface, this);
              break;
            }
            case ScattervTag:
            {
              TSPColl::Scatterv<T_NI>::amsend_reg(p2p_iface, this);
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
  template <class T_NI>
  void TSPColl::NBCollManager<T_NI>::initialize (void)
  {
    for (int i=0; i<MAXTAG; i++)
      {
        _taglist [i] = (Vector<NBColl<T_NI> *> *) malloc (sizeof(Vector<NBColl<T_NI> *>));
        new (_taglist[i]) Vector<NBColl<T_NI> *> ();
      }
  }


/* ************************************************************************ */
/*           NBColl life cycle manager: constructor                         */
/* ************************************************************************ */
  template <class T_NI>
  TSPColl::NBCollManager<T_NI>::NBCollManager (void)
  {
  }

/* ************************************************************************ */
/*              find an instance                                            */
/* ************************************************************************ */
  template <class T_NI>
  TSPColl::NBColl<T_NI> *
  TSPColl::NBCollManager<T_NI>::find (NBTag tag, int id)
  {
    assert (0 <= tag && tag < MAXTAG);
    return (*_taglist[tag])[id];
  }

/* ************************************************************************ */
/*            reserve an instance or create a new one                       */
/* ************************************************************************ */
  template <class T_NI>
  TSPColl::NBColl<T_NI> *
  TSPColl::NBCollManager<T_NI>::allocate (PAMI_GEOMETRY_CLASS * comm, NBTag tag)
  {
    assert (0 <= tag && tag < MAXTAG);
    int nextID = _taglist[tag]->size();
    NBColl<T_NI> * retval = _factory.create (comm, tag, nextID);
    (*_taglist[tag])[nextID] = retval;
    return retval;
  }
};


#include "algorithms/protocols/tspcoll/NBCallbacks.h"


#endif
