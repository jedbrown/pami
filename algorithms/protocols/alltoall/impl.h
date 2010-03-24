/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/alltoall/impl.h
 * \brief CCMI composite template implementations
 */

#ifndef __algorithms_protocols_alltoall_impl_h__
#define __algorithms_protocols_alltoall_impl_h__

#include "algorithms/protocols/alltoall/Alltoall.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Alltoall
    {
      typedef CCMI::Adaptor::A2AProtocol <PAMI_COLL_M2M_CLASS, PAMI_SYSDEP_CLASS, size_t> AlltoallProtocol;
      typedef CCMI::Adaptor::AlltoallFactory <PAMI_COLL_M2M_CLASS, PAMI_SYSDEP_CLASS, size_t> AlltoallFactory;
    };
  };
};  //namespace CCMI::Adaptor::Alltoall

#endif
