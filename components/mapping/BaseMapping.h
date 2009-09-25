/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/mapping/BaseMapping.h
/// \brief ???
///
#ifndef __components_mapping_basemapping_h__
#define __components_mapping_basemapping_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace Mapping
  {
    namespace Interface
    {
      ///
      /// \param T_Mapping Base mapping template class
      ///
      template <class T_Mapping>
      class Base
      {
        public:

          ///
          /// \brief Initialize the mapping object.
          ///
          inline xmi_result_t init ();

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
      };


      template <class T_Mapping>
      inline xmi_result_t Base<T_Mapping>::init ()
      {
        return static_cast<T_Mapping*>(this)->init_impl ();
      }

      template <class T_Mapping>
      inline size_t Base<T_Mapping>::task ()
      {
        return static_cast<T_Mapping*>(this)->task_impl ();
      }

      template <class T_Mapping>
      inline size_t Base<T_Mapping>::size ()
      {
        return static_cast<T_Mapping*>(this)->size_impl ();
      }
      template <class T_Mapping>
      inline xmi_result_t Base<T_Mapping>::network2task (const xmi_coord_t  * addr,
                                                        size_t                     * rank,
                                                        xmi_network               * type)
      {
        return static_cast<T_Mapping*>(this)->network2task_impl (addr, rank, type);
      }

      template <class T_Mapping>
      inline xmi_result_t Base<T_Mapping>::task2network (size_t                rank,
                                                        xmi_coord_t * addr,
                                                        xmi_network          type)
      {
        return static_cast<T_Mapping*>(this)->task2network_impl (rank, addr, type);
      }
    };
  };
};
#endif // __components_mapping_basemapping_h__
