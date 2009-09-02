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

          ///
          /// \brief Number of physical active nodes in the partition.
          ///
          /// This is not necessarily the same as the number of active tasks.
          ///
          /// \see size()
          ///
          inline size_t numActiveNodesGlobal ();

          ///
          /// \brief Number of physical active tasks in the partition.
          ///
          /// This is the same as size.
          ///
          /// \see size()
          ///
          inline size_t numActiveTasksGlobal ();

          ///
          /// \brief Number of physical active tasks in the local node.
          ///
          inline size_t numActiveTasksLocal ();

          ///
          /// \brief Determines if two global task ids are located on the same physical node.
          ///
          inline bool isPeer (size_t task1, size_t task2);
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

      template <class T_Mapping, class T_Memory>
      inline size_t Base<T_Mapping,T_Memory>::numActiveNodesGlobal ()
      {
        return static_cast<T_Mapping*>(this)->numActiveNodesGlobal_impl ();
      }

      template <class T_Mapping, class T_Memory>
      inline size_t Base<T_Mapping,T_Memory>::numActiveTasksGlobal ()
      {
        return static_cast<T_Mapping*>(this)->numActiveTasksGlobal_impl ();
      }

      template <class T_Mapping, class T_Memory>
      inline size_t Base<T_Mapping,T_Memory>::numActiveTasksLocal ()
      {
        return static_cast<T_Mapping*>(this)->numActiveTasksLocal_impl ();
      }

      template <class T_Mapping, class T_Memory>
      inline bool Base<T_Mapping,T_Memory>::isPeer (size_t task1, size_t task2)
      {
        return static_cast<T_Mapping*>(this)->isPeer_impl (task1, task2);
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
