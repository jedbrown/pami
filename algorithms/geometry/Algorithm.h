/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/Geometry.h
 * \brief ???
 */

#ifndef __algorithms_geometry_Algorithm_h__
#define __algorithms_geometry_Algorithm_h__

#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include <map>

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#undef TRACE_ERR2
#define TRACE_ERR2(x) //fprintf x


namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;
  extern std::map<unsigned, pami_geometry_t> cached_geometry;

  namespace Geometry
  {
    // This class manages "Algorithms", which consist of a geometry
    // object and an algorithm to go along with that object
    template <class T_Geometry>
      class Algorithm
      {
      public:
	Algorithm<T_Geometry>(){}

	inline void metadata(pami_metadata_t   *mdata)
	  {
	    TRACE_ERR((stderr, "<%p>Algorithm::metadata() factory %p\n", this ,_factory));
	    _factory->metadata(mdata);
	  }
	inline pami_result_t generate(pami_xfer_t *xfer)
	  {
	    TRACE_ERR((stderr, "<%p>Algorithm::generate() factory %p\n", this, _factory));
	    CCMI::Executor::Composite *exec=_factory->generate((pami_geometry_t)_geometry,
							       (void*) xfer);
	    if(exec)
              {
                exec->setDoneCallback(xfer->cb_done, xfer->cookie);
                exec->start();
              }
	    return PAMI_SUCCESS;
	  }
	static pami_geometry_t mapidtogeometry (int comm)
	  {
	    pami_geometry_t g = geometry_map[comm];
	    TRACE_ERR((stderr, "<%p>Algorithm::mapidtogeometry()\n", g));
	    return g;
	  }

	inline pami_result_t dispatch_set(size_t                     dispatch,
					  pami_dispatch_callback_fn   fn,
					  void                     * cookie,
					  pami_collective_hint_t      options)
	  {
	    TRACE_ERR((stderr, "<%p>Algorithm::dispatch_set()\n", this));
	    _factory->setAsyncInfo(false,
				   fn,
				   mapidtogeometry);

	    return PAMI_SUCCESS;
	  }
	CCMI::Adaptor::CollectiveProtocolFactory                    *_factory;
	T_Geometry                                                  *_geometry;
      };

    // This class manages lists of algorithms
    template <class T_Geometry>
      class AlgoLists
      {
      public:
	AlgoLists():
	  _num_algo(0),
	  _num_algo_check(0)
	    {
	      TRACE_ERR2((stderr, "<%p>AlgoLists()\n", this));
	      // Clear the algorithm list
	      memset(&_algo_list[0], 0, sizeof(_algo_list));
	      memset(&_algo_list[0], 0, sizeof(_algo_list_check));
	      memset(&_algo_list_store[0], 0, sizeof(_algo_list_store));
	      memset(&_algo_list_check_store[0], 0, sizeof(_algo_list_check_store));
	    }
	  inline pami_result_t addCollective(CCMI::Adaptor::CollectiveProtocolFactory *factory,
					     T_Geometry                               *geometry,
					     size_t                                    context_id)
	    {
	      TRACE_ERR((stderr, "<%p>AlgoLists::addCollective() _num_algo=%u, factory=%p, geometry=%p\n", this, _num_algo,factory,geometry));
	      _algo_list_store[_num_algo]._factory  = factory;
	      _algo_list_store[_num_algo]._geometry = geometry;
	      _algo_list[_num_algo]                 = &_algo_list_store[_num_algo];
	      _num_algo++;
	      return PAMI_SUCCESS;
	    }
	  inline pami_result_t addCollectiveCheck(CCMI::Adaptor::CollectiveProtocolFactory  *factory,
						  T_Geometry                                *geometry,
						  size_t                                     context_id)
	    {
	      TRACE_ERR((stderr, "<%p>AlgoLists::addCollectiveCheck() _num_algo_check=%u, factory=%p, geometry=%p\n", this, _num_algo_check,factory,geometry));
	      _algo_list_check_store[_num_algo_check]._factory  = factory;
	      _algo_list_check_store[_num_algo_check]._geometry = geometry;
	      _algo_list_check[_num_algo_check]                 = &_algo_list_check_store[_num_algo_check];
	      _num_algo_check++;
	      return PAMI_SUCCESS;
	    }
	  inline pami_result_t lengths(size_t             *lists_lengths)
	    {
	      TRACE_ERR((stderr, "<%p>AlgoLists::lengths()\n", this));
	      lists_lengths[0] = _num_algo;
	      lists_lengths[1] = _num_algo_check;
	      return PAMI_SUCCESS;
	    }
	  int                     _num_algo;
	  int                     _num_algo_check;
	  Algorithm<T_Geometry>  *_algo_list[16];
	  Algorithm<T_Geometry>  *_algo_list_check[16];
	  Algorithm<T_Geometry>   _algo_list_store[16];
	  Algorithm<T_Geometry>   _algo_list_check_store[16];
      };
  };
};




#endif
