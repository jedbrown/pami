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

#include "sys/xmi.h"

#include "../BaseMapping.h"
#include "../TorusMapping.h"

#include "components/sysdep/bgp/BgpPersonality.h"

extern XMI::SysDep::BgpPersonality __global_personality;

namespace XMI
{
  namespace Mapping
  {
    template <class T_Memory>
    class BgpMapping : public Interface::Base<BgpMapping<T_Memory>,T_Memory>,
                       public Interface::Torus<BgpMapping<T_Memory>,4>
    {
      public:
        inline BgpMapping () :
            Interface::Base<BgpMapping<T_Memory>,T_Memory>(),
            Interface::Torus<BgpMapping<T_Memory>,4>(),
            _x (__global_personality.xCoord()),
            _y (__global_personality.yCoord()),
            _z (__global_personality.zCoord()),
            _t (__global_personality.tCoord())
        {
          
        };

        inline ~BgpMapping () {};

      protected:
        size_t _task;
        size_t _size;
        size_t _nodes;
        size_t _peers;
        size_t _x;
        size_t _y;
        size_t _z;
        size_t _t;
        
        size_t _numActiveRanksLocal;
        size_t _numActiveRanksGlobal;
        size_t _numActiveNodesGlobal;
        size_t _fullSize;
        
        size_t * _mapcache;
        size_t * _rankcache;
        
      public:

        /////////////////////////////////////////////////////////////////////////
        //
        // XMI::Mapping::Interface::Base interface implementation
        //
        /////////////////////////////////////////////////////////////////////////

        ///
        /// \brief Initialize the mapping
        /// \see XMI::Mapping::Interface::Base::init()
        ///
        inline xmi_result_t init_impl(T_Memory & mm);

        ///
        /// \brief Return the BGP global task for this process
        /// \see XMI::Mapping::Interface::Base::task()
        ///
        inline size_t task_impl() const
        {
          return _task;
        }

        ///
        /// \brief Returns the number of global tasks
        /// \see XMI::Mapping::Interface::Base::size()
        ///
        inline size_t size_impl() const
        {
          return _size;
        }

        ///
        /// \brief Number of physical active nodes in the partition.
        /// \see XMI::Mapping::Interface::Base::numActiveNodesGlobal()
        ///
        inline size_t numActiveNodesGlobal_impl () const
        {
          return _nodes;
        }

        ///
        /// \brief Number of physical active tasks in the partition.
        /// \see XMI::Mapping::Interface::Base::numActiveRanksGlobal()
        ///
        inline size_t numActiveRanksGlobal_impl () const
        {
          return _size;
        }

        ///
        /// \brief Number of physical active tasks in the local node.
        /// \see XMI::Mapping::Interface::Base::numActiveRanksLocal()
        ///
        inline size_t numActiveRanksLocal_impl () const
        {
          return _peers;
        }

        ///
        /// \brief Determines if two global tasks are located on the same physical node.
        /// \see XMI::Mapping::Interface::Base::isPeer()
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
        // XMI::Mapping::Interface::Torus interface implementation
        //
        /////////////////////////////////////////////////////////////////////////
        
        ///
        /// \brief Return the BGP torus x coordinate (dimension 0) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        inline size_t x () const
        {
          return __global_personality.xCoord();
        }
        inline size_t y () const
        {
          return __global_personality.yCoord();
        }
        inline size_t z () const
        {
          return __global_personality.zCoord();
        }
        inline size_t t () const
        {
          return __global_personality.tCoord();
        }
        
        inline size_t xSize () const
        {
          return __global_personality.xSize();
        }
        
        inline size_t ySize () const
        {
          return __global_personality.ySize();
        }
        
        inline size_t zSize () const
        {
          return __global_personality.zSize();
        }
        
        inline size_t tSize () const
        {
          return __global_personality.tSize();
        }
        
        
#if 0

        ///
        /// \brief Return the BGP torus y coordinate (dimension 1) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<1> () const
        {
          return __global_personality.yCoord();
        }

        ///
        /// \brief Return the BGP torus z coordinate (dimension 2) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<2> () const
        {
          return __global_personality.zCoord();
        }

        ///
        /// \brief Return the BGP torus t coordinate (dimension 3) for this task
        ///
        /// \see XMI::Mapping::Interface::Torus::torusCoord()
        ///
        template <>
        inline size_t torusCoord_impl<3> () const
        {
          return __global_personality.tCoord();
        }

        ///
        /// \brief Return the size of the BGP torus x dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<0> () const
        {
          return __global_personality.xSize();
        }

        ///
        /// \brief Return the size of the BGP torus y dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<1> () const
        {
          return __global_personality.ySize();
        }

        ///
        /// \brief Return the size of the BGP torus z dimension
        ///
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<2> () const
        {
          return __global_personality.zSize();
        }

        ///
        /// \brief Return the size of the BGP torus t dimension
        /// \see XMI::Mapping::Interface::Torus::torusSize()
        ///
        template <>
        inline size_t torusSize_impl<3> () const
        {
          return __global_personality.tSize();
        }
#endif
#if 0
        ///
        /// \brief Get the number of BGP torus dimensions
        /// \see XMI::Mapping::Interface::Torus::torusDims()
        ///
        inline const size_t torusDims_impl() const
        {
          return 4;
        }
#endif
        ///
        /// \brief Get the BGP torus address for this task
        /// \see XMI::Mapping::Interface::Torus::torusAddr()
        ///
        //template <>
        inline void torusAddr_impl (size_t (&addr)[4]) const
        //inline void torusAddr_impl (Address & addr) const
        {
          addr[0] = _x;
          addr[1] = _y;
          addr[2] = _z;
          addr[3] = _t;
        }

        ///
        /// \brief Get the BGP torus address for a task
        /// \see XMI::Mapping::Interface::Torus::task2torus()
        ///
        /// \todo Error path
        ///
        //template <>
        inline xmi_result_t task2torus_impl (size_t task, size_t (&addr)[4]) const
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
        /// \see XMI::Mapping::Interface::Torus::torus2task()
        ///
        /// \todo Error path
        ///
        //template <>
        inline xmi_result_t torus2task_impl (size_t (&addr)[4], size_t & task) const
        {
          size_t xSize = __global_personality.xSize();
          size_t ySize = __global_personality.ySize();
          size_t zSize = __global_personality.zSize();
          size_t tSize = __global_personality.tSize();
        
          if ((addr[0] >= xSize) ||
              (addr[1] >= ySize) ||
              (addr[2] >= zSize) ||
              (addr[3] >= tSize))
          {
            return XMI_INVAL;
          }

          size_t estimated_task =
            addr[3] * (xSize * ySize * zSize) +
            addr[2] * (xSize * ySize) +
            addr[1] * (xSize) +
            addr[0];

          // convert to 'unlikely_if'
          if (_rankcache [estimated_task] == (unsigned)-1)
          {
            return XMI_ERROR;
          }

          task = _rankcache [estimated_task];
          return XMI_SUCCESS;
        };
    
    };
  };
};
#endif // __components_mapping_bgp_bgpmapping_h__

