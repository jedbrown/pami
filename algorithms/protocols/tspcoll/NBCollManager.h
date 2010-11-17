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
#include "components/devices/generic/Device.h"

namespace TSPColl
{

  /* *********************************************************** */
  /*           simple vector class                               */
  /* *********************************************************** */

  template<class T> class Vector
  {
    public:
      void * operator new (size_t, void * addr) { return addr; }
      Vector()
      {
        _size = _max = 0;
        _v = NULL;
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
      void          initialize ();
      void          setGenericDevice(PAMI::Device::Generic::Device *g);
      NBColl<T_NI> *find(NBTag tag, int id); /* find an existing instance */
      void         *find_ue(NBTag tag, int id); /* find an existing instance */
      NBColl<T_NI> *allocate(PAMI_GEOMETRY_CLASS *, NBTag tag, int id);
      void          allocate_ue(NBTag tag, int id, void*);
      void          delete_ue(NBTag tag, int id);
      void          multisend_reg(NBTag tag, T_NI *p2p_iface);
      pami_result_t postWork(PAMI::Device::Generic::GenericThread *work);
    private:
      /* ------------ */
      /* data members */
      /* ------------ */
      Vector<NBColl<T_NI> *>        * _taglist[MAXTAG];
      Vector<void *>                * _ue_taglist[MAXTAG];
      NBCollFactory<T_NI>             _factory;
      PAMI::Device::Generic::Device * _genericDevice;
    private:
  };


  template <class T_NI>
  void
  TSPColl::NBCollManager<T_NI>::multisend_reg (NBTag tag, T_NI *p2p_iface)
  {
    switch (tag)
      {
        case BarrierTag:
        {
          TSPColl::Barrier<T_NI>::amsend_reg(p2p_iface, this);
          break;
        }
        case BarrierUETag:
        {
          TSPColl::BarrierUE<T_NI>::amsend_reg(p2p_iface, this);
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
          PAMI_abort();
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
    PAMI_assert(idx >= 0);

    if (idx >= _max)
      {
        int oldmax = _max;
        _max = MAXOF((2 * _max + 1), idx + 1);
        _v = (T*) realloc (_v, _max * sizeof(T*));

        if (_v == NULL) CCMI_FATALERROR(-1, "Memory error");

        memset (_v + oldmax, 0, sizeof(T) * (_max - oldmax));
      }

    if (idx >= _size) _size = idx + 1;

    return _v[idx];
  }

//TSPColl::NBCollManager * TSPColl::NBCollManager::_instance = NULL;

  /* ************************************************************************ */
  /*          NBColl life cycle manager: singleton initializer                */
  /* ************************************************************************ */
  template <class T_NI>
  void TSPColl::NBCollManager<T_NI>::initialize (void)
  {
    for (int i = 0; i < MAXTAG; i++)
      {
        _taglist [i] = (Vector<NBColl<T_NI> *> *) malloc (sizeof(Vector<NBColl<T_NI> *>));
        new (_taglist[i]) Vector<NBColl<T_NI> *> ();

        _ue_taglist [i] = (Vector<void *> *) malloc (sizeof(Vector<void *>));
        new (_ue_taglist[i]) Vector<void *> ();
      }
  }

  template <class T_NI>
  void TSPColl::NBCollManager<T_NI>::setGenericDevice(PAMI::Device::Generic::Device *g)
  {
    _genericDevice = g;
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
    PAMI_assert(0 <= tag && tag < MAXTAG);
    return (*_taglist[tag])[id];
  }

  template <class T_NI>
  void * TSPColl::NBCollManager<T_NI>::find_ue (NBTag tag, int id)
  {
    PAMI_assert(0 <= tag && tag < MAXTAG);
    return (*_ue_taglist[tag])[id];
  }

  /* ************************************************************************ */
  /*            reserve an instance or create a new one                       */
  /* ************************************************************************ */
  template <class T_NI>
  TSPColl::NBColl<T_NI> *
  TSPColl::NBCollManager<T_NI>::allocate (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int id)
  {
    PAMI_assert(0 <= tag && tag < MAXTAG);
//    int nextID = _taglist[tag]->size();
    int nextID = id;
    NBColl<T_NI> * retval = _factory.create (comm, tag, nextID);
    PAMI_assert((*_taglist[tag])[nextID] == NULL);
    (*_taglist[tag])[nextID] = retval;
    return retval;
  }


  /* ************************************************************************ */
  /*            allocate a dummy collective in the ue queue                   */
  /* ************************************************************************ */

  template <class T_NI>
  void TSPColl::NBCollManager<T_NI>::allocate_ue (NBTag tag, int id, void *val)
  {
    PAMI_assert(0 <= tag && tag < MAXTAG);
    (*_ue_taglist[tag])[id] = val;
  }

  template <class T_NI>
  void TSPColl::NBCollManager<T_NI>::delete_ue (NBTag tag, int id)
  {
    PAMI_assert(0 <= tag && tag < MAXTAG);
    (*_ue_taglist[tag])[id] = NULL;
  }

  template <class T_NI>
  pami_result_t TSPColl::NBCollManager<T_NI>::postWork(PAMI::Device::Generic::GenericThread *work)
  {
    _genericDevice->postThread(work);
    return PAMI_SUCCESS;
  }


};


#include "algorithms/protocols/tspcoll/NBCallbacks.h"


#endif
