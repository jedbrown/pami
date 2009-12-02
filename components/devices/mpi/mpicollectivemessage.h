/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpicollectivemessage.h
 * \brief Experimental MPI collective device on _communicator
 */

#ifndef __components_devices_mpi_mpicollectivemessage_h__
#define __components_devices_mpi_mpicollectivemessage_h__
#ifndef DISABLE_COLLDEVICE

#include "sys/xmi.h"
#include "util/common.h"
#include "util/queue/Queue.h"
#include <mpi.h>
#include "PipeWorkQueue.h"
#include "Topology.h"

namespace XMI {
  namespace Device {
    class MPICollectiveMcastMessage {
    public:
      MPICollectiveMcastMessage(xmi_multicast_t *mcast, size_t dispatch_id, size_t task_id):
      _task_id(task_id),
      _client(mcast->client),
      _context(mcast->context),
      _cb_done(mcast->cb_done),
      _connection_id(mcast->connection_id),
      _roles(mcast->roles),
      _bytes(mcast->bytes),
      _src(mcast->src),
      _src_participants(mcast->src_participants),
      _dst(mcast->dst),
      _dst_participants(mcast->dst_participants),
      _msginfo(mcast->msginfo),
      _msgcount(mcast->msgcount),
      _dispatch_id(dispatch_id),
      _bytesAvailable(0),
      _bytesComplete(0),
      _dataBuffer(NULL) {
        TRACE_ADAPTOR((stderr,":%d:MPICollectiveMcastMessage id %zd, connection id %d, bytes %zd\n",__LINE__,_dispatch_id, _connection_id, _bytes));
      };
      size_t               _task_id;
      xmi_client_t         _client;           /**< client  to operate within */
      size_t               _context;          /**< context to operate within */
      xmi_callback_t       _cb_done;          /**< Completion callback */
      unsigned             _connection_id;    /**< A connection is a distinct stream of
                                                  traffic. The connection id identifies the
                                                  connection */
      unsigned             _roles;            /**< bitmap of roles to perform */
      size_t               _bytes;            /**< size of the message*/
      xmi_pipeworkqueue_t *_src;              /**< source buffer */
      xmi_topology_t      *_src_participants; /**< root */
      xmi_pipeworkqueue_t *_dst;              /**< dest buffer (ignored for one-sided) */
      xmi_topology_t      *_dst_participants; /**< destinations to multicast to*/
      const xmi_quad_t    *_msginfo;          /**< A extra info field to be sent with the message.
                                                  This might include information about
                                                  the data being sent, for one-sided. */
      unsigned             _msgcount;         /**< info count*/

      size_t              _dispatch_id;
      size_t              _bytesAvailable;
      size_t              _bytesComplete;
      char               *_dataBuffer;

      void setId(size_t  dispatch_id) {
        _dispatch_id = dispatch_id;
      }
      const size_t id() {
        return _dispatch_id;
      }
      const size_t root() const {
        return((XMI::Topology*) _src_participants)->index2Rank(0);
      }
      const unsigned msgcount() {
        return _msgcount;
      }
      const unsigned connection_id() {
        return _connection_id;
      }
      const size_t bytes() {
        return _bytes;
      }
      const xmi_quad_t    * msginfo() {
        return _msginfo;
      }
      const bool isRoot() const {
        return(root() == _task_id);
      }
      unsigned available() {
        XMI::PipeWorkQueue* srcPwq =(XMI::PipeWorkQueue*)_src;
        XMI::PipeWorkQueue* dstPwq =(XMI::PipeWorkQueue*)_dst;
        XMI::Topology* dstTopology =(XMI::Topology*)_dst_participants;
        if(isRoot()) {
          _bytesAvailable = srcPwq->bytesAvailableToConsume();
          _bytesComplete = srcPwq->getBytesConsumed();
          if(dstTopology->isRankMember(_task_id)) //src & dst both?
            _bytesAvailable = MIN(_bytesAvailable, dstPwq->bytesAvailableToProduce());
        } else {
          _bytesAvailable = dstPwq->bytesAvailableToProduce();
          _bytesComplete = dstPwq->getBytesProduced();
        }
        TRACE_ADAPTOR((stderr,":%d:MPICollectiveMcastMessage bytesAvailable(%p/%p) %zd, %zd done out of %zd\n",__LINE__, _src,_dst,_bytesAvailable, _bytesComplete, _bytes));
        return _bytesAvailable;
      }
      char* dataBuffer(unsigned dataBytes) {
        TRACE_ADAPTOR((stderr,":%d:MPICollectiveMcastMessage dataBuffer(%p/%p) bytes %zd, %zd available, %zd done out of %zd\n",__LINE__,_src,_dst,dataBytes, _bytesAvailable, _bytesComplete, _bytes));
        XMI::PipeWorkQueue* dstPwq =(XMI::PipeWorkQueue*)_dst;
        if(isRoot()) {
          XMI::PipeWorkQueue* srcPwq =(XMI::PipeWorkQueue*)_src;
          XMI::Topology* dstTopology =(XMI::Topology*)_dst_participants;
          _dataBuffer = srcPwq->bufferToConsume();
          if(dstTopology->isRankMember(_task_id)) { //src & dst both? do a local copy
            memcpy(dstPwq->bufferToProduce(), _dataBuffer, dataBytes);
            dstPwq->produceBytes(dataBytes);
          }
          // seems premature but we are handing back the buffer so it can be consumed, so count the bytes now
          srcPwq->consumeBytes(dataBytes);
          _bytesComplete = srcPwq->getBytesConsumed();
        } else {
          _dataBuffer = dstPwq->bufferToProduce();
          // seems premature but we are handing back the buffer so it can be produced, so count the bytes now
          dstPwq->produceBytes(dataBytes);
          _bytesComplete = dstPwq->getBytesProduced();
        }
        return _dataBuffer;
      }
      char* dataBuffer() {
/*        char* buffer;
        if(isRoot())
        {
          XMI::PipeWorkQueue* srcPwq =(XMI::PipeWorkQueue*)_src;
          buffer = srcPwq->bufferToConsume();
        }
        else
        {
          XMI::PipeWorkQueue* dstPwq =(XMI::PipeWorkQueue*)_dst;
          buffer = dstPwq->bufferToProduce();
        }
 */
        TRACE_ADAPTOR((stderr,":%d:MPICollectiveMcastMessage dataBuffer %p, %zd available, %zd done out of %zd\n",__LINE__, _dataBuffer, _bytesAvailable, _bytesComplete, _bytes));
        return _dataBuffer;
      }
      const bool done() {
        TRACE_ADAPTOR((stderr,":%d:MPICollectiveMcastMessage done %s, %zd available, %zd done out of %zd\n",__LINE__,_bytesComplete == _bytes?"true":"false", _bytesAvailable, _bytesComplete, _bytes));
        return _bytesComplete == _bytes;
      }
#warning fix complete so it passes the correct context
      const void complete() {
        if(_cb_done.function)
          (_cb_done.function)(NULL, //XMI_Client_getcontext(_client,_context), \todo FIX THIS. It core dumps right now in getcontext.
                              _cb_done.clientdata,
                              XMI_SUCCESS);
      }

    };


  };
};

#endif // DISABLE_COLLDEVICE
#endif // __components_devices_mpi_mpicollectivemessage_h__
