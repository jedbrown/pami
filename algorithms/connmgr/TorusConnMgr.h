/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/TorusConnMgr.h
 * \brief ???
 */

#ifndef  __torus_conn_mgr_h__
#define  __torus_conn_mgr_h__

#include "ConnectionManager.h"

namespace CCMI
{
  namespace ConnectionManager
  {

    class TorusConnMgr : public ConnectionManager
    {
    protected:
      TorusCollectiveMapping *_mapping;

    public:

      ///Connection ids 0,1,2,3 are used for the three color broadcast
      const static unsigned  XPCONN     =  4;
      const static unsigned  XNCONN     =  5;
      const static unsigned  YPCONN     =  6;
      const static unsigned  YNCONN     =  7;
      const static unsigned  ZPCONN     =  8;
      const static unsigned  ZNCONN     =  9;
      const static unsigned  TCONN      = 10;   //local connection id
      const static unsigned  NCONN      = 14;   //number of connections

      /// Constructor
      TorusConnMgr (TorusCollectiveMapping *map) : ConnectionManager(), _mapping(map)
      {
        setNumConnections (NCONN);
      }

      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param root the root of the collective operation
      ///
      virtual unsigned getConnectionId (unsigned comm, unsigned root,
                                        unsigned color, unsigned phase, unsigned dst=(unsigned)-1)
      {

        CCMI_assert (color <= 3);
        CCMI_assert (color >= 1);
        CCMI_assert (dst != (unsigned)-1);

        unsigned conn = (unsigned) -1;
        unsigned coords[CCMI_TORUS_NDIMS];
        unsigned* my_coords = _mapping->Coords();
        _mapping->Rank2Torus (&(coords[0]), dst);

        if(coords[CCMI_X_DIM] != my_coords[CCMI_X_DIM])
        {
          if(coords[CCMI_X_DIM] > my_coords[CCMI_X_DIM])
            conn = XPCONN;   //X+ direction
          else
            conn = XNCONN;   //X- direction
        }
        else if(coords[CCMI_Y_DIM] != my_coords[CCMI_Y_DIM])
        {
          if(coords[CCMI_Y_DIM] > my_coords[CCMI_Y_DIM])
            conn = YPCONN;   //Y+ direction
          else
            conn = YNCONN;   //Y- direction
        }
        else if(coords[CCMI_Z_DIM] != my_coords[CCMI_Z_DIM])
        {
          if(coords[CCMI_Z_DIM] > my_coords[CCMI_Z_DIM])
            conn = ZPCONN;   //Z+ direction
          else
            conn = ZNCONN;   //Z- direction
        }
        else if(coords[CCMI_T_DIM] != my_coords[CCMI_T_DIM])
          conn = TCONN + color - 1;

        CCMI_assert (conn != (unsigned)-1);
        return conn;
      }


      virtual unsigned getRecvConnectionId (unsigned comm, unsigned root,
                                            unsigned src, unsigned phase, unsigned color)
      {
        CCMI_assert (color <= 3);
        CCMI_assert (color >= 1);
        CCMI_assert (src != (unsigned)-1);
        unsigned coords[CCMI_TORUS_NDIMS];
        unsigned* my_coords = _mapping->Coords();
        _mapping->Rank2Torus (&(coords[0]), src);

        unsigned conn = (unsigned) -1;

        if(coords[CCMI_X_DIM] != my_coords[CCMI_X_DIM])
        {
          if(coords[CCMI_X_DIM] > my_coords[CCMI_X_DIM])
            conn = XNCONN;   //X+ direction
          else
            conn = XPCONN;   //X- direction
        }
        else if(coords[CCMI_Y_DIM] != my_coords[CCMI_Y_DIM])
        {
          if(coords[CCMI_Y_DIM] > my_coords[CCMI_Y_DIM])
            conn = YNCONN;   //Y+ direction
          else
            conn = YPCONN;   //Y- direction
        }
        else if(coords[CCMI_Z_DIM] != my_coords[CCMI_Z_DIM])
        {
          if(coords[CCMI_Z_DIM] > my_coords[CCMI_Z_DIM])
            conn = ZNCONN;   //Z+ direction
          else
            conn = ZPCONN;   //Z- direction
        }
        else if(coords[CCMI_T_DIM] != my_coords[CCMI_T_DIM])
          conn = TCONN + color - 1;

        CCMI_assert (conn != (unsigned)-1);
        return conn;
      }
    };
  };
};

#endif
