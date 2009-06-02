/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file connmgr/RankBasedConnMgr.h
 * \brief ???
 */

#ifndef  __rank_conn_mgr_h__
#define  __rank_conn_mgr_h__

#include "ConnectionManager.h"

namespace CCMI
{
  namespace ConnectionManager
  {

    class RankBasedConnMgr : public ConnectionManager
    {
    protected:
      Mapping *_mapping;

    public:

      /// Constructor

      RankBasedConnMgr (Mapping *map) : ConnectionManager(), _mapping(map)
      {
        setNumConnections (_mapping->size());
      }

      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param root the root of the collective operation
      ///

      virtual unsigned getConnectionId (unsigned comm, unsigned root,
                                        unsigned color, unsigned phase, unsigned dst=(unsigned)-1)
      {
        return _mapping->rank();
      }
      virtual unsigned getRecvConnectionId (unsigned comm, unsigned root, 
                                            unsigned src, unsigned phase, unsigned color)
      {
        return src;
      }
    };
  };
};

#endif
