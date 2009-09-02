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

#include "NBColl.h"
#include "Barrier.h"
#include "Allgather.h"
#include "Allgatherv.h"
#include "BinomBcast.h"
#include "ScBcast.h"
#include "Allreduce.h"
#include "Scatter.h"


#include <stdio.h>
#include <string.h>
#include <assert.h>


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

/* ************************************************************************ */
/*                    initialize the factory                                */
/* ************************************************************************ */
template <class T_Mcast>
void TSPColl::NBCollFactory<T_Mcast>::initialize ()
{
  // TSPColl::CollExchange::amsend_reg ();
  // TSPColl::Scatter::amsend_reg ();
  // TSPColl::Gather::amsend_reg ();
}
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
/*                     Collective factory                                   */
/* ************************************************************************ */
template <class T_Mcast>
TSPColl::NBColl<T_Mcast> * 
TSPColl::NBCollFactory<T_Mcast>::create (XMI::Geometry::Geometry<XMI_GEOMETRY_CLASS> * comm, NBTag tag, int instID)
{
  switch (tag)
    {
    case BarrierTag:
      {
	NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(Barrier<T_Mcast>));
	assert (b != NULL);
	memset (b, 0, sizeof(Barrier<T_Mcast>));
	new (b) Barrier<T_Mcast> (comm, tag, instID, 0);
	return b;
      }
    case AllgatherTag:
      {
	NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(Allgather<T_Mcast>));
	assert (b != NULL);
	memset (b, 0, sizeof(Allgather<T_Mcast>));
	new (b) Allgather<T_Mcast> (comm, tag, instID, 0);
	return b;
      }
    case AllgathervTag:
      {
	NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(Allgatherv<T_Mcast>));
	assert (b != NULL);
	memset (b, 0, sizeof(Allgatherv<T_Mcast>));
	new (b) Allgatherv<T_Mcast> (comm, tag, instID, 0);
	return b;
      }
    case BcastTag:
      {
	NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(BinomBcast<T_Mcast>));
	assert (b != NULL);
	memset (b, 0, sizeof(BinomBcast<T_Mcast>));
	new (b) BinomBcast<T_Mcast> (comm, tag, instID, 0);
	return b;
      }
    case BcastTag2:
      {
	NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(ScBcast<T_Mcast>));
	assert (b != NULL);
	memset (b, 0, sizeof(ScBcast<T_Mcast>));
	new (b) ScBcast<T_Mcast> (comm, tag, instID, 0);
	return b;
      }
    case ShortAllreduceTag:
      {
        NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(Allreduce::Short<T_Mcast>));
        assert (b != NULL);
	memset (b, 0, sizeof(Allreduce::Short<T_Mcast>));
        new (b) Allreduce::Short<T_Mcast> (comm, tag, instID, 0);
        return b;
      }
    case LongAllreduceTag:
      {
        NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(Allreduce::Long<T_Mcast>));
        assert (b != NULL);
	memset (b, 0, sizeof(Allreduce::Long<T_Mcast>));
        new (b) Allreduce::Long<T_Mcast> (comm, tag, instID, 0);
        return b;
      }
    case ScatterTag:
      {
	NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(Scatter<T_Mcast>));
        assert (b != NULL);
	memset (b, 0, sizeof(Scatter<T_Mcast>));
        new (b) Scatter<T_Mcast> (comm, tag, instID, 0);
        return b;
      }
    case ScattervTag:
      {
	NBColl<T_Mcast> * b = (NBColl<T_Mcast> *)malloc (sizeof(Scatterv<T_Mcast>));
        assert (b != NULL);
	memset (b, 0, sizeof(Scatterv<T_Mcast>));
        new (b) Scatterv<T_Mcast> (comm, tag, instID, 0);
        return b;
      }
    case GatherTag:
    case GathervTag:
    default:
      {
	assert (0);
      }
    }
  return (NBColl<T_Mcast> *) NULL;
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

