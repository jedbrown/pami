/**
 * \file api/extension/c/torus_network/TorusExtension.h
 * \brief PAMI extension "torus network" interface
 */
#ifndef __api_extension_c_torus_network_TorusExtension_h__
#define __api_extension_c_torus_network_TorusExtension_h__

#include <pami.h>

namespace PAMI
{
  class TorusExtension
  {

    public:

      ///
      /// \brief Information structure for static torus data
      typedef struct torus_info
      {
        size_t   dims;  ///< Number of torus dimensions
        size_t * coord; ///< Torus coordinates of the calling process
        size_t * size;  ///< Length of each torus dimension
        size_t * torus; ///< Mesh (unset) or torus (set)
      } torus_info_t;

      TorusExtension ();

      static const PAMI::TorusExtension::torus_info_t * information ();

      ///
      /// \brief
      ///
      /// \see PAMI::Interface::Mapping::Torus::task2torus
      ///
      static pami_result_t task2torus (pami_task_t task, size_t addr[]);

      ///
      /// \brief
      ///
      /// \see PAMI::Interface::Mapping::Torus::torus2task
      ///
      static pami_result_t torus2task (size_t addr[], pami_task_t * task);

    protected:

      static torus_info_t info;
      static size_t coords[1024];
  };
};

#endif // __api_extension_c_torus_network_TorusExtension_h__
