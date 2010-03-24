/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/tspcoll/NBCollFactory.h
 * \brief ???
 */

#ifndef __algorithms_protocols_tspcoll_NBCollFactory_h__
#define __algorithms_protocols_tspcoll_NBCollFactory_h__

#include "algorithms/protocols/tspcoll/NBColl.h"
#include "algorithms/protocols/tspcoll/Barrier.h"
#include "algorithms/protocols/tspcoll/Allgather.h"
#include "algorithms/protocols/tspcoll/Allreduce.h"
#include "algorithms/protocols/tspcoll/Allgatherv.h"
#include "algorithms/protocols/tspcoll/BinomBcast.h"
#include "algorithms/protocols/tspcoll/ScBcast.h"

#include "algorithms/protocols/tspcoll/Scatter.h"

namespace TSPColl
{
  template <class T_Mcast>
  class NBCollFactory
  {
  public:
    NBColl<T_Mcast> * create (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int id);
    void              initialize();
  };
};


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

/* ************************************************************************ */
/*                     Collective factory                                   */
/* ************************************************************************ */
template <class T_Mcast>
TSPColl::NBColl<T_Mcast> *
TSPColl::NBCollFactory<T_Mcast>::create (PAMI_GEOMETRY_CLASS * comm, NBTag tag, int instID)
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


#endif
