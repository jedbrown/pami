/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/mapping/bgp/BgpMapping.h
/// \brief ???
///
#ifndef __components_mapping_bgp_bgpmapping_h__
#define __components_mapping_bgp_bgpmapping_h__

#include "xmi.h"

#include "../BasMapping.h"
#include "../TorusMapping.h"

namespace XMI
{
  namespace Mapping
  {
    class BgpMapping : public Base<BgpMapping>, public Torus<BgpMapping>
    {
      public:
        inline BgpMapping () :
            Base<BgpMapping>(),
            Torus<BgpMapping>()
        {
        };

        inline ~BgpMapping () {};

        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Base interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Return the BGP global task for this process
        /// \see XMI::Mapping::Base::task()
        ///
        inline size_t task_impl() const
        {
          return _task;
        }

        ///
        /// \brief Returns the number of global tasks
        /// \see XMI::Mapping::Base::size()
        ///
        inline size_t size_impl() const
        {
          return _size;
        }

        ///
        /// \brief Number of physical active nodes in the partition.
        /// \see XMI::Mapping::Base::numActiveNodesGlobal()
        ///
        inline size_t numActiveNodesGlobal_impl () const
        {
          return _nodes;
        }

        ///
        /// \brief Number of physical active tasks in the partition.
        /// \see XMI::Mapping::Base::numActiveRanksGlobal()
        ///
        inline size_t numActiveRanksGlobal_impl () const
        {
          return _size;
        }

        ///
        /// \brief Number of physical active tasks in the local node.
        /// \see XMI::Mapping::Base::numActiveRanksLocal()
        ///
        inline size_t numActiveRanksLocal_impl () const
        {
          return _peers;
        }

        ///
        /// \brief Determines if two global tasks are located on the same physical node.
        /// \see XMI::Mapping::Base::isPeer()
        ///
        inline bool isPeer_impl (size_t task1, size_t task2) const
        {
          unsigned xyzt1 = _mapcache[task1];
          unsigned xyzt2 = _mapcache[task2];
          return ((xyzt1>>8)==(xyzt2>>8));
        }
#if 0
        inline DCMF_Result network2task_impl (const DCMF_NetworkCoord_t  * addr,
                                              size_t                     * task,
                                              DCMF_Network               * type) const
        {
#error implement this
        }

        inline DCMF_Result task2network_impl (size_t                task,
                                              DCMF_NetworkCoord_t * addr,
                                              DCMF_Network          type) const
        {
#error implement this
        }
#endif
        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Torus interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Return the BGP torus x coordinate (dimension 0) for this task
        ///
        /// \see XMI::Mapping::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<0> () const
        {
          return _pers->xCoord();
        }

        ///
        /// \brief Return the BGP torus y coordinate (dimension 1) for this task
        ///
        /// \see XMI::Mapping::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<1> () const
        {
          return _pers->yCoord();
        }

        ///
        /// \brief Return the BGP torus z coordinate (dimension 2) for this task
        ///
        /// \see XMI::Mapping::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<2> () const
        {
          return _pers->zCoord();
        }

        ///
        /// \brief Return the BGP torus t coordinate (dimension 3) for this task
        ///
        /// \see XMI::Mapping::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<3> () const
        {
          return _pers->tCoord();
        }

        ///
        /// \brief Return the size of the BGP torus x dimension
        ///
        /// \see XMI::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<0> () const
        {
          return _pers->xSize();
        }

        ///
        /// \brief Return the size of the BGP torus y dimension
        ///
        /// \see XMI::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<1> () const
        {
          return _pers->ySize();
        }

        ///
        /// \brief Return the size of the BGP torus z dimension
        ///
        /// \see XMI::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<2> () const
        {
          return _pers->zSize();
        }

        ///
        /// \brief Return the size of the BGP torus t dimension
        /// \see XMI::Mapping::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<3> () const
        {
          return _pers->tSize();
        }

        ///
        /// \brief Get the number of BGP torus dimensions
        /// \see XMI::Mapping::Torus::torusDims()
        ///
        inline const size_t torusDims_impl() const
        {
          return 4;
        }

        ///
        /// \brief Get the BGP torus address for this task
        /// \see XMI::Mapping::Torus::torusAddr()
        ///
        template <>
        inline void torusAddr_impl<4> (size_t (&addr)[4]) const
        {
          addr[0] = _x;
          addr[1] = _y;
          addr[2] = _z;
          addr[3] = _t;
        }

        ///
        /// \brief Get the BGP torus address for a task
        /// \see XMI::Mapping::Torus::task2torus()
        ///
        /// \todo Error path
        ///
        template <>
        inline xmi_result_t task2torus_impl<4> (size_t task, size_t (&addr)[4]) const
        {
          unsigned xyzt = _mapcache[task];
          addr[0] = (xyzt & 0xFF000000) >> 24;
          addr[1] = (xyzt & 0xFF0000) >> 16;
          addr[2] = (xyzt & 0xFF00) >> 8;
          addr[3] = (xyzt & 0xFF);
          return XMI_SUCCESS;
        }

        ///
        /// \brief Get the global task for a BGP torus address
        /// \see XMI::Mapping::Torus::torus2task()
        ///
        /// \todo Error path
        ///
        template <>
        inline xmi_result_t torus2task_impl<4> (size_t (&addr)[4], size_t & task) const
        {
          if ((addr[0] >= _pers->xSize()) ||
              (addr[1] >= _pers->ySize()) ||
              (addr[2] >= _pers->zSize()) ||
              (addr[3] >= _pers->tSize()))
          {
            return XMI_INVAL;
          }

          size_t estimated_task =
            addr[3] * (xSize() * ySize() * zSize()) +
            addr[2] * (xSize() * ySize()) +
            addr[1] * (xSize()) +
            addr[0];

          unlikely_if (_taskcache [estimated_task] == (unsigned)-1)
          {
            return XMI_ERROR;
          }

          task = _taskcache [estimated_task];
          return XMI_SUCCESS;
        };
    }
  };
};
#endif /* __dcmf_mapping_bgp_h__ */
