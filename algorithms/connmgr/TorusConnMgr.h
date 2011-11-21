/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/TorusConnMgr.h
 * \brief ???
 */

#ifndef __algorithms_connmgr_TorusConnMgr_h__
#define __algorithms_connmgr_TorusConnMgr_h__

#include "ConnectionManager.h"

namespace CCMI
{
  namespace ConnectionManager
  {
    ///Connection ids 0 used for broadcast
    ///Multi color allreduce must use different contexts
    const static unsigned  BCAST_0     =  0;
    const static unsigned  BCAST_1     =  1;
    const static unsigned  BCAST_2     =  2;
    const static unsigned  BCAST_3     =  3;
    const static unsigned  BCAST_4     =  4;
    const static unsigned  BCAST_5     =  5;
    const static unsigned  BCAST_6     =  6;
    const static unsigned  BCAST_7     =  7;
    const static unsigned  BCAST_8     =  8;
    const static unsigned  BCAST_9     =  9;
    const static unsigned  DIM_0_P     = 10;
    const static unsigned  DIM_0_N     = 11;
    const static unsigned  DIM_1_P     = 12;
    const static unsigned  DIM_1_N     = 13;
    const static unsigned  DIM_2_P     = 14;
    const static unsigned  DIM_2_N     = 15;
    const static unsigned  DIM_3_P     = 16;
    const static unsigned  DIM_3_N     = 17;
    const static unsigned  DIM_4_P     = 18;
    const static unsigned  DIM_4_N     = 19;
    const static unsigned  LOCAL_START = 20;
    
    static const unsigned NumTorusDims = 5;
    static const unsigned  _torus_connids[2*NumTorusDims] = { DIM_0_P,
							      DIM_0_N,
							      DIM_1_P,
							      DIM_1_N,
							      DIM_2_P,
							      DIM_2_N,
							      DIM_3_P,
							      DIM_3_N,
							      DIM_4_P,
							      DIM_4_N };

    const static unsigned GhostPhase = 6;   

    class TorusConnMgr : public ConnectionManager<TorusConnMgr>
    {
      protected:
        pami_coord_t   _self_coords;
      public:
        /// Constructor
	TorusConnMgr () : ConnectionManager<TorusConnMgr>()
        {
	  __global.mapping.task2network( __global.mapping.task(), 
					 &_self_coords, 
					 PAMI_N_TORUS_NETWORK );
        }

        ///
        /// \brief return the connection id given a set of inputs
        /// \param comm the communicator id of the collective
        /// \param root the root of the collective operation
        ///
	virtual unsigned getConnectionId_impl ( unsigned    comm, 
						unsigned    root,
						unsigned    color, 
						unsigned    phase, 
						unsigned    dst=(unsigned)-1 )
        {
	  unsigned conn = (unsigned)-1;
	  if (dst == (unsigned)-1)
	    return color;

	  if (__global.mapping.isPeer(dst, __global.mapping.task())) 
	    return phase + LOCAL_START;	    

          pami_coord_t dst_coords;
	  __global.mapping.task2network(dst, &dst_coords, PAMI_N_TORUS_NETWORK);
	  
	  unsigned i = 0;
	  for (i = 0; i < NumTorusDims; ++i)
	    if (_self_coords.u.n_torus.coords[i] != dst_coords.u.n_torus.coords[i])
	      break;
	  
	  int dir = 0; //-ve
	  if (color >= NumTorusDims)
	    dir = 1;   //+ve
	  if (phase == GhostPhase)
	    dir = !dir; //reverse directions in ghost phase
	  
	  if (i != NumTorusDims) {
	    if (dir == 1)
	      conn = _torus_connids[2*i];   //+ve direction
	    else 	 
	      conn = _torus_connids[2*i + 1];   //-ve direction
	  }
	  else 
	    CCMI_abort(); //error case
	  
	  //printf("TorusConnMgr::getConnectionId conn %d dest %d color %d\n", 
	  // conn, dst, color);
	  
          return conn;
        }


	virtual unsigned getRecvConnectionId_impl (unsigned    comm, 
						   unsigned    root,
						   unsigned    src, 
						   unsigned    phase, 
						   unsigned    color)
        {
	  unsigned conn = (unsigned)-1;
	  if (src == (unsigned)-1)
	    return color;  //BCAST

	  if (__global.mapping.isPeer(src, __global.mapping.task())) 
	    return phase + LOCAL_START;	    

          pami_coord_t src_coords;
	  __global.mapping.task2network(src, &src_coords, PAMI_N_TORUS_NETWORK);
	  
	  unsigned i = 0;
	  for (i = 0; i < NumTorusDims; ++i)
	    if (_self_coords.u.n_torus.coords[i] != src_coords.u.n_torus.coords[i])
	      break;

	  int dir = 0; //-ve
	  if (color >= NumTorusDims)
	    dir = 1;   //+ve
	  if (phase == GhostPhase)
	    dir = !dir; //reverse directions in ghost phase
	  
	  if (i != NumTorusDims) {
	    if (dir == 1)
	      conn = _torus_connids[2*i];   //+ve direction
	    else 	 
	      conn = _torus_connids[2*i + 1];   //-ve direction
	  }
	  else 
	    CCMI_abort(); //error case

	  //printf("TorusConnMgr::getRecvConnectionId conn %d src %d color %d\n", 
	  // conn, src, color);
          return conn;
        }

	virtual void setNumConnections_impl(int nconn)
        {
        }

        virtual int getNumConnections_impl ()
        {
	  return 16; //10 for torus, 5 for bcast 1 for local
        }
    };
  };
};

#endif
