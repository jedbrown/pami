/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/BaseMappingInterface.h
/// \brief ???
///
#ifndef __common_BaseMappingInterface_h__
#define __common_BaseMappingInterface_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace Interface
  {
    namespace Mapping
    {
      ///
      /// \param T_Mapping Base mapping template class
      ///
      template <class T_Mapping, class T_Memory>
      class Base
      {
        public:

          ///
          /// \brief Returns the global task id for this process
          ///
          inline size_t task ();

          ///
          /// \brief Returns the number of global tasks
          ///
          inline size_t size ();


          inline xmi_result_t network2task (const xmi_coord_t  * addr,
                                           size_t                     * rank,
                                           xmi_network               * type);

          inline xmi_result_t task2network (size_t                rank,
                                           xmi_coord_t * addr,
                                           xmi_network          type);
	  inline size_t globalDims();
      };	// class Base

      template <class T_Mapping, class T_Memory>
      inline size_t Base<T_Mapping,T_Memory>::task ()
      {
        return static_cast<T_Mapping*>(this)->task_impl ();
      }

      template <class T_Mapping, class T_Memory>
      inline size_t Base<T_Mapping,T_Memory>::size ()
      {
        return static_cast<T_Mapping*>(this)->size_impl ();
      }
      template <class T_Mapping, class T_Memory>
      inline xmi_result_t Base<T_Mapping,T_Memory>::network2task (const xmi_coord_t  * addr,
                                                        size_t                     * rank,
                                                        xmi_network               * type)
      {
        return static_cast<T_Mapping*>(this)->network2task_impl (addr, rank, type);
      }

      template <class T_Mapping, class T_Memory>
      inline xmi_result_t Base<T_Mapping,T_Memory>::task2network (size_t                rank,
                                                        xmi_coord_t * addr,
                                                        xmi_network          type)
      {
        return static_cast<T_Mapping*>(this)->task2network_impl (rank, addr, type);
      }

      template <class T_Mapping, class T_Memory>
      inline size_t Base<T_Mapping,T_Memory>::globalDims ()
      {
        return static_cast<T_Mapping*>(this)->globalDims_impl ();
      }
    };	// namespace Mapping
  };	// namespace Interface
};	// namespace XMI
#endif // __components_mapping_basemapping_h__
