/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/connmgr/RankBasedConnMgr.h
 * \brief ???
 */

#ifndef __algorithms_connmgr_RankBasedConnMgr_h__
#define __algorithms_connmgr_RankBasedConnMgr_h__

#include "ConnectionManager.h"
#include "Global.h"

namespace CCMI
{
  namespace ConnectionManager
  {
    template <class T_Sysdep>
    class RankBasedConnMgr : public ConnectionManager<RankBasedConnMgr<T_Sysdep> >
    {
    public:
      RankBasedConnMgr (T_Sysdep *sd) :
        ConnectionManager<RankBasedConnMgr<T_Sysdep> >(),
        _sysdep(sd),
        _numConnections(__global.mapping.size())
        {
        }

      inline void setNumConnections (size_t sz)
        {
          _numConnections = sz;
        }

      inline unsigned getConnectionId_impl (unsigned comm,
                                            unsigned root,
                                            unsigned color,
                                            unsigned phase,
                                            unsigned dst=(unsigned)-1)
        {
          return __global.mapping.task();
        }

      inline unsigned getRecvConnectionId_impl (unsigned comm,
                                                unsigned root,
                                                unsigned src,
                                                unsigned phase,
                                                unsigned color)
        {
          return src;
        }
    private:
      T_Sysdep *_sysdep;
      size_t    _numConnections;
    };
  };
};

#endif
