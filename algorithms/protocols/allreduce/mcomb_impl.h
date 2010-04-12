/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/mcomb_impl.h
 * \brief Implement the simple multicombine allreduce factory
 */

#ifndef __algorithms_protocols_allreduce_mcomb_impl_h__
#define __algorithms_protocols_allreduce_mcomb_impl_h__

#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/protocols/allreduce/MultiCombineComposite.h"
#include "algorithms/protocols/AllSidedCollectiveProtocolFactoryT.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {

      void mcomb_allreduce_md(pami_metadata_t *m)
      {
        // \todo:  fill in other metadata
        strcpy(&m->name[0],"CCMIMcombAllreduce<");
      }

      typedef AllSidedCollectiveProtocolFactoryT<MultiCombineComposite,
                                                 mcomb_allreduce_md,
                                                 ConnectionManager::SimpleConnMgr<PAMI_SYSDEP_CLASS> > MultiCombineFactory;
    };
  };
};

#endif
