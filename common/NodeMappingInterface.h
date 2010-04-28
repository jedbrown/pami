/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/NodeMappingInterface.h
/// \brief ???
///
#ifndef __common_NodeMappingInterface_h__
#define __common_NodeMappingInterface_h__

#include <pami.h>

namespace PAMI
{
  namespace Interface
  {
    namespace Mapping
    {
      ///
      /// \brief Task node address structure
      ///
      /// A task on a node is identified with two coordinates. The first
      /// coordinate globally identifies the physical node, and the second
      /// coordinate identifies the task on the node.
      ///
      typedef struct nodeaddr
      {
        size_t global; ///< Global node "rank"
        size_t local;  ///< Local node "rank"
      } nodeaddr_t;

      ///
      /// \param T_Mapping Node mapping template class
      ///
      template <class T_Mapping, unsigned T_Dimensions>
      class Node
      {
        public:
          ///
          /// \brief Get the number of possible tasks on any node
          ///
          inline pami_result_t nodeTasks (size_t global, size_t & tasks);

          ///
          /// \brief Get the number of peer tasks on the local node
          ///
          inline pami_result_t nodePeers (size_t & peers);

          ///
          /// \brief Determines if two tasks are located on the same node
          ///
          inline bool isPeer (size_t task1, size_t task2);

          ///
          /// \brief Node address for the local task
          ///
          /// \param[out] address Node address
          ///
          inline void nodeAddr (nodeaddr_t & address);

          ///
          /// \brief Node address for a specific task
          ///
          /// The global task identifier monotonically increases from zero to
          /// PAMI::Mapping::Interface::Base.size() - 1.
          ///
          /// \param[in]  task    Global task identifier
          /// \param[out] address Node address
          ///
          inline pami_result_t task2node (size_t task, nodeaddr_t & address);

          ///
          /// \brief Global task identifier associated with a specific node address
          ///
          /// The global task identifier monotonically increases from zero to
          /// PAMI::Mapping::Interface::Base.size() - 1.
          ///
          /// \param[in]  address Node address
          /// \param[out] task    Global task identifier
          ///
          inline pami_result_t node2task (nodeaddr_t & address, size_t & task);

          ///
          /// \brief Peer identifier associated with a specific node address
          ///
          /// The local peer identifier monotonically increases from zero to
          /// PAMI::Mapping::Interface::Node.nodePeers() - 1.
          ///
          /// \param[in]  address Node address
          /// \param[out] peer    peer identifier
          ///
          inline pami_result_t node2peer (nodeaddr_t & address, size_t & peer);
      };	// class Node

      template <class T_Mapping, unsigned T_Dimensions>
      inline pami_result_t Node<T_Mapping,T_Dimensions>::nodeTasks (size_t global, size_t & tasks)
      {
        return static_cast<T_Mapping*>(this)->nodeTasks_impl (global, tasks);
      }

      template <class T_Mapping, unsigned T_Dimensions>
      inline pami_result_t Node<T_Mapping,T_Dimensions>::nodePeers (size_t & peers)
      {
        return static_cast<T_Mapping*>(this)->nodePeers_impl (peers);
      }

      template <class T_Mapping, unsigned T_Dimensions>
      inline bool Node<T_Mapping,T_Dimensions>::isPeer (size_t task1, size_t task2)
      {
        return static_cast<T_Mapping*>(this)->isPeer_impl (task1, task2);
      }

      template <class T_Mapping, unsigned T_Dimensions>
      inline void Node<T_Mapping,T_Dimensions>::nodeAddr (nodeaddr_t & address)
      {
        return static_cast<T_Mapping*>(this)->nodeAddr_impl (address);
      }

      template <class T_Mapping, unsigned T_Dimensions>
      inline pami_result_t Node<T_Mapping,T_Dimensions>::task2node (size_t task, nodeaddr_t & address)
      {
        return static_cast<T_Mapping*>(this)->task2node_impl (task, address);
      }

      template <class T_Mapping, unsigned T_Dimensions>
      inline pami_result_t Node<T_Mapping,T_Dimensions>::node2task (nodeaddr_t & address, size_t & task)
      {
        return static_cast<T_Mapping*>(this)->node2task_impl (address, task);
      }

      template <class T_Mapping, unsigned T_Dimensions>
      inline pami_result_t Node<T_Mapping,T_Dimensions>::node2peer (nodeaddr_t & address, size_t & peer)
      {
        return static_cast<T_Mapping*>(this)->node2peer_impl (address, peer);
      }
    };	// namespace Mapping
  };	// namespace Interface
};	// namespace PAMI
#endif // __components_mapping_nodemapping_h__
