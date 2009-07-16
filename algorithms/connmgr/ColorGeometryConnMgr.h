/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file connmgr/ColorGeometryConnMgr.h
 * \brief ???
 */

#ifndef  __cg_conn_mgr_h__
#define  __cg_conn_mgr_h__

#include "algorithms/schedule/Rectangle.h"

namespace CCMI
{
  namespace ConnectionManager
  {

    class ColorGeometryConnMgr : public ConnectionManager
    {
    public:
      enum
      {
        color0 = 0x00000000, color1=0x00010000, color2=0x00020000, color3=0x00030000, color4=0x00040000, color5=0x00050000
      };


      // the conn parm is what the connection id is, since multiple color executors
      //   will still only have one connection manager this parm will be the last
      //   color.
      /* This class is really just a place holder for future extensions.  */
      ColorGeometryConnMgr (int conn=0) 
      : ConnectionManager() 
      {
        setNumConnections (conn == 0 ? 1 : conn );
      }
      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param root the root of the collective operation
      /// \param color the dimension of the collective operation
      virtual unsigned getConnectionId (unsigned comm, unsigned root,
                                        unsigned color, unsigned phase, unsigned dst=(unsigned)-1)
      {

        assert(_numConnections != 0);

        unsigned connid = 0;

        if(color == 0 || // this is for legacy operation, we expect 1-based colors
           color == 1)
          connid = comm | color0;
        else if(color == 2)
          connid = comm | color1;
        else if(color == 3)
          connid = comm | color2;
        else if(color == 4)
          connid = comm | color3;
        else if(color == 5)
          connid = comm | color4;
        else if(color == 6)
          connid = comm | color5;
        else
          assert(color <= 6);

        return connid;  
      }

      virtual unsigned getRecvConnectionId (unsigned comm, unsigned root, 
                                            unsigned src, unsigned phase, unsigned color)
      {
        return getConnectionId (comm, root, color, phase);
      }
    };
  };
};

#endif
