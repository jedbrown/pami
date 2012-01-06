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
  Algorithm<T_Geometry>() {}
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
    return PAMI_SUCCESS;
  }
  Factory    *_factory;
  T_Geometry *_geometry;
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
