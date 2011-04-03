/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/ClassRouteId.h
 * \brief ???
 */

#ifndef __algorithms_geometry_ClassRouteId_h__
#define __algorithms_geometry_ClassRouteId_h__

#include "algorithms/geometry/Algorithm.h"

namespace PAMI
{
  namespace Geometry
  {


    template <class T_Geometry>
    class ClassRouteId
    {
      static const size_t _max_reductions = 16;

      typedef void (*cr_event_function)(pami_context_t  context,
                                        void           *cookie,
                                        uint64_t       *reduce_result,
                                        T_Geometry     *geometry,
                                        pami_result_t   result );


    public:
      ClassRouteId<T_Geometry>(Algorithm<T_Geometry>    *ar_algo,
                               T_Geometry               *geometry,
                               uint64_t                 *bitmask,
                               size_t                    count,
                               cr_event_function         result_cb_done,
                               void                     *result_cookie,
                               pami_event_function       user_cb_done,
                               void                     *user_cookie,
                               bool                      free_bitmask = false):
        _ar_algo(ar_algo),
        _geometry(geometry),
        _bitmask(bitmask),
        _count(count),
        _result_cb_done(result_cb_done),
        _result_cookie(result_cookie),
        _user_cb_done(user_cb_done),
        _user_cookie(user_cookie),
        _free_bitmask(free_bitmask)
        
        {
          PAMI_assert(count <= _max_reductions);
        }

      static void get_cr_done(pami_context_t   context,
                              void           * cookie,
                              pami_result_t    result )
        {
          ClassRouteId     *c        = (ClassRouteId *)cookie;
          c->_result_cb_done(context,c->_result_cookie,c->_bitmask,c->_geometry,result);
          c->_user_cb_done(context, c->_user_cookie, result);
          if(c->_free_bitmask)
            __global.heap_mm->free(c->_bitmask);
          free(c);
        }

      static void get_classroute(pami_context_t   context,
                                 void           * cookie,
                                 pami_result_t    result )
        {
          pami_xfer_t                          ar;
          ClassRouteId                        *c  = (ClassRouteId *)cookie;
          ar.cb_done                              = get_cr_done;
          ar.cookie                               = c;
          // algorithm not needed here
          memset(&ar.options,0,sizeof(ar.options));
          ar.cmd.xfer_allreduce.sndbuf            = (char*)c->_bitmask;
          ar.cmd.xfer_allreduce.stype             = PAMI_TYPE_CONTIGUOUS;
          ar.cmd.xfer_allreduce.stypecount        = sizeof(uint64_t)*c->_count;
          ar.cmd.xfer_allreduce.rcvbuf            = (char*)c->_bitmask;
          ar.cmd.xfer_allreduce.rtype             = PAMI_TYPE_CONTIGUOUS;
          ar.cmd.xfer_allreduce.rtypecount        = sizeof(uint64_t)*c->_count;
          ar.cmd.xfer_allreduce.dt                = PAMI_UNSIGNED_LONG_LONG;
          ar.cmd.xfer_allreduce.op                = PAMI_BAND;
          c->_ar_algo->generate(&ar);
        }
    private:
      Algorithm<T_Geometry>  *_ar_algo;
      T_Geometry             *_geometry;
      uint64_t               *_bitmask;
      size_t                  _count;
      cr_event_function       _result_cb_done;
      void                   *_result_cookie;
      pami_event_function     _user_cb_done;
      void                   *_user_cookie;
      bool                    _free_bitmask;
    };

  };
};
#endif
