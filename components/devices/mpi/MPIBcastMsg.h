/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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

#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/SubDevice.h"
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

    template< class T_Thread>
    class MPIBcastDev : public XMI::Device::Generic::SimpleSubDevice<T_Thread>
    {
    public:
      MPI_Comm _mcast_communicator;
      MPIBcastDev() :
      XMI::Device::Generic::SimpleSubDevice<T_Thread>(),
      _mcast_communicator(MPI_COMM_NULL)
      {
      };
      /// \brief Initialization for the subdevice
      ///
      /// \param[in] sd		SysDep object (not used?)
      /// \param[in] devices		Array of Generic::Device objects for client
      /// \param[in] contextId	Id of current context (index into devices[])
      /// \ingroup gendev_subdev_api
      ///
      inline void init(XMI::SysDep &sd, XMI::Device::Generic::Device *((*devices)[XMI_MAX_NUM_CLIENTS]), size_t clientId, size_t contextId)
      {
        MPI_Comm_dup(MPI_COMM_WORLD,&_mcast_communicator);
        Generic::SimpleSubDevice<T_Thread>::init(sd, devices, clientId, contextId);
      }

    };
  }; //-- Device
}; //-- XMI

extern XMI::Device::MPIBcastDev<XMI::Device::Generic::SimpleAdvanceThread> _g_mpibcast_dev;

namespace XMI
{
  namespace Device
  {

///
/// \brief
///
    template < class T_Device >
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
      MPIBcastMsg(T_Device &Generic_QS, //Generic::BaseGenericDevice &Generic_QS,
                  xmi_multicast_t *mcast) :
      XMI::Device::Generic::GenericMessage(Generic_QS, mcast->cb_done,
                                           XMI_GD_ClientId(mcast->client), mcast->context),
      _device(&Generic_QS),
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
        XMI_assert(src_topo != NULL);
        _root = src_topo->index2Rank(0);

        //I've had some bad topo's, so try to detect it here...
        XMI_assert(src_topo->size() == 1); //techinically only one root...
        XMI_assert((0 == src_topo->rank2Index(_root)) && src_topo->isRankMember(_root));

        TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg client %p, context %zd, root %zd, iwq %p, rwq %p, bytes %zd\n",(unsigned)this,
                      mcast->client, mcast->context, _root, _iwq, _rwq, _bytes));
        bool iamroot = (_root == __global.mapping.task());
        if(iamroot)
        {
          //_rwq = NULL;// \todo Why?  Can't a source (root) also be a destination?

          // no actual data to send, indicate we're done with a pending status (for advance)
          if((_iwq == NULL) || (_bytes == 0))
          {
            TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg root has no data\n",(unsigned)this));
            // We have to use a local pending status because the sub device is too smart for us and will
            // reset the _status to initialized after __setThreads
            _pendingStatus = XMI::Device::Done; //setStatus(XMI::Device::Done);
          }
        }
        else // I must be a dst_participant
        {
          XMI_assert(_dst->isRankMember(__global.mapping.task()));
          // no actual data to send, indicate we're done with a pending status (for advance)
          if((_rwq == NULL) && (_bytes == 0))
          {
            TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg dst expects no data\n",(unsigned)this));
            // We have to use a local pending status because the sub device is too smart for us and will
            // reset the _status to initialized after __setThreads
            _pendingStatus = XMI::Device::Done; //setStatus(XMI::Device::Done);
          }
          _iwq = NULL; // ignore the src pwq
        }
        //XMI_assertf(_rwq || _iwq, "MPIBcastMsg has neither input or output data\n");
      }

      STD_POSTNEXT(T_Device,XMI::Device::Generic::SimpleAdvanceThread,_device)
    protected:
      //	STD_POSTNEXT(MPIBcastDev,XMI::Device::Generic::SimpleAdvanceThread,&_g_mpibcast_dev)
/*      // This is a virtual function, but declaring inline here avoids linker
      // complaints about multiple definitions.
      inline void complete(xmi_context_t context);
 Compile with xl but not gcc.
      {
        ((T_Device &)_QS).__complete<MPIBcastMsg>(this);
        TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg::complete() \n",(unsigned)this));
        executeCallback(context);
      }
*/
      //friend class MPIBcastDev; // Until C++ catches up with real programming languages:
      friend class XMI::Device::Generic::SimpleSubDevice<XMI::Device::Generic::SimpleAdvanceThread>;

      ADVANCE_ROUTINE(advanceThread,MPIBcastMsg<T_Device>,XMI::Device::Generic::SimpleAdvanceThread);
      inline int __setThreads(XMI::Device::Generic::SimpleAdvanceThread *t, int n)
      {
        int nt = 0;
        // assert(nt < n);
        _nThreads = 1;  // must predict total number of threads now,
        // so early advance(s) work
        t[nt].setMsg(this);
        t[nt].setAdv(advanceThread);
        t[nt].setStatus(XMI::Device::Ready);
        t[nt]._bytesLeft = _bytes;
        __advanceThread(&t[nt]);
        ++nt;
        // assert(nt > 0? && nt < n);
        TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg::__setThreads(%d) _nThreads %d, bytes left %zd\n",(unsigned)this,
                      n,nt,t[nt]._bytesLeft));
        return nt;
      }

      friend class XMI::Device::Generic::GenericMessage;
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
          TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg::__advanceThread() done - no data\n",(unsigned)this));
          return XMI_SUCCESS;
        }
        int flag = 0;
        MPI_Status status;
        //static unsigned count = 5; if(count) count--;
        //if(count)TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg::__advanceThread() idx %zd/%zd, currBytes %zd, bytesLeft %zd, tag %d %s\n",(unsigned)this,
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
          TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg::__advanceThread() sending, idx %zd, currBytes %zd, bytesLeft %zd, dst %zd, tag %d %s\n",(unsigned)this,
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
                           _device->_mcast_communicator, &_req);
          }
          TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg::__advanceThread() sending rc = %d, idx %zd, currBytes %zd, bytesLeft %zd, dst %zd, tag %d %s\n",(unsigned)this,
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
                          "MPIBcastMsg recv overrun (got %d, kept %ld)\n",
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
          TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg::__advanceThread() recving idx %zd, currBytes %zd, bytesLeft %zd, src %zd, tag %d %s\n",(unsigned)this,
                        _idx, _currBytes, thr->_bytesLeft, _root, _tag,_req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
          if(_currBytes == 0)
          {
            return XMI_EAGAIN;
          }
          //count = 5;
          int rc = MPI_Irecv(_currBuf, _currBytes, MPI_BYTE,
                             _root, _tag,
                             _device->_mcast_communicator, &_req);
          TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMsg::__advanceThread() recving rc = %d, idx %zd, currBytes %zd, bytesLeft %zd, src %zd, tag %d %s\n",(unsigned)this,
                        rc, _idx, _currBytes, thr->_bytesLeft, _root, _tag, _req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
          // error checking?
        }

        return XMI_EAGAIN;
      }

      unsigned _nThreads;
      T_Device *_device;
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


    class MPIBcastMdl : public XMI::Device::Interface::MulticastModel<MPIBcastMdl>
    {
    public:
      static const int NUM_ROLES = 2;
      static const int REPL_ROLE = 1;
      static const size_t sizeof_msg = sizeof(MPIBcastMsg<XMI::Device::MPIBcastDev<XMI::Device::Generic::SimpleAdvanceThread> >);

      MPIBcastMdl(xmi_result_t &status) :
      XMI::Device::Interface::MulticastModel<MPIBcastMdl>(status)
      {
        TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMdl()\n",(unsigned)this));
      }

      inline bool postMulticast_impl(xmi_multicast_t *mcast);

    private:
    }; // class MPIBcastMdl

    inline bool MPIBcastMdl::postMulticast_impl(xmi_multicast_t *mcast)
    {
      TRACE_DEVICE((stderr,"<%#8.8X>MPIBcastMdl::postMulticast() dispatch %zd, connection_id %d, msgcount %d, bytes %zd, request %p\n",(unsigned)this,
                    mcast->dispatch, mcast->connection_id, mcast->msgcount, mcast->bytes, mcast->request));
      MPIBcastMsg<XMI::Device::MPIBcastDev<XMI::Device::Generic::SimpleAdvanceThread> > *msg =
      new (mcast->request) MPIBcastMsg<XMI::Device::MPIBcastDev<XMI::Device::Generic::SimpleAdvanceThread> >(_g_mpibcast_dev, mcast);
      _g_mpibcast_dev.__post<MPIBcastMsg<XMI::Device::MPIBcastDev<XMI::Device::Generic::SimpleAdvanceThread> > >(msg);
      return true;
    }

  }; //-- Device
}; //-- XMI
#undef TRACE_DEVICE
#endif // __components_devices_workqueue_MPIBcastMsg_h__
