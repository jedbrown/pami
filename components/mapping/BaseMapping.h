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
      template <class T_Mapping, class T_Memory>
      class Base
      {
        public:

          ///
          /// \brief Initialize the mapping object.
          ///
          inline xmi_result_t init (T_Memory & mm);

          ///
          /// \brief Returns the global task id for this process
          ///
          inline size_t task ();

          ///
          /// \brief Returns the number of global tasks
          ///
          inline size_t size ();

 
#if 0
          inline DCMF_Result network2rank (const DCMF_NetworkCoord_t  * addr,
                                           size_t                     * rank,
                                           DCMF_Network               * type) const;

          inline DCMF_Result rank2network (size_t                rank,
                                           DCMF_NetworkCoord_t * addr,
                                           DCMF_Network          type) const;
#endif
      };


      template <class T_Mapping, class T_Memory>
      inline xmi_result_t Base<T_Mapping,T_Memory>::init (T_Memory & mm)
      {
        return static_cast<T_Mapping*>(this)->init_impl (mm);
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
#if 0
      template <class T_Mapping, class T_Memory>
      inline DCMF_Result Base<T_Mapping,T_Memory>::network2rank (const DCMF_NetworkCoord_t  * addr,
                                                        size_t                     * rank,
                                                        DCMF_Network               * type) const
      {
        return static_cast<T_Mapping*>(this)->network2rank_impl (addr, rank, type);
      }

      template <class T_Mapping, class T_Memory>
      inline DCMF_Result Base<T_Mapping,T_Memory>::rank2network (size_t                rank,
                                                        DCMF_NetworkCoord_t * addr,
                                                        DCMF_Network          type) const
      {
        return static_cast<T_Mapping*>(this)->rank2network_impl (rank, addr, type);
      }
#endif
    };
  };
};
#endif // __components_mapping_basemapping_h__
