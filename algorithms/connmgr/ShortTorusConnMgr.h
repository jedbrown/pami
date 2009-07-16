/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file connmgr/ShortTorusConnMgr.h
 * \brief ???
 */

#ifndef  __short_torus_conn_mgr_h__
#define  __short_torus_conn_mgr_h__

#include "ConnectionManager.h"

namespace CCMI
{
  namespace ConnectionManager
  {

    class ShortTorusConnMgr : public ConnectionManager
    {
    protected:
      TorusCollectiveMapping *_mapping;

    public: 
      /// Constructor
      ShortTorusConnMgr (TorusCollectiveMapping *map) : ConnectionManager(), _mapping(map)
      {
        unsigned* dims_lengths = _mapping->DimsLengths();
        unsigned nconn = dims_lengths[CCMI_X_DIM];
        if(nconn < dims_lengths[CCMI_Y_DIM])
          nconn = dims_lengths[CCMI_Y_DIM];
        if(nconn < dims_lengths[CCMI_Z_DIM])
          nconn = dims_lengths[CCMI_Z_DIM];
        if(nconn < dims_lengths[CCMI_T_DIM])
          nconn = dims_lengths[CCMI_T_DIM];

        setNumConnections (nconn);
      }

      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param root the root of the collective operation
      ///
      virtual unsigned getConnectionId (unsigned comm, unsigned root,
                                        unsigned color, unsigned phase, unsigned dst=(unsigned)-1)
      {
        unsigned* my_coords = _mapping->Coords();
        unsigned coords[CCMI_TORUS_NDIMS];
        unsigned conn = (unsigned) -1;
        CCMI_assert (dst != (unsigned)-1);

        _mapping->Rank2Torus(&(coords[0]), dst);


        if(coords[CCMI_X_DIM] != my_coords[CCMI_X_DIM])
          return my_coords[CCMI_X_DIM];
        else if(coords[CCMI_Y_DIM] != my_coords[CCMI_Y_DIM])
          return my_coords[CCMI_Y_DIM];
        else if(coords[CCMI_Z_DIM] != my_coords[CCMI_Z_DIM])
          return my_coords[CCMI_Z_DIM];
        else if(coords[CCMI_T_DIM] != my_coords[CCMI_T_DIM])
          return my_coords[CCMI_T_DIM];

        ///We shouldnt be here!
        CCMI_abort();

        return conn;
      }


      virtual unsigned getRecvConnectionId (unsigned comm, unsigned root, 
                                            unsigned src, unsigned phase, unsigned color)
      {
        unsigned* my_coords = _mapping->Coords();
        unsigned coords[CCMI_TORUS_NDIMS];

        CCMI_assert (src != (unsigned)-1);

        _mapping->Rank2Torus(&(coords[0]), src);        

        unsigned conn = (unsigned) -1;    

        if(coords[CCMI_X_DIM] != my_coords[CCMI_X_DIM])
          return my_coords[CCMI_X_DIM];
        else if(coords[CCMI_Y_DIM] != my_coords[CCMI_Y_DIM])
          return my_coords[CCMI_Y_DIM];
        else if(coords[CCMI_Z_DIM] != my_coords[CCMI_Z_DIM])
          return my_coords[CCMI_Z_DIM];
        else if(coords[CCMI_T_DIM] != my_coords[CCMI_T_DIM])
          return my_coords[CCMI_T_DIM];

        CCMI_abort(); //We shouldnt be here, conn mgr does not
        //permit sending to self
        return conn;
      }
    };
  };
};

#endif
