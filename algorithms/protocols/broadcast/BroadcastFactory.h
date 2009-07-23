/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/./BroadcastFactory.h
 * \brief ???
 */

#ifndef __ccmi_adaptor_broadcast_factory_h__
#define __ccmi_adaptor_broadcast_factory_h__

#include "algorithms/executor/Broadcast.h"
#include "algorithms/composite/Composite.h"
#include "interface/MultiSend.h"
#include "algorithms/connmgr/SimpleConnMgr.h"
#include "algorithms/connmgr/RankBasedConnMgr.h"
#include "interface/Geometry.h"
#include "algorithms/protocols/CollectiveProtocolFactory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      ///
      /// \brief choose if this protocol is supports the input geometry
      ///
      typedef bool      (*AnalyzeFn)   (Geometry                  * g);

      inline bool true_analyze (Geometry *geometry)
      {
        return true;
      }
      inline bool global_analyze   (Geometry *geometry)
      {
        return(geometry->isGlobalContext()); 
      }      

      inline bool rectangle_analyze (Geometry *geometry)
      {
        return geometry->isRectangle();
      }      

      ///
      /// \brief Base virtual factory class for synchronous broadcast factory
      ///  implementations.
      ///
      class BaseBroadcastFactory : public CollectiveProtocolFactory
      {
      protected:
      public:
        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }


        /// \brief All protocols determine if a given geometry is supported
        virtual bool Analyze(CCMI::Adaptor::Geometry *grequest)
        {
          CCMI_abort(); return 0;
        }


        ///
        /// \brief Generate a non-blocking broadcast message.
        ///
        /// \param[in]  request      Opaque memory to maintain
        ///                          internal message state.
        /// \param[in] rsize         size of the request object. 
        ///                          In async broadcast smaller buffer can be 
        ///                          passed in as it doesent use multiple colors.
        /// \param[in]  cb_done      Callback to invoke when
        ///				 message is complete.
        /// \param[in]  consistency  Required consistency level
        /// \param[in]  geometry     Geometry to use for this
        ///				 collective operation.
        ///                          \c NULL indicates the global geometry.
        /// \param[in]  root         Rank of the node performing
        ///				 the broadcast.
        /// \param[in]  src          Source buffer to broadcast.
        /// \param[in]  bytes        Number of bytes to broadcast.
        ///
        /// \retval     0            Success
        /// \retval     1            Unavailable in geometry
        ///	
        virtual CCMI::Executor::Composite * generate
        (void                      * request_buf,
         size_t                      rsize,
         CM_Callback_t             cb_done,
         CCMI_Consistency            consistency,
         Geometry                  * geometry,
         unsigned                    root,
         char                      * src,
         unsigned                    bytes)=0;

      };  //- Base Broadcast Factory
      ///
      /// \brief Base factory class for synchronous broadcast factory
      ///  implementations.
      ///
      template <class MAP>
      class BroadcastFactory : public BaseBroadcastFactory
      {
      protected:
        ///
        /// \brief Multisend interface
        ///
        CCMI::MultiSend::OldMulticastInterface   * _minterface;

        ///
        ///  \brief Connection Manager for the broadcast
        ///
        CCMI::ConnectionManager::ConnectionManager   * _connmgr;

        ///
        /// \brief CollectiveMapping module
        ///
        MAP                          * _mapping;

        ///
        /// \brief async broadcast handler
        ///
        CCMI_RecvAsyncBroadcast            _cb_async;

        ///
        /// \brief get geometry from comm id
        ///
        CCMI_mapIdToGeometry               _cb_geometry;

        ///
        /// \brief Callback for one-sided broadcasts
        ///
        CCMI::MultiSend::LL_OldRecvMulticast_t      _cb_head;

        ///
        /// \brief Callback for two-sided broadcasts
        ///
        CCMI::MultiSend::LL_OldRecvMulticast_t      _cb_head_buffered;  

        bool                      _isBuffered;

      public:
        /// NOTE: This is required to make "C" programs link successfully with virtual destructors
        void operator delete(void * p)
        {
          CCMI_abort();
        }

        ///
        /// \brief Constructor for broadcast factory implementations.
        ///
        BroadcastFactory
        (CCMI::MultiSend::OldMulticastInterface          * minterface,
         MAP                                             * mapping,
         CCMI::ConnectionManager::ConnectionManager      * connmgr,
         unsigned                                          nconn,
         CCMI::MultiSend::LL_OldRecvMulticast_t          cb_head = NULL,
         CCMI::MultiSend::LL_OldRecvMulticast_t          cb_head_buffered = NULL ) :
        _minterface (minterface), _connmgr(connmgr), _mapping (mapping),
        _cb_async(NULL), _cb_geometry(NULL), _cb_head(cb_head), 
        _cb_head_buffered (cb_head_buffered), _isBuffered (true)    
        {
        };

        ///
        /// \brief Utility functions
        ///
        void setAsyncInfo (bool                     is_buffered,
                           CCMI_RecvAsyncBroadcast  cb_async,
                           CCMI_mapIdToGeometry     cb_geometry)
        {
          _isBuffered = is_buffered;

          if(!_isBuffered)
          {
            CCMI_assert (cb_async != NULL);
            _cb_async    =  cb_async;
            _minterface->setCallback (_cb_head, this);
          }
          else
            _minterface->setCallback (_cb_head_buffered, this);

          _cb_geometry =  cb_geometry;
        }

        void setConnectionManager (CCMI::ConnectionManager::ConnectionManager  * connmgr)
        {
          _connmgr = connmgr;
        }

      };  //- Broadcast Factory
    };  //- end namespace Collective
  };  //- end namespace Broadcast
};  //- end namespace CCMI

#endif
