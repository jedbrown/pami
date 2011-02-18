/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/Algorithm.h
 * \brief ???
 */

#ifndef __algorithms_geometry_Algorithm_h__
#define __algorithms_geometry_Algorithm_h__

#include <map>
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "components/memory/MemoryAllocator.h"
#include "algorithms/geometry/UnexpBarrierQueueElement.h"

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

#undef TRACE_ERR2
#define TRACE_ERR2(x) //fprintf x


extern pami_geometry_t mapidtogeometry (pami_context_t context, int comm);

namespace PAMI
{
  namespace Geometry
  {
    // This class manages "Algorithms", which consist of a geometry
    // object and an algorithm to go along with that object
    template <class T_Geometry>
    class Algorithm
    {
      public:
        Algorithm<T_Geometry>() {}

        inline void metadata(pami_metadata_t   *mdata)
        {
          TRACE_ERR((stderr, "<%p>Algorithm::metadata() factory %p\n", this , _factory));
          _factory->metadata(mdata);
        }
        inline pami_result_t generate(pami_xfer_t *xfer)
        {
          TRACE_ERR((stderr, "<%p>Algorithm::generate() factory %p\n", this, _factory));
	  CCMI::Executor::Composite *exec = _factory->generate((pami_geometry_t)_geometry,
							       (void*) xfer);
	  
	  //As, the factories may override completion callbacks, 
	  //so we dont reset completion callback here. Factory
	  //responsible to call the application callback.
          if (exec)
	  {
	    //  exec->setDoneCallback(xfer->cb_done, xfer->cookie);
	    exec->start();
	  }
          return PAMI_SUCCESS;
        }

        inline pami_result_t dispatch_set(size_t                           dispatch,
                                          pami_dispatch_callback_function  fn,
                                          void                            *cookie,
                                          pami_collective_hint_t           options)
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
      static const int MAX_NUM_ALGO = 16;
      public:
        AlgoLists():
            _num_algo(0),
            _num_algo_check(0)
        {
          TRACE_ERR2((stderr, "<%p>AlgoLists()\n", this));
          // Clear the algorithm list
          memset(&_algo_list[0], 0, sizeof(_algo_list));
          memset(&_algo_list_check[0], 0, sizeof(_algo_list_check));
          memset(&_algo_list_store[0], 0, sizeof(_algo_list_store));
          memset(&_algo_list_check_store[0], 0, sizeof(_algo_list_check_store));
        }
        inline pami_result_t addCollective(CCMI::Adaptor::CollectiveProtocolFactory *factory,
                                           T_Geometry                               *geometry,
                                           size_t                                    context_id)
        {
          TRACE_ERR((stderr, "<%p>AlgoLists::addCollective() _num_algo=%u, factory=%p, geometry=%p\n", this, _num_algo, factory, geometry));
          _algo_list_store[_num_algo]._factory  = factory;
          _algo_list_store[_num_algo]._geometry = geometry;
          _algo_list[_num_algo]                 = &_algo_list_store[_num_algo];
          _num_algo++;
          return PAMI_SUCCESS;
        }
        inline pami_result_t rmCollective(CCMI::Adaptor::CollectiveProtocolFactory *factory,
                                           T_Geometry                               *geometry,
                                           size_t                                    context_id)
        {
	  int i;
	  for (i = 0; i < _num_algo; ++i)
	  {
	    if (_algo_list[i]->_factory == factory)
	    {
	      PAMI_assertf(_algo_list_store[i]._factory == factory,
			"Internal consistency error on algorithm list");
	      size_t n = _num_algo - i - 1;
	      if (n)
	      {
	        memcpy(&_algo_list[i], &_algo_list[i + 1], n * sizeof(_algo_list[0]));
	        memcpy(&_algo_list_store[i], &_algo_list_store[i + 1], n * sizeof(_algo_list_store[0]));
	      }
	      --_num_algo;
	    }
	  }
          return PAMI_SUCCESS;
        }
        inline pami_result_t addCollectiveCheck(CCMI::Adaptor::CollectiveProtocolFactory  *factory,
                                                T_Geometry                                *geometry,
                                                size_t                                     context_id)
        {
          TRACE_ERR((stderr, "<%p>AlgoLists::addCollectiveCheck() _num_algo_check=%u, factory=%p, geometry=%p\n", this, _num_algo_check, factory, geometry));
          _algo_list_check_store[_num_algo_check]._factory  = factory;
          _algo_list_check_store[_num_algo_check]._geometry = geometry;
          _algo_list_check[_num_algo_check]                 = &_algo_list_check_store[_num_algo_check];
          _num_algo_check++;
          return PAMI_SUCCESS;
        }
        inline pami_result_t lengths(size_t             *lists_lengths)
        {
          TRACE_ERR((stderr, "<%p>AlgoLists::lengths() _num_algo %d, _num_algo_check %d\n", this, _num_algo,_num_algo_check));
          lists_lengths[0] = _num_algo;
          lists_lengths[1] = _num_algo_check;
          return PAMI_SUCCESS;
        }
        int                     _num_algo;
        int                     _num_algo_check;
        Algorithm<T_Geometry>  *_algo_list[MAX_NUM_ALGO];
        Algorithm<T_Geometry>  *_algo_list_check[MAX_NUM_ALGO];
        Algorithm<T_Geometry>   _algo_list_store[MAX_NUM_ALGO];
        Algorithm<T_Geometry>   _algo_list_check_store[MAX_NUM_ALGO];
    };
  };
};




#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
