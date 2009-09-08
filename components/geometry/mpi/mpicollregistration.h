#ifndef   __xmi_mpicollregistration__h__
#define   __xmi_mpicollregistration__h__

#define XMI_COLLREGISTRATION_CLASS XMI::CollRegistration::MPI

#include "components/geometry/CollRegistration.h"

namespace XMI
{
  namespace CollRegistration
  {
    template <class T_Geometry, class T_Collfactory>
    class MPI : public CollRegistration<XMI::CollRegistration::MPI<T_Geometry, T_Collfactory>, T_Geometry, T_Collfactory>
    {
    public:
      inline MPI():
        CollRegistration<XMI::CollRegistration::MPI<T_Geometry, T_Collfactory>, T_Geometry, T_Collfactory>()
        {
        }

      inline T_Collfactory * analyze_impl(T_Geometry *geometry)
        {
          return XMI_UNIMPL;
        }
    }; // class Collregistration
  };  // namespace Collregistration
}; // namespace XMI
#endif
