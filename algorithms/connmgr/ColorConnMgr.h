/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/ColorConnMgr.h
 * \brief ???
 */

#ifndef __algorithms_connmgr_ColorConnMgr_h__
#define __algorithms_connmgr_ColorConnMgr_h__

#include "ConnectionManager.h"

#define MAX_COLORS  15  //colors 0 - 14

namespace CCMI
{
  namespace ConnectionManager
  {
    class ColorConnMgr : public ConnectionManager<ColorConnMgr>
    {
      unsigned  _offset;
    public:
      
      /// Constructor
      ColorConnMgr () : ConnectionManager<ColorConnMgr> (), _offset(0)
	{
	}

      /// Constructor
      ColorConnMgr (unsigned offset) : ConnectionManager<ColorConnMgr> (), _offset(offset)
	{
	}

      inline void setNumConnections_impl (size_t sz)
	{
	}
      
      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param root the root of the collective operation
      /// \param color the dimension of the collective operation
      inline unsigned getConnectionId_impl (unsigned comm, unsigned root,
                                            unsigned color, unsigned phase, unsigned dst=(unsigned)-1)
	{
	  return color + _offset;
	}
      
      inline unsigned getRecvConnectionId_impl (unsigned comm, unsigned root,
						unsigned src, unsigned phase, unsigned color)
	{
	  return color + _offset;
	}
      
      virtual int getNumConnections_impl ()
      {
	return MAX_COLORS;
      }
    };
  };
};

#endif
