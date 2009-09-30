/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/sysdep/SysDep.h
/// \brief ???
///
#ifndef __components_sysdep_sysdep_h__
#define __components_sysdep_sysdep_h__

#include "sys/xmi.h"
#include "util/common.h"

namespace XMI
{
  namespace SysDep
  {
    ///
    /// \param T_Memory   Platform-specific memory manager class
    /// \param T_Mapping  Platform-specific mapping class
    /// \param T_Time     Platform-specific time class
    /// \param T_Topology Platform-specific topology class
    ///
    template <class T_Memory, class T_Mapping, class T_Time, class T_Topology>
    class SysDep
    {
      public:
        inline SysDep () :
            mm (),
            mapping (),
            time (),
            topology_local (),
            topology_global ()
        {
	  xmi_coord_t ll, ur;
	  size_t min=0, max=0;
          mapping.init (ll, ur, min, max, mm);
	  // no one can use a Topology until after this point...
	  T_Topology::static_init(&mapping);
	  size_t rectsize = 1;
	  for (unsigned d = 0; d < mapping.globalDims(); ++d) {
	  	rectsize *= (ur.n_torus.coords[d] - ll.n_torus.coords[d] + 1);
	  }
	  if (mapping.size() == rectsize) {
	    new (&topology_global) T_Topology(&ll, &ur);
	  } else if (mapping.size() == max - min + 1) {
	    new (&topology_global) T_Topology(min, max);
	  } else {
	    // wait for COMM_WORLD so we don't allocate yet-another ranks list?
	    // actually, COMM_WORLD should use our rank list...
	    // does this ever happen for "COMM_WORLD"?
	    // (isn't COMM_WORLD, by definition, a contig set of ranks 0..(N-1)?)
	    // topology_global(ranks, nranks);
	    XMI_abortf("failed to build global-world topology");
	  }
	  topology_global.subTopologyLocalToMe(&topology_local);
        };

        T_Memory   mm;
        T_Mapping  mapping;
        T_Time     time;
        T_Topology topology_local;
        T_Topology topology_global;
    };

    class NullSysDep
    {
    }; // class NullSysDep
  };
};
#endif // __components_sysdep_sysdep_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
