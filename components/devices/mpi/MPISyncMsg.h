/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/MPISyncMsg.h
 * \brief ???
 */

#ifndef __components_devices_mpi_mpisyncmsg_h__
#define __components_devices_mpi_mpisyncmsg_h__

#include "components/devices/generic/Device.h"
#include "components/devices/generic/Message.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/SubDevice.h"
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
    class MPISyncMsg;
    typedef XMI::Device::Generic::SimpleAdvanceThread MPISyncThr; //GenericAdvanceThread
    template< class T_Thread>
    class MPISyncDev : public XMI::Device::Generic::SimpleSubDevice<T_Thread>
    {
    public:
      MPI_Comm _msync_communicator; 
      MPISyncDev() :
      XMI::Device::Generic::SimpleSubDevice<T_Thread>() 
      {
        MPI_Comm_dup(MPI_COMM_WORLD,&_msync_communicator);
      };
    };
  }; //-- Device
}; //-- XMI

extern XMI::Device::MPISyncDev<XMI::Device::MPISyncThr> _g_mpisync_dev;

namespace XMI
{
  namespace Device
  {

///
/// \brief 
///
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
      MPISyncMsg(Generic::BaseGenericDevice &Generic_QS,
                 xmi_multisync_t *msync) :
      XMI::Device::Generic::GenericMessage(Generic_QS, msync->cb_done,
                                           msync->client, msync->context),
      _participants((XMI::Topology *)msync->participants),
      _tag(msync->connection_id),
      _idx(0),
      _req(MPI_REQUEST_NULL),
      _pendingStatus(XMI::Device::Initialized),
      _root(_participants->index2Rank(0))
      {
        TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg client %p, context %zd\n",(unsigned)this,
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
          TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::ctor send rc = %d, dst %zd, tag %d \n",(unsigned)this,
                        rc, _root, _tag));
        }
        int rc = MPI_Irecv(NULL,0, MPI_BYTE,
                           _participants->index2Rank(_idx),_tag,
                           _g_mpisync_dev._msync_communicator, &_req);
        TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::ctor irecv rc = %d, dst %zd, tag %d \n",(unsigned)this,
                      rc, _participants->index2Rank(_idx), _tag));
      }

      // This is a virtual function, but declaring inline here avoids linker
      // complaints about multiple definitions.
      inline void complete(xmi_context_t context);

    protected:
      friend class MPISyncDev<XMI::Device::MPISyncThr>; 
      friend class XMI::Device::Generic::SimpleSubDevice<MPISyncThr>; // this makes no sense

      ADVANCE_ROUTINE(advanceThread,MPISyncMsg,MPISyncThr);
      inline int __setThreads(MPISyncThr *t, int n)
      {
        int nt = 0;
        // assert(nt < n);
        t[nt].setMsg(this);
        t[nt].setAdv(advanceThread);
        t[nt].setDone(false);
        ++nt;
        // assert(nt > 0? && nt < n);
        _nThreads = nt;
        TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::__setThreads(%d) _nThreads %d\n",(unsigned)this,
                      n,nt));
        return nt;
      }

      friend class XMI::Device::Generic::GenericMessage;
      inline xmi_result_t __advanceThread(MPISyncThr *thr)
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
          thr->setDone(true); 
          setStatus(XMI::Device::Done);
          TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::__advanceThread() done - no participants\n",(unsigned)this));
          return XMI_SUCCESS;
        }
        int flag = 0;
        MPI_Status status;
        //static unsigned count = 5; if(count) count--;
        //if(count)TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::__advanceThread() idx %zd/%zd, currBytes %zd, bytesLeft %zd, tag %d %s\n",(unsigned)this,
        //              _idx, _dst->size(), _currBytes, thr->_bytesLeft, _tag,_req == MPI_REQUEST_NULL?"MPI_REQUEST_NULL":""));
        if(_req != MPI_REQUEST_NULL)
        {
          MPI_Test(&_req, &flag, &status);
          if(flag)
          {
            TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::__advanceThread test done, dst %zd, tag %d \n",(unsigned)this,
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
            TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::__advanceThread irecv rc = %d, dst %zd, tag %d \n",(unsigned)this,
                          rc, _participants->index2Rank(_idx), _tag));
            return XMI_EAGAIN;
          }
          else
          {
            for(int idx=1; idx < _participants->size(); ++idx)
            {
              int rc = MPI_Send(NULL, 0, MPI_BYTE,
                                _participants->index2Rank(idx),_tag,
                                _g_mpisync_dev._msync_communicator);
              TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::__advanceThread() sending rc = %d, idx %zd, dst %zd, tag %d\n",(unsigned)this,
                            rc,idx, _participants->index2Rank(idx), _tag));
            }
          }
        }
        thr->setDone(true);
        setStatus(XMI::Device::Done);
        return XMI_SUCCESS;
      }

      unsigned _nThreads;
      XMI::Topology *_participants;
      int            _tag;
      size_t         _idx;
      MPI_Request    _req;
      MessageStatus  _pendingStatus;
      size_t         _root; // first rank in the sync - arbitrary 'root'
    }; //-- MPISyncMsg

    class MPISyncMdl : public XMI::Device::Interface::MultisyncModel<MPISyncMdl>
    {
    public:
      static const int NUM_ROLES = 2;
      static const int REPL_ROLE = 1;
      static const size_t sizeof_msg = sizeof(MPISyncMsg);

      MPISyncMdl(xmi_result_t &status) :
      XMI::Device::Interface::MultisyncModel<MPISyncMdl>(status)
      {
        TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMdl()\n",(unsigned)this));
        //XMI::SysDep *sd = _g_mpisync_dev.getSysdep();
      }

      inline bool postMultisync_impl(xmi_multisync_t *msync);

    private:
    }; // class MPISyncMdl

    void MPISyncMsg::complete(xmi_context_t context)
    {
      ((MPISyncDev<XMI::Device::MPISyncThr> &)_QS).__complete<MPISyncMsg>(this);
      TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMsg::complete() \n",(unsigned)this));
      executeCallback(context);
    }

    inline bool MPISyncMdl::postMultisync_impl(xmi_multisync_t *msync)
    {
      TRACE_DEVICE((stderr,"<%#8.8X>MPISyncMdl::postMulticast() connection_id %d, request %p\n",(unsigned)this,
                    msync->connection_id, msync->request));
      MPISyncMsg *msg = new (msync->request) MPISyncMsg(_g_mpisync_dev, msync);
      _g_mpisync_dev.__post<MPISyncMsg>(msg);
      return true;
    }

  }; //-- Device
}; //-- XMI
#undef TRACE_DEVICE
#endif // __components_devices_workqueue_mpisyncmsg_h__
