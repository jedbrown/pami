/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/FCACollRegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_FCACollRegistration_h__
#define __algorithms_geometry_FCACollRegistration_h__

#include "sys/pami.h"
#include "components/devices/fca/fcafunc.h"

namespace PAMI{namespace CollRegistration{namespace FCA{
template <class T_Geometry>
class FCARegistration : public CollRegistration <FCARegistration<T_Geometry>, T_Geometry >
{
public:
  class GeometryInfo
  {
    public:
    GeometryInfo():
      _dummy(0)
    {
    }
    int _dummy;
  };
  typedef PAMI::MemoryAllocator<sizeof(GeometryInfo),16> GIAllocator;
  inline FCARegistration(pami_client_t                        client,
                         pami_context_t                       context,
                         size_t                               context_id,
                         size_t                               client_id,
                         size_t                               num_contexts):
    CollRegistration <FCARegistration<T_Geometry>, T_Geometry > (),
    _client(client),
    _context(context),
    _context_id(context_id),
    _num_contexts(num_contexts),
    _client_id(client_id),
    _enabled(false)
  {
    if(num_contexts > 1) return;
    void * rc = FCA_Dlopen(0);
    if(!rc) return;
    else
      {
        // TODO:  Fill in FCA Init Spec
        // TODO:  print the FCA Banner and check FCA version
        FCA_Init(&_fca_init_spec, &_fca_context);
        _enabled=true;
      }
  }

  inline pami_result_t analyze_impl(size_t         context_id,
                                    T_Geometry    *geometry,
                                    uint64_t      *inout_val,
                                    int           *inout_nelem,
                                    int            phase)
  {
    if(!_enabled) return PAMI_SUCCESS;
    return PAMI_SUCCESS;
  }

  inline void freeGeomInfo(GeometryInfo *gi)
  {
    _geom_allocator.returnObject(gi);
  }
  static inline void cleanupCallback(pami_context_t ctxt,
                                     void          *data,
                                     pami_result_t  res)
  {
    GeometryInfo *gi = (GeometryInfo*) data;
    gi->_registration->freeGeomInfo(gi);
  }
private:
  // Client, Context, and Utility variables
  pami_client_t  _client;
  pami_context_t _context;
  size_t         _context_id;
  size_t         _num_contexts;
  size_t         _client_id;
  bool           _enabled;
  GIAllocator    _geom_allocator;
  FCA_init_spec  _fca_init_spec;
  FCA_t         *_fca_context;
}; // FCARegistration
}; // FCA
}; // CollRegistration
}; // PAMI
#endif
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
