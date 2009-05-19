/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file interfaces/ConnectionManager.h
 * \brief ???
 */

#ifndef   __conn_mgr_h__
#define   __conn_mgr_h__

namespace CCMI
{
  namespace ConnectionManager
  {

    ///
    /// \brief A class to manage multisend connections for
    /// collective protocols
    ///
    class ConnectionManager
    {
    protected:
      int _numConnections;

      void setNumConnections (int nconn)
      {
        _numConnections = nconn;
      }

    public:

      /// Constructor
      ConnectionManager ()
      {
        _numConnections = -1;
      }

      /// Destructor
      virtual ~ConnectionManager ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      ///
      /// \brief return the number of connections that multisend has
      /// to support. The child class will set that up
      ///

      int getNumConnections()
      {
        return _numConnections;
      }

      ///
      /// \brief return the connection id given a set of inputs
      /// \param comm the communicator id of the collective
      /// \param root the root of the collective operation
      ///

      virtual unsigned getConnectionId (unsigned comm, unsigned root,
                                        unsigned color, unsigned phase, unsigned dst) = 0;
      virtual unsigned getRecvConnectionId (unsigned comm, unsigned root, 
                                            unsigned src, unsigned phase, unsigned color) = 0;
    }; //- ConnectionManager
  };  //- namespace ConnectionManager
};  //- CCMI

#endif
