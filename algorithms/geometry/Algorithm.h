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

#include <vector>
#include "algorithms/protocols/CollectiveProtocolFactory.h"
#include "components/memory/MemoryAllocator.h"
#include "algorithms/geometry/UnexpBarrierQueueElement.h"

extern pami_geometry_t mapidtogeometry (pami_context_t context, int comm);

namespace PAMI{namespace Geometry{
  typedef CCMI::Adaptor::CollectiveProtocolFactory Factory;

    // This class manages "Algorithms", which consist of a geometry
    // object and an algorithm to go along with that object
    template <class T_Geometry>
    class Algorithm
    {
      public:
    Algorithm<T_Geometry>(Factory        *factory,
                          T_Geometry     *geometry):
      _factory(factory),
      _geometry(geometry)
      {}
        inline void metadata(pami_metadata_t   *mdata)
        {
          _factory->metadata(mdata);
        }
        inline pami_result_t generate(pami_xfer_t *xfer)
        {
          CCMI::Executor::Composite *exec = _factory->generate((pami_geometry_t)_geometry,
                                                               (void*) xfer);
        if (exec)exec->start();
          return PAMI_SUCCESS;
        }
        inline pami_result_t dispatch_set(size_t                           dispatch,
                                          pami_dispatch_callback_function  fn,
                                          void                            *cookie,
                                          pami_collective_hint_t           options)
        {
          DispatchInfo info;
          info.fn = fn;
          info.cookie = cookie;
          info.options = options;
          _geometry->setDispatch(dispatch, &info);
//          _factory->setAsyncInfo(false,
//                                 fn,
//                                 mapidtogeometry);
          return PAMI_SUCCESS;
        }

    Factory          *_factory;
        T_Geometry                                                  *_geometry;
    };

    // This class manages lists of algorithms
  // It assumes the order of insertion into the list is:
  // All algorithms from context 0 are added
  // Then all algorithms from context 1.
  // We can make this more general by implementing a dynamically
  // growing 2d array of algorithm objects
    template <class T_Geometry>
    class AlgoLists
    {
    typedef std::vector<Algorithm<T_Geometry> *> Alist;
      public:
    inline AlgoLists(size_t num_contexts):
      _num_contexts(num_contexts),
      _cur_count(0),
      _total_count(0),
      _cur_count_check(0),
      _total_count_check(0)
        {
      }
    ~AlgoLists()
      {
        typename Alist::iterator it;
        for(it=_algo_array.begin(); it<_algo_array.end(); it++)
          {
            __global.heap_mm->free(*it);
          }

        for(it=_algo_array_check.begin(); it<_algo_array_check.end();it++)
          {
            __global.heap_mm->free(*it);
          }
      }

    static void resetFactoryCache (pami_context_t   ctxt,
                                   void           * factory,
                                   pami_result_t    result)
    {
      CCMI::Adaptor::CollectiveProtocolFactory *cf =
        (CCMI::Adaptor::CollectiveProtocolFactory *) factory;
      cf->clearCache();
        }

    inline pami_result_t addCollectiveList(Alist                 &theList,
                                           size_t                 context_id,
                                           Factory               *factory,
                                           T_Geometry                               *geometry,
                                           size_t                &cur_count,
                                           size_t                &total_count)
        {
        size_t idx;
        if(context_id == 0)
          {
          idx                         = total_count++;
          Algorithm<T_Geometry> *p;
          pami_result_t rc = __global.heap_mm->memalign((void**)&p,
                                                        0,
                                                        sizeof(Algorithm<T_Geometry>)*_num_contexts);
          if(rc != PAMI_SUCCESS || p == NULL)
            return rc;
          theList.push_back(p);
        }
        else
              {
          idx                         = cur_count%total_count;
          cur_count++;
              }
        Algorithm<T_Geometry> *elem = &theList[idx][context_id];
        new(elem) Algorithm<T_Geometry>(factory,geometry);
        geometry->setCleanupCallback(resetFactoryCache, factory);
        return PAMI_SUCCESS;
            }
    inline pami_result_t rmCollectiveList(Alist      *theList,
                                          Factory    *factory,
                                          T_Geometry *geometry)
      {
        typename Alist::iterator it;
        for(it=theList->begin(); it<theList->end();it++)
        {
          if ((*it)->_factory == factory)
          {
            theList->erase(it);
            break;
          }
        }
        if(it == theList->end())
          return PAMI_ERROR;
        else
          return PAMI_SUCCESS;
        }
    inline pami_result_t addCollective(Factory        *factory,
                                                T_Geometry                                *geometry,
                                       pami_context_t  context,
                                                size_t                                     context_id)
        {
        return addCollectiveList(_algo_array,
                                 context_id,
                                 factory,
                                 geometry,
                                 _cur_count,
                                 _total_count);
        }
    inline pami_result_t addCollectiveCheck(Factory        *factory,
                                               T_Geometry                               *geometry,
                                            pami_context_t  context,
                                               size_t                                    context_id)
        {
        return addCollectiveList(_algo_array_check,
                                 context_id,
                                 factory,
                                 geometry,
                                 _cur_count_check,
                                 _total_count_check);
            }
    inline pami_result_t rmCollective(Factory        *factory,
                                      T_Geometry     *geometry)
      {
        return rmCollectiveList(&_algo_array,
				factory,
                                geometry);
          }
    inline pami_result_t rmCollectiveCheck(Factory        *factory,
                                           T_Geometry     *geometry)
      {
        return rmCollectiveList(&_algo_array_check,
				factory,
                                geometry);
        }
        inline pami_result_t lengths(size_t             *lists_lengths)
        {
        // all lists should have the same size
        lists_lengths[0] = _algo_array.size();
        lists_lengths[1] = _algo_array_check.size();
          return PAMI_SUCCESS;
        }
    Alist                  _algo_array;
    Alist                  _algo_array_check;
    size_t                 _num_contexts;
    size_t                 _cur_count;
    size_t                 _total_count;
    size_t                 _cur_count_check;
    size_t                 _total_count_check;

};

};}; //namespaces




#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
