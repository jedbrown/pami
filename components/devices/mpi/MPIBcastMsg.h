/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/MPIBcastMsg.h
 * \brief ???
 */

#ifndef __components_devices_mpi_MPIBcastMsg_h__
#define __components_devices_mpi_MPIBcastMsg_h__

#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/util/SubDeviceSuppt.h"
#include "sys/xmi.h"
#include "Global.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include "components/devices/MulticastModel.h"
#undef TRACE_DEVICE
#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {

    class MPIBcastMdl;

    class MPIBcastDev : public XMI::Device::Generic::MultiSendQSubDevice<XMI::Device::Generic::SimpleAdvanceThread,1,1,true>
    {
    public:
      MPI_Comm _mcast_communicator;
      MPIBcastDev() :
      XMI::Device::Generic::MultiSendQSubDevice<XMI::Device::Generic::SimpleAdvanceThread,1,1,true>(),
      _mcast_communicator(MPI_COMM_NULL)
      {
      };

      static inline MPIBcastDev *create(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices);

      /// \brief Initialization for the subdevice
      ///
      /// \param[in] sd		SysDep object (not used?)
      /// \param[in] devices		Array of Generic::Device objects for client
      /// \param[in] contextId	Id of current context (index into devices[])
      /// \ingroup gendev_subdev_api
      ///
      inline void init(XMI::SysDep *sd, size_t clientId, size_t num_ctx, xmi_context_t ctx,  size_t contextId)
      {
        MPI_Comm_dup(MPI_COMM_WORLD,&_mcast_communicator);
	XMI::Device::Generic::MultiSendQSubDevice<XMI::Device::Generic::SimpleAdvanceThread,1,1,true>::init(sd, clientId, num_ctx, ctx, contextId);
      }

    };
  }; //-- Device
}; //-- XMI

static XMI::Device::MPIBcastDev _g_mpibcast_dev;

namespace XMI
{
  namespace Device
  {

inline MPIBcastDev *MPIBcastDev::create(size_t client, size_t num_ctx, XMI::Device::Generic::Device *devices) {
	_g_mpibcast_dev.__create(client, num_ctx, devices);
	return &_g_mpibcast_dev;
}

///
/// \brief
///
    class MPIBcastMsg : public XMI::Device::Generic::GenericMessage
    {
    private:
      enum roles
      {
        NO_ROLE = 0,
        ROOT_ROLE = (1 << 0), // first role must be root
        NON_ROOT_ROLE = (1 << 1), // last role must be non-root(s)
      };
    public:
      MPIBcastMsg(Generic::BaseGenericDevice *Generic_QS,
                  xmi_multicast_t *mcast) :
      XMI::Device::Generic::GenericMessage(Generic_QS, mcast->cb_done,
                                           mcast->client, mcast->context),
      _dst((XMI::Topology *)mcast->dst_participants),
      _iwq((XMI::PipeWorkQueue *)mcast->src),
      _rwq((XMI::PipeWorkQueue *)mcast->dst),
      _bytes(mcast->bytes),
      _tag(mcast->connection_id),
      _idx(0),
      _currBytes(0),
      _currBuf(NULL),
      _req(MPI_REQUEST_NULL),
      _pendingStatus(XMI::Device::Initialized)
      {
        XMI::Topology *src_topo = (XMI::Topology *)mcast->src_participants;
        //XMI_assert(src_topo != NULL);
        if(src_topo && (src_topo->type()!=XMI_EMPTY_TOPOLOGY))
        {
          _root = src_topo->index2Rank(0);

          //I've had some bad topo's, so try to detect it here...
          XMI_assert(src_topo->size() == 1); //technically only one root...
          XMI_assert((0 == src_topo->rank2Index(_root)) && src_topo->isRankMember(_root));
        }
        else // we must be a dst_participant and we don't particularly care who is the root - just not me.
          _root = MPI_ANY_SOURCE;

        TRACE_DEVICE((stderr,"<%p>MPIBcastMsg client %p, context %zd, root %zd, iwq %p, rwq %p, bytes %zd/%zd/%zd\n",this,
                      mcast->client, mcast->context, _root, _iwq, _rwq, _bytes,
                      _iwq?_iwq->bytesAvailableToConsume():-1,
                      _rwq?_rwq->bytesAvailableToProduce():-1));
        bool iamroot = (_root == __global.mapping.task());
        if(iamroot)
        {
          //_rwq = NULL;// \todo Why?  Can't a source (root) also be a destination?

          // no actual data to send, indicate we're done with a pending status (for advance)
          if((_iwq == NULL) || (_bytes == 0))
          {
            TRACE_DEVICE((stderr,"<%p>MPIBcastMsg root has no data\n",this));
            // We have to use a local pending status because the sub device is too smart for us and will
            // reset the _status to initialized after __setThreads
            _pendingStatus = XMI::Device::Done; //setStatus(XMI::Device::Done);
          }
        }
        else // I must be a dst_participant
        {
          //XMI_assert(_dst->isRankMember(__global.mapping.task()));
          // no actual data to send, indicate we're done with a pending status (for advance)
          if((_rwq == NULL) && (_bytes == 0))
          {
            TRACE_DEVICE((stderr,"<%p>MPIBcastMsg dst expects no data\n",this));
            // We have to use a local pending status because the sub device is too smart for us and will
            // reset the _status to initialized after __setThreads
            _pendingStatus = XMI::Device::Done; //setStatus(XMI::Device::Done);
          }
          _iwq = NULL; // ignore the src pwq
        }
        //XMI_assertf(_rwq || _iwq, "MPIBcastMsg has neither input or output data\n");
      }

      // virtual function
      xmi_context_t postNext(bool devPosted) {
	return _g_mpibcast_dev.__postNext<MPIBcastMsg>(this, devPosted);
      }

      inline int setThreads(XMI::Device::Generic::SimpleAdvanceThread **th)
      {
	XMI::Device::Generic::SimpleAdvanceThread *t;
	int n;
	_g_mpibcast_dev.__getThreads(&t, &n);
        int nt = 0;
        _nThreads = 1;  // must predict total number of threads now,
        // so early advance(s) work
        t[nt].setMsg(this);
        t[nt].setAdv(advanceThread);
        t[nt].setStatus(XMI::Device::Ready);
        t[nt]._bytesLeft = _bytes;
        __advanceThread(&t[nt]);
        ++nt;
        // assert(nt > 0? && nt < n);
        TRACE_DEVICE((stderr,"<%p>MPIBcastMsg::__setThreads(%d) _nThreads %d, bytes left %zd\n",this,
                      n,nt,t[nt]._bytesLeft));
	*th = t;
        return nt;
      }

    protected:
      DECL_ADVANCE_ROUTINE(advanceThread,MPIBcastMsg,XMI::Device::Generic::SimpleAdvanceThread);
      inline xmi_result_t __advanceThread(XMI::Device::Generic::SimpleAdvanceThread *thr)
      {
        if(getStatus() == XMI::Device::Done)
        {
          fprintf(stderr, "Warning: message/thread advanced after Done\n");
          return XMI_SUCCESS;
        }
        if(_pendingStatus == XMI::Device::Done)
        {
          // This happens when there is no data to send/receive and ctor set a "pending status" to done,
          //  so on the first advance, setDone and return.
          thr->setStatus(XMI::Device::Complete);
          setStatus(XMI::Device::Done);
          TRACE_DEVICE((stderr,"<%p>MPIBcastMsg::__advanceThread() done - no data\n",this));
          return XMI_SUCCESS;
        }
        int flag = 0;
        MPI_Status status;
        //static unsigned count = 5; if(count) count--;
        //if(count)TRACE_DEVICE((stderr,"<%p>MPIBcastMsg::__advanceThread() idx %zd/%zd, currBytes %zd, bytesLeft %zd, tag %d %s\n",this,
        //              _idx, _dst->size(), _currBytes, thr->_bytesLeft, _tag,_req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
        if(_req != MPI_REQUEST_NULL)
        {
          MPI_Test(&_req, &flag, &status);
          if(flag)
          {
            _req = MPI_REQUEST_NULL; // redundant?
          }
          else
          {
            return XMI_EAGAIN;
          }
          // current message was completed...
        }
        if(_iwq) // root - keep send until all ranks have all bytes...
        {
          if(_idx >= _dst->size())
          {
            thr->_bytesLeft -= _currBytes;
            _iwq->consumeBytes(_currBytes);
            if(thr->_bytesLeft == 0)
            {
              thr->setStatus(XMI::Device::Complete);
              setStatus(XMI::Device::Done);
              return XMI_SUCCESS;
            }
            _currBytes = 0;
          }
          if(_currBytes == 0)
          {
            _currBytes = _iwq->bytesAvailableToConsume();
            _currBuf = _iwq->bufferToConsume();
            _idx = 0;
          }
          if(_currBytes == 0)
          {
            return XMI_EAGAIN;
          }
          TRACE_DEVICE((stderr,"<%p>MPIBcastMsg::__advanceThread() sending, idx %zd, currBytes %zd, bytesLeft %zd, dst %zd, tag %d %s\n",this,
                        _idx, _currBytes, thr->_bytesLeft, _dst->index2Rank(_idx), _tag,_req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
          int rc = 0;
          if(_dst->index2Rank(_idx) == __global.mapping.task()) // This src task is also a dst? do a local copy
          {
            memcpy(_rwq->bufferToProduce(), _currBuf, _currBytes);
            _rwq->produceBytes(_currBytes);
          }
          else
          {
            rc = MPI_Isend(_currBuf, _currBytes, MPI_BYTE,
                           _dst->index2Rank(_idx), _tag,
                           _g_mpibcast_dev._mcast_communicator, &_req);
          }
          TRACE_DEVICE((stderr,"<%p>MPIBcastMsg::__advanceThread() sending rc = %d, idx %zd, currBytes %zd, bytesLeft %zd, dst %zd, tag %d %s\n",this,
                        rc,_idx, _currBytes, thr->_bytesLeft, _dst->index2Rank(_idx), _tag,_req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
          //count = 5;
          // error checking?
          ++_idx;
        }
        else // receive data until done...
        {
          XMI_assert(_rwq);
          if(flag)
          {
            int scount;
            MPI_Get_count(&status, MPI_BYTE, &scount);
            if((size_t)scount < _currBytes)
            {
              // make sure we only count what was recv'ed
              _currBytes = scount;
            }
            else
            {
              // how does MPI_Status.count work?
              XMI_assertf((size_t)scount <= _currBytes,
                          "MPIBcastMsg recv overrun (got %d, kept %zd)\n",
                          scount, _currBytes);
            }
          }

          // if we are here, we completed a chunk of recv data...
          thr->_bytesLeft -= _currBytes;
          _rwq->produceBytes(_currBytes);
          if(thr->_bytesLeft == 0)
          {
            thr->setStatus(XMI::Device::Complete);
            setStatus(XMI::Device::Done);
            return XMI_SUCCESS;
          }
          _currBytes = _rwq->bytesAvailableToProduce();
          _currBuf = _rwq->bufferToProduce();
          TRACE_DEVICE((stderr,"<%p>MPIBcastMsg::__advanceThread() recving idx %zd, currBytes %zd, bytesLeft %zd, src %zd, tag %d %s\n",this,
                        _idx, _currBytes, thr->_bytesLeft, _root, _tag,_req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
          if(_currBytes == 0)
          {
            return XMI_EAGAIN;
          }
          //count = 5;
          int rc = MPI_Irecv(_currBuf, _currBytes, MPI_BYTE,
                             _root, _tag,
                             _g_mpibcast_dev._mcast_communicator, &_req);
          TRACE_DEVICE((stderr,"<%p>MPIBcastMsg::__advanceThread() recving rc = %d, idx %zd, currBytes %zd, bytesLeft %zd, src %zd, tag %d %s\n",this,
                        rc, _idx, _currBytes, thr->_bytesLeft, _root, _tag, _req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
          // error checking?
        }

        return XMI_EAGAIN;
      }

      unsigned _nThreads;
      XMI::Topology *_dst;
      XMI::PipeWorkQueue *_iwq;
      XMI::PipeWorkQueue *_rwq;
      size_t _root;
      size_t _bytes;
      int _tag;
      size_t _idx;
      size_t _currBytes;
      char *_currBuf;
      MPI_Request _req;
      MessageStatus _pendingStatus;
    }; //-- MPIBcastMsg

    class MPIBcastMdl : public XMI::Device::Interface::MulticastModel<MPIBcastMdl,sizeof(MPIBcastMsg)>
    {
    public:
      static const int NUM_ROLES = 2;
      static const int REPL_ROLE = 1;
      static const size_t sizeof_msg = sizeof(MPIBcastMsg);

      MPIBcastMdl(xmi_result_t &status) :
        XMI::Device::Interface::MulticastModel<MPIBcastMdl, sizeof(MPIBcastMsg)>(status)
      {
        TRACE_DEVICE((stderr,"<%p>MPIBcastMdl()\n",this));
      }

      inline xmi_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                             xmi_multicast_t *mcast);

    private:
    }; // class MPIBcastMdl

    inline xmi_result_t MPIBcastMdl::postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                                        xmi_multicast_t *mcast)
    {
      TRACE_DEVICE((stderr,"<%p>MPIBcastMdl::postMulticast() dispatch %zd, connection_id %d, msgcount %d, bytes %zd, request %p\n",this,
                    mcast->dispatch, mcast->connection_id, mcast->msgcount, mcast->bytes, &state));
      MPIBcastMsg *msg =
      new (&state) MPIBcastMsg(_g_mpibcast_dev.getQS(), mcast);
      _g_mpibcast_dev.__post<MPIBcastMsg>(msg);
      return XMI_SUCCESS;
    }

  }; //-- Device
}; //-- XMI
#undef TRACE_DEVICE
#endif // __components_devices_workqueue_MPIBcastMsg_h__
