/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef __xmi_common_global_h__
#define __xmi_common_global_h__

///
/// \file components/sysdep/Global.h
/// \brief Blue Gene/P Global Object
///
/// This global object is constructed before main() and is a container class
/// for all other classes that need to be constructed and initialized before
/// the application starts.
///

#include "Platform.h"
#include "util/common.h"
#include "Topology.h"
#include "Wtime.h"

namespace XMI
{
namespace Interface
{
    template<class T_Global>
    class Global
    {
      public:

        inline Global() :
	  time(),
	  topology_global(),
	  topology_local()
        {
		ENFORCE_CLASS_MEMBER(T_Global,mapping);
        }

        inline ~Global () {};

      public:

	XMI::Time	time;
	XMI::Topology	topology_global;
	XMI::Topology	topology_local;

  };   // class Global
};     // namespace Interface
};     // namespace XMI

#endif // __xmi_common_global_h__
