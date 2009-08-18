/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/broadcast/tree/ShortTreeBcast.h
 * \brief ???
 */

#ifndef __ccmi_adaptor_broadcast_short_tree_h__
#define __ccmi_adaptor_broadcast_short_tree_h__

#include "multisend/multisend_impl.h"

//#include "../multi_color_impl.h"
#include "interface/TorusCollectiveMapping.h"
#include "util/ccmi_debug.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Broadcast
    {
      namespace Tree
      {
        ///
        /// \brief Tree broadcast protocol for short msg in VN mode
        ///
        /// 
        class ShortTreeBcast 
        {

          //static const unsigned MaxDataBytes = 4096;
          static const unsigned MaxDataBytes = 32768;

          typedef struct _SharedData
          {
            char dst[MaxDataBytes]  __attribute__((__aligned__(16)));
            //const void * srcs[ MAX_NUM_CORES ];
            char pad[32];

            struct 
            {
              volatile unsigned isSrcReady;
              volatile unsigned isDstReady;
              char src[MaxDataBytes]  __attribute__((__aligned__(16)));
              char pad[32];
            } client[ MAX_NUM_CORES ];

          } SharedData __attribute__((__aligned__(32)));

        public:

          ShortTreeBcast (CCMI::MultiSend::OldMulticastInterface * mf,
                          CCMI::TorusCollectiveMapping                  * mapping,
                          void  *request,
                          XMI_Callback_t           & cb_done,
                          CCMI_Consistency            consistency,
                          char                      * buf,
                          unsigned                    count,
                          int                         root) :
          _mapping      (mapping),
          _mcast        ((DCMF::Collectives::MultiSend::OldMulticastImpl *)mf),
          _dstbuf       (NULL),
          _op           (XMI_UNDEFINED_OP),
          _dt           (XMI_UNDEFINED_DT),
          _count        (0),
          _bytes        (0),
          _masterCore   (_minTCoord),
          _numCores     (_numPeers)
          {
            CCMI_assert( _shared != NULL );

            _opcodes[0] = CCMI_TREE_BCAST;

            //_mcastArgs.setRequestBuffer( &_reqMcast );
            _mcastArgs.setCallback     ( NULL, NULL );
            //_mcastArgs.setConsistency  ( CCMI_MATCH_CONSISTENCY );
            _mcastArgs.setInfo         ( NULL, 1 );  // ???
            //_mcastArgs.setConnectionId ( 1 );
            _mcastArgs.setOpcodes      ( _opcodes );
            _mcastArgs.setRanks        ( NULL, 1 ); // ???
            _mcastArgs.setFlags        ( CCMI::MultiSend::CCMI_FLAGS_UNSET );

            //_mcastRecvArgs.setRequestBuffer( &_reqMcastRecv );
            _mcastRecvArgs.setCallback     (  cb_treeRecvDone, this );
            _mcastRecvArgs.setConnectionId ( 1 );
            _mcastRecvArgs.setPipelineWidth( 0x7fffffff );

            internal_restart(request, cb_done, consistency, buf, count, root);

          }

          virtual ~ShortTreeBcast() { CCMI_abort();};

          static void init( CCMI::TorusCollectiveMapping * mapping );

          static void cb_treeRecvDone( void *me, XMI_Error_t *err )
          {
            ShortTreeBcast * a = (ShortTreeBcast *)me;

            asm volatile ("mbar" ::: "memory");

            // notify peers that result is ready
            a->_shared->client[ 1 ].isDstReady = 1;
            a->_shared->client[ 2 ].isDstReady = 1;
            a->_shared->client[ 3 ].isDstReady = 1;

            // local completion 
            a->done();
          }

          void done()
          {

            TRACE_ADAPTOR((stderr,"Broadcast::Tree::ShortTreeBcast::done \n"));

            assert (_dstbuf != NULL);

            // copy result into user dstbuf ...
            if(_numPeers != 1)
            {
              if(_isRootVnPeer)
              {

                if(_mapping->GetCoord(CCMI_T_DIM) != _rootTcoord)
                  memcpy( _dstbuf,  _shared->client[ _rootTcoord ].src, _bytes );
                //memcpy( _dstbuf, _shared->dst, _bytes );

              }
              else
              {
                memcpy( _dstbuf, _shared->dst, _bytes );
              }  
            }

            // completion callback
            if(_cb_done.function)
            {
              (*_cb_done.function)(_cb_done.clientdata, NULL);
            }
          }

          virtual unsigned internal_restart   ( void  * request,
                                                XMI_Callback_t           & cb_done,
                                                CCMI_Consistency            consistency,
                                                char                      * buf,
                                                unsigned                    count,
                                                int                         root ) 
          {
            TRACE_ADAPTOR((stderr,"Broadcast::Tree::ShortTreeBcast::restart(), root:%d\n ",root));

            _cb_done.function   = cb_done.function;
            _cb_done.clientdata = cb_done.clientdata;

            _dstbuf = buf;
            _bytes  = count; 


            if(_bytes > MaxDataBytes)
            {
              return XMI_INVAL;
            }

            unsigned tCoord = _mapping->GetCoord(CCMI_T_DIM);
            _isRootVnPeer =  is_vn_peer(root, _rootTcoord);

            //Not the master core
            if(tCoord != _masterCore)
            {
              if(_mapping->rank() == (unsigned) root)  //copy the data into shared buffer
              {
                memcpy( _shared->client[ tCoord ].src, buf, _bytes );
                asm volatile ("mbar" ::: "memory");
              }
              _shared->client[ tCoord ].isSrcReady = 1;

              TRACE_ADAPTOR((stderr,"Waiting for the master\n "));

              // (busy) wait for the result 
              while((volatile unsigned)_shared->client[ tCoord ].isDstReady == 0); 

              // reset result ready flag for the next run
              _shared->client[ tCoord ].isDstReady = 0;
              // local completion
              done();
            }
            else // master core writing/reading the tree
            {
              // wait for peers to copy  src data into the shared buffer 
              volatile unsigned num;
              do
              {
                num = 
                (volatile unsigned)_shared->client[ 1 ].isSrcReady +
                (volatile unsigned)_shared->client[ 2 ].isSrcReady +
                (volatile unsigned)_shared->client[ 3 ].isSrcReady;       
              } while(num < _numCores - 1);
              // reset shared src buffer ready flags for the next run
              _shared->client[ 1 ].isSrcReady = 0;
              _shared->client[ 2 ].isSrcReady = 0;
              _shared->client[ 3 ].isSrcReady = 0;

              _mcastArgs.setRequestBuffer( (XMI_Request_t *)request);
              _mcastArgs.setSendData( _shared->dst, _bytes );

              _mcastRecvArgs.setRequestBuffer( (XMI_Request_t *)request + 1 );

              if(_numPeers == 1) _mcastRecvArgs.setRecvData( buf, _bytes );
              else  _mcastRecvArgs.setRecvData( _shared->dst, _bytes );

              unsigned destrank;

              if(_mapping->rank() == (unsigned) root)
              {
                //memcpy( _shared->dst, buf, _bytes );
                memcpy( _shared->client[ _rootTcoord ].src, buf, _bytes );
                TRACE_ADAPTOR((stderr,"copied bytes to shmem buf\n "));
              }

              if(_isRootVnPeer)
              {

                if(_mapping->rank() == (unsigned) root)
                {
                  _mcastArgs.setSendData( buf, _bytes );
                }
                else
                {
                  _mcastArgs.setSendData(_shared->client[ _rootTcoord ].src, _bytes);
                }

                destrank = _mapping->rank();
                _mcastArgs.setRanks( &destrank, 1 ); // ???
                _mcast->send( &_mcastArgs );

                _mcastRecvArgs.setOpcode( CCMI_BCAST_RECV_NOSTORE );
                //_mcastRecvArgs.setOpcode( CCMI_BCAST_RECV_STORE );
                _mcast->postRecv( &_mcastRecvArgs );


              }
              else
              {
                destrank = (unsigned)-1;
                _mcastArgs.setRanks( &destrank, 1 ); // ???
                _mcast->send( &_mcastArgs );

                _mcastRecvArgs.setOpcode( CCMI_BCAST_RECV_STORE );
                _mcast->postRecv( &_mcastRecvArgs );

              }

            }

            return XMI_SUCCESS;
          } 

          bool is_vn_peer(unsigned rank, unsigned &root_tcoord){
            unsigned* my_coords = _mapping->Coords();
            unsigned coords[CCMI_TORUS_NDIMS];
            _mapping->Rank2Torus( coords, rank);
            root_tcoord = coords[CCMI_T_DIM];
            return((coords[0] == my_coords[0]) && (coords[1] == my_coords[1]) && (coords[2] ==  my_coords[2]));
          }

        private:

          CCMI::TorusCollectiveMapping                 * _mapping;
          DCMF::Collectives::MultiSend::OldMulticastImpl     * _mcast;
          XMI_Callback_t                _cb_done;
          char                         * _dstbuf;
          XMI_Op                        _op;
          XMI_Dt                        _dt;
          unsigned                       _count;
          unsigned                       _bytes;
          unsigned                       _masterCore;
          unsigned                       _numCores;
          bool             _isRootVnPeer;
          unsigned           _rootTcoord;


          CCMI::MultiSend::CCMI_OldMulticast_t      _mcastArgs __attribute__ ((__aligned__ (16)));
          CCMI::MultiSend::CCMI_OldMulticastRecv_t  _mcastRecvArgs;
          CCMI_Subtask                   _opcodes[1];

          static SharedData            * _shared;
          static unsigned                _minTCoord;
          static unsigned                _numPeers;
        }; /* ShortTreeBcast */
      }
    }
  }
} 





#endif
