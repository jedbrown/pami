/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/mapping/TorusMapping.h
/// \brief ???
///
#ifndef __components_mapping_torusmapping_h__
#define __components_mapping_torusmapping_h__

#include "sys/xmi.h"

namespace XMI
{
  namespace Mapping
  {
    namespace Interface
    {
      ///
      /// \param T_Mapping Torus mapping template class
      ///
      template <class T_Mapping, unsigned T_Dimensions>
      class Torus
      {


        public:
#if 0
          ///
          /// \brief Get the torus coordinate of a particular dimension for the local task
          ///
          /// \note The C++ template parameter \c T_Dimension is used to specify
          ///       torus coordinate to retrieve. The use of C++ templates for this
          ///       method allows the compiler to generate code without an if-branch
          ///       to determine which coordinate to retrieve.
          ///
          /// \code
          ///   size_t x = mapping.torusCoord<0>();
          ///   size_t y = mapping.torusCoord<1>();
          ///   size_t z = mapping.torusCoord<2>();
          ///   size_t t = mapping.torusCoord<3>();
          /// \endcode
          ///
          /// \return Torus coordinate for this task
          ///
          template <int T_Dimension>
          inline size_t torusCoord () const
          {
            abort();
            return;
          };

          ///
          /// \brief Get the size of a torus dimension
          ///
          /// \note The C++ template parameter \c T_Dimension is used to specify
          ///       torus dimension to query. The use of C++ templates for this
          ///       method allows the compiler to generate code without an if-branch.
          ///
          /// \code
          ///   size_t x = mapping.torusSize<0>();
          ///   size_t y = mapping.torusSize<1>();
          ///   size_t z = mapping.torusSize<2>();
          ///   size_t t = mapping.torusSize<3>();
          /// \endcode
          ///
          /// \return Torus dimension size
          ///
          template <int T_Dimension>
          inline size_t torusSize () const;
#endif
          ///
          /// \brief Get the number of torus dimensions
          ///
          /// \return Torus dimensions
          ///
          inline size_t torusDims() const
          {
            return T_Dimensions;
          };

          ///
          /// \brief Get the torus address for the local task
          ///
          /// \note The C++ template parameter \c T_Dimension is used to specify
          ///       the number torus dimensions. The use of C++ templates for this
          ///       method allows the compiler to generate code specific to the
          ///       number of torus dimensions for a specific platform.
          ///       Additionally, the template parameter will enforce a range check
          ///       on the output array.
          ///
          /// \todo Determine if the range check on the output array is a
          ///       hinderance or is helpful.
          ///
          /// \code
          ///   // Example torus mapping implentation:
          ///   template <>
          ///   inline void torusAddr<4> (size_t (&addr)[4]) const
          ///   {
          ///     ...
          ///   };
          ///
          ///   size_t addr[4];
          ///   mapping.torusAddr<4>(addr);
          ///   size_t x = addr[0];
          ///   size_t y = addr[1];
          ///   size_t z = addr[2];
          ///   size_t t = addr[3];
          /// \endcode
          ///
          /// \param[out] addr Array of torus coordinates
          ///
//          inline void torusAddr (size_t (&addr)[T_Dimensions]) const;
          inline void torusAddr (size_t (&addr)[T_Dimensions]) const;

          ///
          /// \brief Get the torus address for a specific task
          ///
          /// \note The C++ template parameter \c T_Dimension is used to specify
          ///       the number torus dimensions. The use of C++ templates for this
          ///       method allows the compiler to generate code specific to the
          ///       number of torus dimensions for a specific platform.
          ///       Additionally, the template parameter will enforce a range check
          ///       on the output array.
          ///
          /// \todo Determine if the range check on the output array is a
          ///       hinderance or is helpful.
          ///
          /// \code
          ///   size_t addr[4];
          ///   mapping.task2torus<4>(0, addr);
          ///   size_t x = addr[0];
          ///   size_t y = addr[1];
          ///   size_t z = addr[2];
          ///   size_t t = addr[3];
          /// \endcode
          ///
          /// \param[in]  task Global task identifier
          /// \param[out] addr Array of torus coordinates
          ///
          /// \retval XMI_SUCCESS
          /// \retval XMI_INVAL   Invalid task used as an input
          ///
          inline xmi_result_t task2torus (size_t task, size_t (&addr)[T_Dimensions]) const;

          ///
          /// \brief Get the global task for a specific torus address
          ///
          /// \note The C++ template parameter \c T_Dimension is used to specify
          ///       the number torus dimensions. The use of C++ templates for this
          ///       method allows the compiler to generate code specific to the
          ///       number of torus dimensions for a specific platform.
          ///       Additionally, the template parameter will enforce a range check
          ///       on the output array.
          ///
          /// \todo Determine if the range check on the output array is a
          ///       hinderance or is helpful.
          ///
          /// \code
          ///   size_t task;
          ///   size_t addr[4];
          ///   addr[0] = 0;
          ///   addr[1] = 1;
          ///   addr[2] = 2;
          ///   addr[3] = 3;
          ///   if (mapping.torus2task<4>(task, addr) == DCMF_SUCCESS)
          ///   { ... }
          ///
          ///   if (mapping.torus2task<4>(task, {3,2,1,0]) == DCMF_SUCCESS)
          ///   { ... }
          /// \endcode
          ///
          /// \param[in]  addr Array of torus coordinates
          /// \param[out] task Global task identifier
          ///
          /// \retval DCMF_SUCCESS
          /// \retval DCMF_INVAL   Invalid torus address used as an input
          ///
//          template <int T_Dimension>
          inline xmi_result_t torus2task (size_t (&addr)[T_Dimensions], size_t & task) const;
#if 0
        //protected:
          ///
          /// \brief Default torus coordintate template implementation.
          ///
          /// If a torus mapping implementation does not define a method for a
          /// specific torus coordinate dimension then the base torus mapping
          /// template implementation will be invoked and will abort.
          ///
          /// \see XMI::Mapping::Torus::torusCoord()
          ///
          template <int T_Dimension>
          inline size_t torusCoord_impl<T_Dimension> () const;

          ///
          /// \brief Default torus dimension size template implementation.
          ///
          /// If a torus mapping implementation does not define a method for a
          /// specific dimension then the base torus mapping
          /// template implementation will be invoked and will abort.
          ///
          /// \see XMI::Mapping::Torus::torusSize()
          ///
          template <int T_Dimension>
          inline size_t torusSize_impl<T_Dimension> () const;

          ///
          /// \brief Default torus address template implementation.
          ///
          /// If a torus mapping implementation does not define a method for a
          /// specific dimension then the base torus mapping
          /// template implementation will be invoked and will abort.
          ///
          /// \see XMI::Mapping::Torus::torusAddr()
          ///
          template <int T_Dimension>
          inline void torusAddr_impl (size_t (&addr)[T_Dimension]) const;

          ///
          /// \brief Default task to torus address template implementation.
          ///
          /// If a torus mapping implementation does not define a method for a
          /// specific dimension then the base torus mapping
          /// template implementation will be invoked and will abort.
          ///
          /// \see XMI::Mapping::Torus::task2torus()
          ///
          template <int T_Dimension>
          inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[T_Dimension]) const;

          ///
          /// \brief Default torus to task address template implementation.
          ///
          /// If a torus mapping implementation does not define a method for a
          /// specific dimension then the base torus mapping
          /// template implementation will be invoked and will abort.
          ///
          /// \see XMI::Mapping::Torus::torus2task()
          ///
          template <int T_Dimension>
          inline xmi_result_t torus2task_impl (size_t (&addr)[T_Dimension], size_t & task) const;
#endif
      };
#if 0
      template <class T_Mapping, unsigned T_Dimensions>
      template <int T_Dimension>
      inline size_t Torus<T_Mapping,T_Dimensions>::torusCoord<T_Dimension> () const
      {
        abort();
        return 0;
        //return static_cast<T_Mapping*>(this)->torusCoord_impl[T_Dimension];
      }


      template <class T_Mapping, unsigned T_Dimensions>
      template <int T_Dimension>
      inline size_t Torus<T_Mapping>::torusSize<T_Dimension> () const
      {
        return static_cast<T_Mapping*>(this)->torusSize_impl<T_Dimension> ();
      }
#endif
#if 0
      template <class T_Mapping, unsigned T_Dimensions>
      inline size_t Torus<T_Mapping,T_Dimensions>::torusDims () const
      {
        return T_Dimensions;
      }
#endif
      template <class T_Mapping, unsigned T_Dimensions>
      //template <int T_Dimension>
      //inline size_t Torus<T_Mapping,T_Dimensions>::torusAddr (size_t (&addr)[T_Dimensions]) const
      inline void Torus<T_Mapping,T_Dimensions>::torusAddr (size_t (&addr)[T_Dimensions]) const
      {
        return static_cast<T_Mapping*>(this)->torusAddr_impl (addr);
      }

      template <class T_Mapping, unsigned T_Dimensions>
      //template <int T_Dimension>
      inline xmi_result_t Torus<T_Mapping,T_Dimensions>::task2torus (size_t task, size_t (&addr)[T_Dimensions]) const
      {
        return static_cast<T_Mapping*>(this)->task2torus_impl (task, addr);
      }

      template <class T_Mapping, unsigned T_Dimensions>
      //template <int T_Dimension>
      inline xmi_result_t Torus<T_Mapping,T_Dimensions>::torus2task (size_t (&addr)[T_Dimensions], size_t & task) const
      {
        return static_cast<T_Mapping*>(this)->torus2task_impl (addr, task);
      }
#if 0
      template <class T_Mapping>
      template <int T_Dimension>
      inline size_t Torus<T_Mapping>::torusCoord_impl<T_Dimension> () const
      {
        abort();
        return 0;
      }

      template <class T_Mapping>
      template <int T_Dimension>
      inline size_t Torus<T_Mapping>::torusSize_impl<T_Dimension> () const
      {
        abort();
        return 0;
      }

      template <class T_Mapping>
      template <int T_Dimension>
      inline void Torus<T_Mapping>::torusAddr_impl (size_t (&addr)[T_Dimension]) const;
      {
        abort();
        return 0;
      }

      template <class T_Mapping>
      template <int T_Dimension>
      inline xmi_result_t Torus<T_Mapping>::task2torus_impl (size_t task, size_t (&addr)[T_Dimension]) const;
      {
        abort();
        return 0;
      }

      template <class T_Mapping>
      template <int T_Dimension>
      inline xmi_result_t Torus<T_Mapping>::torus2task_impl (size_t (&addr)[T_Dimension], size_t & task) const;
      {
        abort();
        return 0;
      }
#endif
    };
  };
};
#endif // __components_mapping_torusmapping_h__
