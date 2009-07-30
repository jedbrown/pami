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
TSPColl::NBColl::NBColl (Communicator * comm, NBTag tag, int instID,
			 void (*cb_complete)(void *), void *arg):
  _comm (comm), _tag (tag), _instID (instID),
  _cb_complete (cb_complete), _arg(arg)
{
}

/* ************************************************************************ */
/* ************************************************************************ */
void TSPColl::NBColl::setComplete (void (*cb_complete)(void *), void *arg)
{
  _cb_complete = cb_complete;
  _arg = arg;
}

/* ************************************************************************ */
/*                    initialize the factory                                */
/* ************************************************************************ */
void TSPColl::NBCollFactory::initialize ()
{
  // TSPColl::CollExchange::amsend_reg ();
  // TSPColl::Scatter::amsend_reg ();
  // TSPColl::Gather::amsend_reg ();
}

void
TSPColl::NBCollManager::multisend_reg (NBTag tag,CCMI::MultiSend::OldMulticastInterface *mcast_iface)
{
  switch (tag)
    {
    case BarrierTag:
      {
	TSPColl::Barrier::amsend_reg(mcast_iface);
	break;
      }
    case AllgatherTag:
      {
	TSPColl::Allgather::amsend_reg(mcast_iface);
	break;
      }
    case AllgathervTag:
      {
	TSPColl::Allgatherv::amsend_reg(mcast_iface);
	break;
      }
    case BcastTag:
      {
	TSPColl::BinomBcast::amsend_reg(mcast_iface);
	break;
      }
    case BcastTag2:
      {
	TSPColl::ScBcast::amsend_reg(mcast_iface);
	break;
      }
    case ShortAllreduceTag:
      {
	TSPColl::Allreduce::Short::amsend_reg(mcast_iface);
	break;
      }
    case LongAllreduceTag:
      {
	TSPColl::Allreduce::Long::amsend_reg(mcast_iface);
	break;
      }
    case ScatterTag:
      {
	TSPColl::Scatter::amsend_reg(mcast_iface);
	break;
      }
    case ScattervTag:
      {
	TSPColl::Scatterv::amsend_reg(mcast_iface);
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
TSPColl::NBColl * 
TSPColl::NBCollFactory::create (Communicator * comm, NBTag tag, int instID)
{
  switch (tag)
    {
    case BarrierTag:
      {
	NBColl * b = (NBColl *)malloc (sizeof(Barrier));
	assert (b != NULL);
	memset (b, 0, sizeof(Barrier));
	new (b) Barrier (comm, tag, instID, 0);
	return b;
      }
    case AllgatherTag:
      {
	NBColl * b = (NBColl *)malloc (sizeof(Allgather));
	assert (b != NULL);
	memset (b, 0, sizeof(Allgather));
	new (b) Allgather (comm, tag, instID, 0);
	return b;
      }
    case AllgathervTag:
      {
	NBColl * b = (NBColl *)malloc (sizeof(Allgatherv));
	assert (b != NULL);
	memset (b, 0, sizeof(Allgatherv));
	new (b) Allgatherv (comm, tag, instID, 0);
	return b;
      }
    case BcastTag:
      {
	NBColl * b = (NBColl *)malloc (sizeof(BinomBcast));
	assert (b != NULL);
	memset (b, 0, sizeof(BinomBcast));
	new (b) BinomBcast (comm, tag, instID, 0);
	return b;
      }
    case BcastTag2:
      {
	NBColl * b = (NBColl *)malloc (sizeof(ScBcast));
	assert (b != NULL);
	memset (b, 0, sizeof(ScBcast));
	new (b) ScBcast (comm, tag, instID, 0);
	return b;
      }
    case ShortAllreduceTag:
      {
        NBColl * b = (NBColl *)malloc (sizeof(Allreduce::Short));
        assert (b != NULL);
	memset (b, 0, sizeof(Allreduce::Short));
        new (b) Allreduce::Short (comm, tag, instID, 0);
        return b;
      }
    case LongAllreduceTag:
      {
        NBColl * b = (NBColl *)malloc (sizeof(Allreduce::Long));
        assert (b != NULL);
	memset (b, 0, sizeof(Allreduce::Long));
        new (b) Allreduce::Long (comm, tag, instID, 0);
        return b;
      }
    case ScatterTag:
      {
	NBColl * b = (NBColl *)malloc (sizeof(Scatter));
        assert (b != NULL);
	memset (b, 0, sizeof(Scatter));
        new (b) Scatter (comm, tag, instID, 0);
        return b;
      }
    case ScattervTag:
      {
	NBColl * b = (NBColl *)malloc (sizeof(Scatterv));
        assert (b != NULL);
	memset (b, 0, sizeof(Scatterv));
        new (b) Scatterv (comm, tag, instID, 0);
        return b;
      }
    case GatherTag:
    case GathervTag:
    default:
      {
	assert (0);
      }
    }
  return (NBColl *) NULL;
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
TSPColl::NBCollManager * TSPColl::NBCollManager::_instance = NULL;

/* ************************************************************************ */
/*          NBColl life cycle manager: singleton initializer                */
/* ************************************************************************ */
void TSPColl::NBCollManager::initialize (void)
{
  _instance = (NBCollManager *)malloc (sizeof(NBCollManager));
  assert (_instance);
  new (_instance) NBCollManager();
  NBCollFactory::initialize();
}
TSPColl::NBCollManager * TSPColl::NBCollManager::instance()
{
  if (_instance==NULL) initialize();
  return _instance;
}

/* ************************************************************************ */
/*           NBColl life cycle manager: constructor                         */
/* ************************************************************************ */
TSPColl::NBCollManager::NBCollManager (void)
{
  for (int i=0; i<MAXTAG; i++)
    {
      _taglist [i] = (Vector<NBColl *> *) malloc (sizeof(Vector<NBColl *>));
      new (_taglist[i]) Vector<NBColl *> ();
    }
}

/* ************************************************************************ */
/*              find an instance                                            */
/* ************************************************************************ */
TSPColl::NBColl * 
TSPColl::NBCollManager::find (NBTag tag, int id)
{
  assert (0 <= tag && tag < MAXTAG);
  return (*_taglist[tag])[id];
}

/* ************************************************************************ */
/*            reserve an instance or create a new one                       */
/* ************************************************************************ */
TSPColl::NBColl * 
TSPColl::NBCollManager::allocate (Communicator * comm, NBTag tag)
{
  assert (0 <= tag && tag < MAXTAG);
  int nextID = _taglist[tag]->size();
  NBColl * retval = NBCollFactory::create (comm, tag, nextID);
  (*_taglist[tag])[nextID] = retval;
  return retval;
}

