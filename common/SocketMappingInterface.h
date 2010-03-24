/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/SocketMappingInterface.h
/// \brief ???
///
#ifndef __common_SocketMappingInterface_h__
#define __common_SocketMappingInterface_h__

#include "sys/pami.h"

namespace PAMI
{
  namespace Interface
  {
    namespace Mapping
    {
      ///
      /// \param T_Mapping Socket mapping template class
      ///
      template <class T_Mapping>
      class Socket
      {
        public:

          ///
          /// \brief Get the socket address for the local task
          ///
          /// \param[out] recv_fd Socket recv file descriptor
          /// \param[out] send_fd Socket send file descriptor
          ///
          inline void socketAddr (size_t & recv_fd, size_t & send_fd) const;

          ///
          /// \brief Get the socket address for a specific task
          ///
          /// \param[in]  task    Global task identifier
          /// \param[out] recv_fd Socket recv file descriptor
          /// \param[out] send_fd Socket send file descriptor
          ///
          inline pami_result_t task2socket (size_t task, size_t & recv_fd, size_t & send_fd) const;

          ///
          /// \brief Get the task associated with a specific socket address
          ///
          /// \param[in]  recv_fd Socket recv file descriptor
          /// \param[in]  send_fd Socket send file descriptor
          /// \param[out] task    Global task identifier
          ///
          inline pami_result_t socket2task (size_t recv_fd, size_t send_fd, size_t & task) const;

      }; // class Socket

      template <class T_Mapping>
      inline void Socket<T_Mapping>::socketAddr (size_t & recv_fd, size_t & send_fd) const
      {
        return static_cast<T_Mapping*>(this)->socketAddr_impl (recv_fd, send_fd);
      }

      template <class T_Mapping>
      inline pami_result_t Socket<T_Mapping>::task2socket (size_t task, size_t & recv_fd, size_t & send_fd) const
      {
        return static_cast<T_Mapping*>(this)->task2socket_impl (task, recv_fd, send_fd);
      }

      template <class T_Mapping>
      inline pami_result_t Socket<T_Mapping>::socket2task (size_t recv_fd, size_t send_fd, size_t & task) const
      {
        return static_cast<T_Mapping*>(this)->socket2task_impl (recv_fd, send_fd, task);
      }
    };	// namespace Mapping
  };	// namespace Interface
};	// namespace PAMI
#endif // __components_mapping_socketmapping_h__
