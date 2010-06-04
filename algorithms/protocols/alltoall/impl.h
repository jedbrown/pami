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
#include "algorithms/protocols/alltoall/All2All.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Alltoall
    {
      void getAlltoallMetaData(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"Alltoall");
      }      
      typedef CCMI::Adaptor::All2AllProtocol All2AllProtocol;
      typedef CCMI::Adaptor::All2AllFactoryT <All2AllProtocol, getAlltoallMetaData, CCMI::ConnectionManager::CommSeqConnMgr> All2AllFactory;

      // old CCMI
      typedef CCMI::Adaptor::A2AProtocol <PAMI_COLL_M2M_CLASS, PAMI_SYSDEP_CLASS, size_t> AlltoallProtocol;
      typedef CCMI::Adaptor::AlltoallFactory <PAMI_COLL_M2M_CLASS, PAMI_SYSDEP_CLASS, size_t> AlltoallFactory;
      
    };
  };
};  //namespace CCMI::Adaptor::Alltoall

#endif

