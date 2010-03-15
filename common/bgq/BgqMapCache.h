/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_bgq_BgqMapCache_h__
#define __common_bgq_BgqMapCache_h__
///
/// \file common/bgq/BgqMapCache.h
/// \brief Blue Gene/Q Map- and Rank-Cache Object
///

#include <hwi/include/bqc/A2_inlines.h>

#include "common/bgq/BgqPersonality.h"

/// \brief Creates valid index into _rankcache[].
///
/// It is imperative that all who create an index into _rankcache[]
/// do it in the identical way. Thus all should use this macro.
///
#define ESTIMATED_TASK(a,b,c,d,e,p,t,aSize,bSize,cSize,dSize,eSize,pSize,tSize) \
(t+tSize*(p+pSize*(a+aSize*(b+bSize*(c+cSize*(d+dSize*(e)))))))

//	((((((p * tSize + t) * eSize + e) * dSize + d) * cSize + c)* bSize + b) * aSize + a)

namespace XMI
{
  ///
  /// \brief Blue Gene/Q coordinate structure
  ///
  /// This structure takes 32-bits on any 32/64 bit system. The a, b, c,
  /// and d fields are the same size and in the same location as the MU
  /// descriptor structure. The thread/core fields are sized for 16 cores
  /// with 4 hardware threads each, though the reserved bit can be stolen
  /// for the 17th core if it needs a rank. The e dimension is sized to the
  /// current node-layout maximum, though the MU hardware supports the full
  /// 6 bits.
  ///
  /// \see MUHWI_Destination_t
  ///
  typedef union bgq_coords
  {
    struct
    {
      uint32_t thread   : 2; ///< Hardware thread id, 4 threads per core
      uint32_t a        : 6; ///< Torus 'a' dimension
      uint32_t b        : 6; ///< Torus 'b' dimension
      uint32_t c        : 6; ///< Torus 'c' dimension
      uint32_t d        : 6; ///< Torus 'd' dimension
      uint32_t e        : 1; ///< Torus 'e' dimension, two nodes per node card
      uint32_t reserved : 1; ///< Reserved - possibly to identify the 17th core
      uint32_t core     : 4; ///< Core id, 16 application cores per node
    };
    uint32_t   raw;          ///< Raw memory storage
  } bgq_coords_t;

  typedef struct
  {
    struct
    {
      bgq_coords_t * task2coords;
      uint32_t     * coords2task;
    } torus;
    struct
    {
      size_t       * local2peer;
      size_t       * peer2task;
    } node;
    size_t  size;
    size_t  local_size;//hack
  } bgq_mapcache_t;

  class BgqMapCache
  {
    public:

      inline BgqMapCache ()
      {
      };

      inline ~BgqMapCache ()
      {
      };

      inline size_t init (XMI::BgqPersonality & personality,
                          void                   * ptr,
                          size_t                   bytes
                         )
      {
        return 0;
      };


  }; // XMI::BgqMapCache
};     // XMI

#endif // __xmi_components_mapping_bgq_bgqmapcache_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
