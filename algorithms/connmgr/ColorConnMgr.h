/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file connmgr/ColorConnMgr.h
 * \brief ???
 */

#ifndef  __color_conn_mgr_h__
#define  __color_conn_mgr_h__

#include "interfaces/ConnectionManager.h"

#define MAX_COLORS  7  //colors 0 - 6

namespace CCMI
{
  namespace ConnectionManager
  {

    class ColorConnMgr : public ConnectionManager
    {
    public:

      /// Constructor
      ColorConnMgr () : ConnectionManager()
      {
        setNumConnections (MAX_COLORS);
      }

      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param root the root of the collective operation
      ///

      virtual unsigned getConnectionId (unsigned comm, unsigned root,
                                        unsigned color, unsigned phase, unsigned dst=(unsigned)-1)
      {
        return color;
      }

      virtual unsigned getRecvConnectionId (unsigned comm, unsigned root, 
                                            unsigned src, unsigned phase, unsigned color)
      {
        return color;
      }
    };
  };
};

#endif
