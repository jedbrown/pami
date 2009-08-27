/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/mapping/NodeMapping.h
/// \brief ???
///
#ifndef __components_mapping_nodemapping_h__
#define __components_mapping_nodemapping_h__

#include "xmi.h"

namespace XMI
{
  namespace Mapping
  {
    namespace Interface
    {
      ///
      /// \param T_Mapping Node mapping template class
      ///
      template <class T_Mapping>
      class Node
      {
        public:
          ///
          /// \brief Get the number of possible tasks on a node
          ///
          /// \return Dimension size
          ///
          inline xmi_result_t nodeSize (size_t global, size_t & size) const;

          ///
          /// \brief Get the node address for the local task
          ///
          /// \param[out] global Node global identifier
          /// \param[out] local  Node local identifier
          ///
          inline void nodeAddr (size_t & global, size_t & local) const;

          ///
          /// \brief Get the node address for a specific task
          ///
          /// \param[in]  task   Global task identifier
          /// \param[out] global Node global identifier
          /// \param[out] local  Node local identifier
          ///
          inline xmi_result_t task2node (size_t task, size_t & global, size_t & local) const;

          ///
          /// \brief Get the task associated with a specific node address
          ///
          /// \param[in]  global Node global identifier
          /// \param[in]  local  Node local identifier
          /// \param[out] task   Global task identifier
          ///
          inline xmi_result_t node2task (size_t global, size_t local, size_t & task) const;
      }

      template <class T_Mapping>
      inline xmi_result_t Node<T_Mapping>::nodeSize (size_t global, size_t & size) const
      {
        return static_cast<T_Mapping*>(this)->nodeSize_impl (global, size);
      }

      template <class T_Mapping>
      inline void Node<T_Mapping>::nodeAddr (size_t & global, size_t & local) const
      {
        return static_cast<T_Mapping*>(this)->nodeAddr_impl (global, local);
      }

      template <class T_Mapping>
      inline xmi_result_t Node<T_Mapping>::task2node (size_t task, size_t & global, size_t & local) const
      {
        return static_cast<T_Mapping*>(this)->task2node_impl (task, global, local);
      }

      template <class T_Mapping>
      inline xmi_result_t Node<T_Mapping>::node2task (size_t global, size_t local, size_t & task) const
      {
        return static_cast<T_Mapping*>(this)->node2task_impl (global, local, task);
      }
    };
  };
};
#endif // __components_mapping_nodemapping_h__
