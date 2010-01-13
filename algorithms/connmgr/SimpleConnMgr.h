/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/SimpleConnMgr.h
 * \brief ???
 */

#ifndef __algorithms_connmgr_SimpleConnMgr_h__
#define __algorithms_connmgr_SimpleConnMgr_h__

namespace CCMI
{
  namespace ConnectionManager
  {
    template <class T_Sysdep>
    class SimpleConnMgr : public ConnectionManager<T_Sysdep>
    {
    protected:
      int   _connid;

    public:
      // the conn parm is what the connection id is, since multiple color executors
      //   will still only have one connection manager this parm will be the last
      //   color.
      /* This class is really just a place holder for future extensions.  */
      SimpleConnMgr (int conn=0)
        : ConnectionManager<T_Sysdep>()
      , _connid(0)
      {
        this->setNumConnections (conn == 0 ? 1 : conn );
      }

      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param root the root of the collective operation
      /// \param color the dimension of the collective operation
      virtual unsigned getConnectionId (unsigned comm, unsigned root,
                                        unsigned color, unsigned phase, unsigned dst=(unsigned)-1)
      {
        return _connid;
      }

      virtual unsigned getRecvConnectionId (unsigned comm, unsigned root,
                                            unsigned src, unsigned phase, unsigned color)
      {
        return _connid;
      }
    };
  };
};

#endif
