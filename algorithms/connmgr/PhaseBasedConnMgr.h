/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/PhaseBasedConnMgr.h
 * \brief ???
 */

#ifndef __algorithms_connmgr_PhaseBasedConnMgr_h__
#define __algorithms_connmgr_PhaseBasedConnMgr_h__

#include "ConnectionManager.h"

///
/// This connection manager uses the phase of the collective for the
/// connection id. This is suitable for collectives which only receive
/// one message in each phase. The number of outstanding connections
/// is probably smaller than RankBasedConnection manager for example
///

namespace CCMI
{
  namespace ConnectionManager
  {

    class PhaseBasedConnMgr : public ConnectionManager
    {
      protected:
        CollectiveMapping *_mapping;

      public:

        /// Constructor
        PhaseBasedConnMgr (CollectiveMapping *map) : ConnectionManager(), _mapping(map)
        {
          setNumConnections (-1);
        }

        ///
        /// \brief return the connection id given a set of inputs
        /// \param comm the communicator id of the collective
        /// \param root the root of the collective operation
        ///

        virtual unsigned getConnectionId (unsigned comm, unsigned root,
                                          unsigned color, unsigned phase, unsigned dst = (unsigned) - 1)
        {
          return phase;
        }
        virtual unsigned getRecvConnectionId (unsigned comm, unsigned root, unsigned src,
                                              unsigned phase, unsigned color)
        {
          return phase;
        }
    };
  };
};

#endif
