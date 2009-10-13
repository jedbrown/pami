/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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
      template <class T_Mapping, class T_Memory>
      class Base
      {
        public:

          ///
          /// \brief Initialize the mapping object.
          ///
          inline xmi_result_t init (xmi_coord_t &ll, xmi_coord_t &ur,
				    size_t &min_rank, size_t &max_rank,
				    T_Memory &mm);

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
      };


      template <class T_Mapping, class T_Memory>
      inline xmi_result_t Base<T_Mapping,T_Memory>::init (xmi_coord_t &ll, xmi_coord_t &ur,
				    size_t &min_rank, size_t &max_rank, T_Memory &mm)
      {
        return static_cast<T_Mapping*>(this)->init_impl (ll, ur, min_rank, max_rank, mm);
      }

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
    };
  };
};
#endif // __components_mapping_basemapping_h__
