/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/MPISyncMsg.h
 * \brief ???
 */

#ifndef __components_devices_mpi_MPISyncMsg_h__
#define __components_devices_mpi_MPISyncMsg_h__

#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/SubDeviceSuppt.h"
#include "sys/xmi.h"
#include "Global.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include "components/devices/MultisyncModel.h"
#undef TRACE_DEVICE
#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {

    class MPISyncMdl;

    template< class T_Thread>
    class MPISyncDev : public XMI::Device::Generic::SimpleSubDevice<T_Thread>
    {
    public:
      MPI_Comm _msync_communicator;
      MPISyncDev() :
      XMI::Device::Generic::SimpleSubDevice<T_Thread>(),
      _msync_communicator(MPI_COMM_NULL)
      {
      };
      /// \brief Initialization for the subdevice
      ///
      /// \param[in] sd		SysDep object (not used?)
      /// \param[in] devices		Array of Generic::Device objects for client
      /// \param[in] contextId	Id of current context (index into devices[])
      /// \ingroup gendev_subdev_api
      ///
      inline void init(XMI::SysDep &sd, XMI::Device::Generic::Device *devices, size_t clientId, size_t contextId)
      {
        MPI_Comm_dup(MPI_COMM_WORLD,&_msync_communicator);
        Generic::SimpleSubDevice<T_Thread>::init(sd, devices, clientId, contextId);
      }
    };
  }; //-- Device
}; //-- XMI

extern XMI::Device::MPISyncDev<XMI::Device::Generic::SimpleAdvanceThread> _g_mpisync_dev;

namespace XMI
{
  namespace Device
  {

///
/// \brief
///
    template < class T_Device >
    class MPISyncMsg : public XMI::Device::Generic::GenericMessage
    {
    private:
      enum roles
      {
        NO_ROLE = 0,
        ROOT_ROLE = (1 << 0), // first role must be root
        NON_ROOT_ROLE = (1 << 1), // last role must be non-root(s)
      };
    public:
      MPISyncMsg(T_Device &Generic_QS,
                 xmi_multisync_t *msync) :
      XMI::Device::Generic::GenericMessage(Generic_QS, msync->cb_done,
                                           XMI_GD_ClientId(msync->client), msync->context),
      _device(&Generic_QS),
      _participants((XMI::Topology *)msync->participants),
      _tag(msync->connection_id),
      _idx(0),
      _req(MPI_REQUEST_NULL),
      _pendingStatus(XMI::Device::Initialized),
      _root(_participants->index2Rank(0))
      {
        TRACE_DEVICE((stderr,"<%p>MPISyncMsg client %p, context %zd\n",this,
                      msync->client, msync->context));
        if(_participants->size() == 1)
        {
          // We have to use a local pending status because the sub device is too smart for us and will
          // reset the _status to initialized after __setThreads
          _pendingStatus = XMI::Device::Done; //setStatus(XMI::Device::Done);
          return;
        }
        if(_root == __global.mapping.task())
        {
          _idx++;
        }
        else
        {
          int rc = MPI_Send(NULL, 0, MPI_BYTE,
                            _root,_tag,_g_mpisync_dev._msync_communicator);
          TRACE_DEVICE((stderr,"<%p>MPISyncMsg::ctor send rc = %d, dst %zd, tag %d \n",this,
                        rc, _root, _tag));
        }
        int rc = MPI_Irecv(NULL,0, MPI_BYTE,
                           _participants->index2Rank(_idx),_tag,
                           _g_mpisync_dev._msync_communicator, &_req);
        TRACE_DEVICE((stderr,"<%p>MPISyncMsg::ctor irecv rc = %d, dst %zd, tag %d \n",this,
                      rc, _participants->index2Rank(_idx), _tag));
      }

      STD_POSTNEXT(T_Device,XMI::Device::Generic::SimpleAdvanceThread,_device)
    protected:
      friend class MPISyncDev<XMI::Device::Generic::SimpleAdvanceThread>;
      friend class XMI::Device::Generic::SimpleSubDevice<XMI::Device::Generic::SimpleAdvanceThread>; // this makes no sense

      ADVANCE_ROUTINE(advanceThread,MPISyncMsg<T_Device>,XMI::Device::Generic::SimpleAdvanceThread);
      inline int __setThreads(XMI::Device::Generic::SimpleAdvanceThread *t, int n)
      {
        int nt = 0;
        // assert(nt < n);
        t[nt].setMsg(this);
        t[nt].setAdv(advanceThread);
        t[nt].setStatus(XMI::Device::Ready);
        ++nt;
        // assert(nt > 0? && nt < n);
        _nThreads = nt;
        TRACE_DEVICE((stderr,"<%p>MPISyncMsg::__setThreads(%d) _nThreads %d\n",this,
                      n,nt));
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
          TRACE_DEVICE((stderr,"<%p>MPISyncMsg::__advanceThread() done - no participants\n",this));
          return XMI_SUCCESS;
        }
        int flag = 0;
        MPI_Status status;
        //static unsigned count = 5; if(count) count--;
        //if(count)TRACE_DEVICE((stderr,"<%p>MPISyncMsg::__advanceThread() idx %zd/%zd, currBytes %zd, bytesLeft %zd, tag %d %s\n",this,
        //              _idx, _dst->size(), _currBytes, thr->_bytesLeft, _tag,_req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
        if(_req != MPI_REQUEST_NULL)
        {
          MPI_Test(&_req, &flag, &status);
          if(flag)
          {
            TRACE_DEVICE((stderr,"<%p>MPISyncMsg::__advanceThread test done, dst %zd, tag %d \n",this,
                          _participants->index2Rank(_idx), _tag));
            _req = MPI_REQUEST_NULL; // redundant?
          }
          else
          {
            return XMI_EAGAIN;
          }
          // current message was completed...
        }
        if(_root == __global.mapping.task())
        {
          if(++_idx < _participants->size())
          {
            int rc = MPI_Irecv(NULL,0, MPI_BYTE,
                               _participants->index2Rank(_idx),_tag,
                               _g_mpisync_dev._msync_communicator, &_req);
            TRACE_DEVICE((stderr,"<%p>MPISyncMsg::__advanceThread irecv rc = %d, dst %zd, tag %d \n",this,
                          rc, _participants->index2Rank(_idx), _tag));
            return XMI_EAGAIN;
          }
          else
          {
            for(size_t idx=1; idx < _participants->size(); ++idx)
            {
              int rc = MPI_Send(NULL, 0, MPI_BYTE,
                                _participants->index2Rank(idx),_tag,
                                _g_mpisync_dev._msync_communicator);
              TRACE_DEVICE((stderr,"<%p>MPISyncMsg::__advanceThread() sending rc = %d, idx %zd, dst %zd, tag %d\n",this,
                            rc,idx, _participants->index2Rank(idx), _tag));
            }
          }
        }
        thr->setStatus(XMI::Device::Complete);
        setStatus(XMI::Device::Done);
        return XMI_SUCCESS;
      }

      unsigned _nThreads;
      T_Device *_device;
      XMI::Topology *_participants;
      int            _tag;
      size_t         _idx;
      MPI_Request    _req;
      MessageStatus  _pendingStatus;
      size_t         _root; // first rank in the sync - arbitrary 'root'
    }; //-- MPISyncMsg

    typedef MPISyncMsg<XMI::Device::MPISyncDev<XMI::Device::Generic::SimpleAdvanceThread> > MPISyncMsg_t;

    class MPISyncMdl : public XMI::Device::Interface::MultisyncModel<MPISyncMdl,sizeof(MPISyncMsg_t)>
    {
    public:
      static const int NUM_ROLES = 2;
      static const int REPL_ROLE = 1;
      static const size_t sizeof_msg = sizeof(MPISyncMsg_t);

      MPISyncMdl(xmi_result_t &status) :
      XMI::Device::Interface::MultisyncModel<MPISyncMdl,sizeof(MPISyncMsg_t)>(status)
      {
        TRACE_DEVICE((stderr,"<%p>MPISyncMdl()\n",this));
        //XMI::SysDep *sd = _g_mpisync_dev.getSysdep();
      }

      inline xmi_result_t postMultisync_impl(uint8_t         (&state)[sizeof_msg],
                                             xmi_multisync_t *msync);

    private:
    }; // class MPISyncMdl

    inline xmi_result_t MPISyncMdl::postMultisync_impl(uint8_t         (&state)[sizeof_msg],
                                                       xmi_multisync_t *msync)
    {
      TRACE_DEVICE((stderr,"<%p>MPISyncMdl::postMulticast() connection_id %d, request %p\n",this,
                    msync->connection_id, &state));
      MPISyncMsg<XMI::Device::MPISyncDev<XMI::Device::Generic::SimpleAdvanceThread> > *msg =
      new (&state) MPISyncMsg<XMI::Device::MPISyncDev<XMI::Device::Generic::SimpleAdvanceThread> >(_g_mpisync_dev, msync);
      _g_mpisync_dev.__post<MPISyncMsg<XMI::Device::MPISyncDev<XMI::Device::Generic::SimpleAdvanceThread> > >(msg);
      return XMI_SUCCESS;
    }

  }; //-- Device
}; //-- XMI
#undef TRACE_DEVICE
#endif // __components_devices_workqueue_mpisyncmsg_h__
