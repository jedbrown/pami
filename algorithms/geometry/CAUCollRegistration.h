/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/CAURegistration.h
 * \brief ???
 */

#ifndef __algorithms_geometry_CAURegistration_h__
#define __algorithms_geometry_CAURegistration_h__

#include <map>
#include <vector>
#include "common/NativeInterface.h"


// CAU Template implementations for CAU
namespace CAU
{
  namespace Adaptor
  {


  }//Adaptor
}//CAU

// Collective Registration for CAU protocols for p2p
namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;

  namespace CollRegistration
  {
    namespace CAU
    {
      template <class T_Geometry,
                class T_Local_Device,
                class T_Global_Device,
                class T_LocalNI_AM,
                class T_GlobalNI_AM>
      class CAURegistration :
        public CollRegistration < PAMI::CollRegistration::CAU::CAURegistration < T_Geometry,
                                                                                 T_Local_Device,
                                                                                 T_Global_Device,
                                                                                 T_LocalNI_AM,
                                                                                 T_GlobalNI_AM>,
                                  T_Geometry >
      {
        public:
          inline CAURegistration(pami_client_t       client,
                                  pami_context_t      context,
                                  size_t              context_id,
                                  size_t              client_id,
                                  T_Local_Device     &ldev,
                                  T_Global_Device    &gdev,
                                  size_t              global_size,
                                  size_t              local_size):
            CollRegistration < PAMI::CollRegistration::CAU::CAURegistration < T_Geometry,
                                                                              T_Local_Device,
                                                                              T_Global_Device,
                                                                              T_LocalNI_AM,
                                                                              T_GlobalNI_AM>,
                               T_Geometry > (),
            _client(client),
            _context(context),
            _context_id(context_id),
            _client_id(client_id),
            _local_dev(ldev),
            _global_dev(gdev)
          {
          }

          inline pami_result_t analyze_impl(size_t context_id, T_Geometry *geometry)
          {
            return PAMI_SUCCESS;
          }

          static pami_geometry_t mapidtogeometry (int comm)
          {
            pami_geometry_t g = geometry_map[comm];
            TRACE_INIT((stderr, "<%p>CAURegistration::mapidtogeometry()\n", g));
            return g;
          }
      private:
          /// \todo use allocator instead of _storage?  Since they aren't always constructed, we waste memory now.
          pami_client_t                                                _client;
          pami_context_t                                               _context;
          size_t                                                       _context_id;
          size_t                                                       _client_id;

          // Protocol device(s) and allocator
          T_Local_Device                                              &_local_dev;
          T_Global_Device                                             &_global_dev;
      };
    }; // CAU
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
