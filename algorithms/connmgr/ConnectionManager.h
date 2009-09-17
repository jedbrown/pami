/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/ConnectionManager.h
 * \brief ???
 */

#ifndef   __conn_mgr_h__
#define   __conn_mgr_h__

namespace CCMI
{
  namespace ConnectionManager
  {
    template <class T_ConnectionManager>
    class ConnectionManager
    {
    public:
      ConnectionManager () {}
      inline void     setNumConnections (int nconn);
      inline int      getNumConnections();
      inline unsigned getConnectionId (unsigned comm, unsigned root,
                                       unsigned color, unsigned phase, unsigned dst);
      inline unsigned getRecvConnectionId (unsigned comm, unsigned root,
                                           unsigned src, unsigned phase, unsigned color);
    }; //- ConnectionManager


    template <class T_ConnectionManager>
    inline void ConnectionManager<T_ConnectionManager>::setNumConnections(int nconn)
    {
      static_cast<T_ConnectionManager*>(this)->setNumConnections_impl(nconn);
    }

    template <class T_ConnectionManager>
    inline int      ConnectionManager<T_ConnectionManager>::getNumConnections()
    {
      return static_cast<T_ConnectionManager*>(this)->getNumConnections_impl();
    }

    template <class T_ConnectionManager>
    inline unsigned ConnectionManager<T_ConnectionManager>::getConnectionId (unsigned comm, unsigned root,
                                                                             unsigned color, unsigned phase, unsigned dst)
    {
      return static_cast<T_ConnectionManager*>(this)->getConnectionId_impl(comm,
                                                                           root,
                                                                           color,
                                                                           phase,
                                                                           dst);
    }

    template <class T_ConnectionManager>
    inline unsigned ConnectionManager<T_ConnectionManager>::getRecvConnectionId (unsigned comm, unsigned root,
                                                                                 unsigned src, unsigned phase, unsigned color)
    {
      return static_cast<T_ConnectionManager*>(this)->getRecvConnectionId_impl(comm,
                                                                               root,
                                                                               src,
                                                                               phase,
                                                                               color);
    }
  };  //- namespace ConnectionManager
};  //- CCMI

#endif
