/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/tree/ShortTreeFactory.h
 * \brief ???
 */

#ifndef __algorithms_protocols_broadcast_tree_ShortTreeFactory_h__
#define __algorithms_protocols_broadcast_tree_ShortTreeFactory_h__

//#include "algorithms/schedule/TreeSchedule.h"
//#include "algorithms/schedule/TreeBwSchedule.h"
//#include "connmgr/SimpleConnMgr.h"
//#include "protocols/broadcast/multi_color_impl.h"
#include "./ShortTreeBcast.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      namespace Tree
      {
        ///
        /// \brief ShortTreeFactory class for broadcast factory implementations.
        ///
        class ShortTreeFactory : public BroadcastFactory<CCMI::TorusCollectiveMapping>
        {
        public:
          ///
          /// \brief Constructor for broadcast factory implementations.
          ///
          ShortTreeFactory
          (CCMI::TorusCollectiveMapping                        * map,
           CCMI::MultiSend::OldMulticastInterface        * mf,
           CCMI::ConnectionManager::ConnectionManager * cmgr,
           unsigned                                    nconn)
          : BroadcastFactory<CCMI::TorusCollectiveMapping>(mf, map, cmgr, nconn),
                        _map(map),
                        _mf(mf)

                        //tree_bcast(mf, this, map)
          {
            CCMI::Adaptor::Broadcast::Tree::ShortTreeBcast::init(map);
          }

          ///\brief Tree only works on comm_world
          virtual bool Analyze(Geometry *geometry)
          {
            return global_analyze (geometry);
          }

          ///
          /// \brief Generate a non-blocking broadcast message.
          ///
          /// \param[in]  request      Opaque memory to maintain
          ///                          internal message state.
          /// \param[in]  cb_done      Callback to invoke when
          ///                message is complete.
          /// \param[in]  consistency  Required consistency level
          /// \param[in]  geometry     Geometry to use for this
          ///                collective operation.
          ///                          \c NULL indicates the global geometry.
          /// \param[in]  root         Rank of the node performing
          ///                the broadcast.
          /// \param[in]  src          Source buffer to broadcast.
          /// \param[in]  bytes        Number of bytes to broadcast.
          ///
          /// \retval     0            Success
          /// \retval     1            Unavailable in geometry
          ///

          virtual CCMI::Executor::Composite * generate
          (void                      * request,
           size_t                      rsize,
           PAMI_Callback_t             cb_done,
           CCMI_Consistency            consistency,
           Geometry                  * geometry,
           unsigned                    root,
           char                      * src,
           unsigned                    bytes)
          {
            TRACE_ADAPTOR ((stderr, "Tree Broadcast Generate\n"));

            PAMI_assert(rsize >= sizeof(ShortTreeBcast));
            new (_request)ShortTreeBcast(_mf, _map, request, cb_done, consistency, src, bytes, root);
                        return NULL;
          }

                  private:

                  //ShortTreeBcast tree_bcast;
          CCMI::TorusCollectiveMapping                         * _map;
          CCMI::MultiSend::OldMulticastInterface        * _mf;
          PAMI_Request_t    _request __attribute__((__aligned__(16)));


        };

      };
    };
  };
};

#endif
