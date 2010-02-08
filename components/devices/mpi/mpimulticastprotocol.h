/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/mpi/mpimulticastprotocol.h
/// \brief Defines protocol classes for multicast
///
#ifndef __components_devices_mpi_mpimulticastprotocol_h__
#define __components_devices_mpi_mpimulticastprotocol_h__

#include "sys/xmi.h"
#include "SysDep.h"
#include "components/memory/MemoryAllocator.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include "Global.h"
#include "components/devices/MulticastModel.h"
//#include <list>
//#include "components/devices/mpi/mpicollectiveheader.h"

#undef TRACE_DEVICE
#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace XMI
{
  namespace Protocol
  {
    namespace MPI
    {

      ///
      /// \brief 1-sided multicast protocol built on a p2p dispatch and all-sided multicast
      ///
      template <
        class T_P2P_DEVICE,
        class T_P2P_PROTOCOL,
        class T_MULTICAST_MODEL>
      class P2PMcastProto:
        public XMI::Device::Interface::AMMulticastModel<P2PMcastProto<T_P2P_DEVICE,T_P2P_PROTOCOL, T_MULTICAST_MODEL>, T_MULTICAST_MODEL::sizeof_msg+sizeof(XMI::Topology)+sizeof(xmi_callback_t)+sizeof(void*) >
      {
        ///
        /// Point-to-point dispatch header.
        ///
        typedef struct _dispatch_header_
        {
          xmi_task_t root;        /// multicast root
          unsigned connection_id; /// multicast connection id
          size_t   bytes;         /// total bytes being multicast
        } dispatch_hdr_t;

        ///
        /// Allocation for multicast. Passed on cb_done client data so it can be processed and freed
        ///
        typedef struct _allocation_
        {
          uint8_t                  request[T_MULTICAST_MODEL::sizeof_msg]; /// request storage for the message
          XMI::Topology            topology; ///  storage for src_participants
          xmi_callback_t           cb_done;  ///  original user's cb_done
          P2PMcastProto<T_P2P_DEVICE,T_P2P_PROTOCOL, T_MULTICAST_MODEL>
          *protocol;/// this protocol object - to retrieve allocator
        } allocation_t;
      public:
        static const int NUM_ROLES = 2;
        static const int REPL_ROLE = 1;
        static const size_t sizeof_msg = sizeof(allocation_t);

        ///
        /// \brief Base class constructor
        ///
        inline P2PMcastProto(xmi_result_t             & status) :
          XMI::Device::Interface::AMMulticastModel<P2PMcastProto<T_P2P_DEVICE,T_P2P_PROTOCOL, T_MULTICAST_MODEL>,sizeof(allocation_t) >(status),
          _dst_participants(__global.mapping.task()), // default dst is this task when dispatched from another src
          _dispatch_id(0),
          _dispatch_fn(NULL),
//          _task_id(origin_task),
          _client(NULL),
          _context(NULL),
          _contextid(0),
          _task_id((size_t)__global.mapping.task()),
          _cookie(NULL),
          _multicast_model(status)
          {
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto(status)  allocator size %zd\n",this,_allocator.objsize));
          }
        inline P2PMcastProto(size_t                     dispatch_id,
                             xmi_dispatch_multicast_fn  dispatch,
                             void                     * cookie,
                             T_P2P_DEVICE             & p2p_device,
                             //size_t                     origin_task,
                             xmi_client_t               client,
                             xmi_context_t              context,
                             size_t                     contextid,
                             xmi_result_t             & status) :
          XMI::Device::Interface::AMMulticastModel<P2PMcastProto<T_P2P_DEVICE,T_P2P_PROTOCOL, T_MULTICAST_MODEL>,sizeof(allocation_t) >(status),
          _dst_participants(__global.mapping.task()), // default dst is this task when dispatched from another src
          _dispatch_id(dispatch_id),
          _dispatch_fn(dispatch),
//          _task_id(origin_task),
          _client(client),
          _context(context),
          _contextid(contextid),
          _task_id((size_t)__global.mapping.task()),
          _cookie(cookie),
          _multicast_model(status)
          {
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto.  allocator size %zd\n",this,_allocator.objsize));
            // Construct a p2p protocol for dispatching
            xmi_dispatch_callback_fn fn;
            fn.p2p = dispatch_p2p;
            new (&_p2p_protocol) T_P2P_PROTOCOL(dispatch_id, fn, (void*)this,
                                                p2p_device, status);
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto status %d\n",this,status));
          }

        xmi_result_t registerMcastRecvFunction_impl(int dispatch_id,
                                                    xmi_dispatch_multicast_fn     dispatch,
                                                    void                         *cookie)
          {
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto::register id %zd, fn %p, cookie %p\n",this,dispatch_id, dispatch, cookie));
            xmi_result_t status = XMI_SUCCESS;
            _dispatch_id=dispatch_id;
            _dispatch_fn=dispatch;
            _cookie=cookie;
            // Construct a p2p protocol for dispatching
            xmi_dispatch_callback_fn fn;
            fn.p2p = dispatch_p2p;
            new (&_p2p_protocol) T_P2P_PROTOCOL(_dispatch_id, fn, (void*)this,
                                                __global.mpi_device,
                                                status);
            XMI_assertf(status == XMI_SUCCESS,"<%p>P2PMcastProto::register status=%d\n",this,status);
            return status;
          }
        ///
        /// \brief Base class destructor.
        ///
        /// \note Any class with virtual functions must define a virtual
        ///       destructor.
        ///
//        virtual ~P2PMcastProto ()
//        {
//        };

        ///
        /// \brief Start a new multicast message.
        ///
        /// \param[in] mcast
        ///
        xmi_result_t multicast(uint8_t (&state)[sizeof_msg],
                               xmi_multicast_t *mcast) // \todo deprecated - remove
          {
            postMulticast_impl(state, mcast);
            return XMI_SUCCESS;
          };
        xmi_result_t postMulticast_impl(uint8_t (&state)[sizeof_msg],
                                        xmi_multicast_t *mcast)
          {
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto::multicast() id %zd, connection_id %d\n",this,mcast->dispatch,mcast->connection_id));

            // First, send (p2p) the header/msgdata to dispatch destinations.  They will start all-sided multicasts when dispatched.
            dispatch_hdr_t header;
            header.root  = _task_id;
            header.bytes = mcast->bytes;
            header.connection_id = mcast->connection_id;

            // Use sendi so we don't need to allocate storage. \todo does msgdata always fit in immediate send? Probably need a check.
            xmi_send_immediate_t sendi;
            sendi.dispatch        = _dispatch_id;
            sendi.header.iov_base = (void*)&header;
            sendi.header.iov_len  = sizeof(header);
            sendi.data.iov_base   = (void*)mcast->msginfo;
            sendi.data.iov_len    = mcast->msgcount*sizeof(xmi_quad_t);

            // \todo indexToRank() doesn't always work so convert a local copy to a list topology...
            XMI::Topology l_dst_participants = *((XMI::Topology*)mcast->dst_participants);
            l_dst_participants.convertTopology(XMI_LIST_TOPOLOGY);
            xmi_task_t *rankList=NULL;  l_dst_participants.rankList(&rankList);
            size_t  size    = l_dst_participants.size();
            for(unsigned i = 0; i< size; ++i)
                {
                  if(rankList[i]==_task_id) continue; // don't dispatch myself

                  sendi.dest = XMI_ENDPOINT_INIT(_client, rankList[i],0);

                  // Dispatch over p2p
                  TRACE_DEVICE((stderr,"<%p>P2PMcastProto::multicast() send dispatch task_id[%d] %zd\n",
                                this, i, rankList[i]));
                  ((T_P2P_PROTOCOL*)&_p2p_protocol)->immediate(&sendi);
                }

            // No data? We're done.
            if(mcast->bytes == 0)
                {
/** \todo fix or remove this hack */
                  // call original done
                  if(mcast->cb_done.function)
                    (mcast->cb_done.function)(NULL,//XMI_Client_getcontext(mcast->client,mcast->context),
                                              mcast->cb_done.clientdata, XMI_SUCCESS);
                  return XMI_SUCCESS;
                }

            allocation_t *allocation = (allocation_t *) _allocator.allocateObject();
            allocation->protocol = this; // so we can free it later
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto::multicast() allocated %p, mcast %p, cb_done %p, client data %p\n",
                          this, allocation, mcast, mcast->cb_done.function, mcast->cb_done.clientdata));

            // work with a local copy of mcast and override cb_done and (maybe) src topology since all-sided requires it.
            xmi_multicast_t l_mcast = *mcast;
            if(l_mcast.src_participants == NULL) // all-sided model expects a src/root topology
                {
                  new (&allocation->topology) XMI::Topology(_task_id);
                  l_mcast.src_participants = (xmi_topology_t *) &allocation->topology;
                }
            // Save the caller's cb_done and set our own so we can free the allocation
            allocation->cb_done = l_mcast.cb_done;
            l_mcast.cb_done.clientdata = (void*) allocation;
            l_mcast.cb_done.function = &done;

            //This is an all-sided multicast


            return _multicast_model.postMulticast(allocation->request,&l_mcast);
          }
        ///
        /// \brief multicast is done, free the allocation and call user cb_done
        ///
        /// \param[in] context
        /// \param[in] cookie (allocation pointer)
        /// \param[in] result
        ///
        static void done(xmi_context_t   context,
                         void          * cookie,
                         xmi_result_t    result )
          {
            allocation_t  *allocation = (allocation_t *) cookie;

            // get the original done cb (saved in allocation)
            xmi_callback_t cb_done = allocation->cb_done;

            // Find (this) protocol and it's associated allocator
            P2PMcastProto<T_P2P_DEVICE,T_P2P_PROTOCOL, T_MULTICAST_MODEL>  *protocol = allocation->protocol;

            TRACE_DEVICE((stderr,"<%p>P2PMcastProto::done() free allocation %p, cb_done %p, client data %p\n",
                          (unsigned)protocol, cookie, cb_done.function, cb_done.clientdata));

            memset(cookie, 0x00, sizeof(allocation_t)); // cleanup for debug
            protocol->_allocator.returnObject(cookie);  // and release storage

            // call original done
/** \todo fix or remove this hack */
            if(cb_done.function)
              (cb_done.function)(NULL,//XMI_Client_getcontext(protocol->_client,protocol->_contextid),
                                 cb_done.clientdata, result);

            return;
          }


        ///
        /// \brief Received a p2p dispatch from another src (static function).  Call the member function on the protocol.
        ///
        static void dispatch_p2p(xmi_context_t        context_hdl,  /**< IN:  communication context handle */
                                 void               * cookie,       /**< IN:  dispatch cookie (pointer to protocol object)*/
                                 void               * header,       /**< IN:  header address  */
                                 size_t               header_size,  /**< IN:  header size     */
                                 void               * data,         /**< IN:  address of XMI pipe  buffer, valid only if non-NULL        */
                                 size_t               data_size,    /**< IN:  number of byts of message data, valid regarldless of message type */
                                 xmi_recv_t         * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
          {
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto::dispatch_p2p header size %zd, data size %zd\n",(unsigned)cookie, header_size, data_size));
            P2PMcastProto<T_P2P_DEVICE,T_P2P_PROTOCOL,T_MULTICAST_MODEL> *p = (P2PMcastProto<T_P2P_DEVICE,T_P2P_PROTOCOL,T_MULTICAST_MODEL> *)cookie;
            p->dispatch(context_hdl,
                        header,
                        header_size,
                        data,
                        data_size,
                        recv);
          }
        ///
        /// \brief Received a p2p dispatch from another src (member function).
        /// Call user's dispatch, allocate some storage and start all-sided multicast.
        ///
        void dispatch(xmi_context_t        context_hdl,  /**< IN:  communication context handle */
                      void               * header,       /**< IN:  header address  */
                      size_t               header_size,  /**< IN:  header size     */
                      void               * data,         /**< IN:  address of XMI pipe  buffer, valid only if non-NULL        */
                      size_t               data_size,    /**< IN:  number of byts of message data, valid regarldless of message type */
                      xmi_recv_t         * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
          {
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto::dispatch() header size %zd, data size %zd\n",this, header_size, data_size));

            // Call user's dispatch to get receive pwq and cb_done.
            xmi_multicast_t mcast;
            mcast.connection_id = ((dispatch_hdr_t*)header)->connection_id;
            mcast.bytes         = ((dispatch_hdr_t*)header)->bytes;
            _dispatch_fn((xmi_quad_t*)data, (unsigned)data_size/sizeof(xmi_quad_t), mcast.connection_id, (size_t)(((dispatch_hdr_t*)header)->root), mcast.bytes, _cookie, &mcast.bytes, &mcast.dst, &mcast.cb_done);

            // No data? We're done.
            if(mcast.bytes == 0)
                {
                  // call original done
                  /** \todo fix or remove this hack */
                  if(mcast.cb_done.function)
                    (mcast.cb_done.function)(NULL,//XMI_Client_getcontext(_client,_contextid),
                                             mcast.cb_done.clientdata, XMI_SUCCESS);
                  return;
                }

            // Allocate storage and call all-sided multicast.
            mcast.client = _client;
            mcast.context  = _contextid;

            allocation_t *allocation = (allocation_t *) _allocator.allocateObject();
            allocation->protocol = this; // so we can free it later
            TRACE_DEVICE((stderr,"<%p>P2PMcastProto::dispatch_p2p() allocated %p, cb_done %p, client data %p\n",
                          this, allocation, mcast.cb_done.function, mcast.cb_done.clientdata));

            mcast.src_participants = NULL; // I'm a dst so all-sided mcast src can be null

            // Save the caller's cb_done and set our own so we can free the allocation
            allocation->cb_done = mcast.cb_done;
            mcast.cb_done.clientdata = (void*) allocation;
            mcast.cb_done.function = &done;

            mcast.dispatch = _dispatch_id;
            //mcast.hints = 0;
            mcast.roles = 0;

            mcast.src = NULL;
            mcast.dst_participants = (xmi_topology_t *) &_dst_participants; // this task is dst
            mcast.msginfo = NULL;
            mcast.msgcount = 0;

            _multicast_model.postMulticast(allocation->request,
                                           &mcast);


          }
        XMI::Topology                 _dst_participants; // default dst is this task when dispatched
        size_t                        _dispatch_data_id;
        size_t                        _dispatch_header_id;
        size_t                        _dispatch_id;
        xmi_dispatch_multicast_fn     _dispatch_fn;
        xmi_client_t                  _client;
        xmi_context_t                 _context;
        size_t                        _contextid;
        size_t                        _task_id;
        void                         *_cookie;
        char                          _p2p_protocol[sizeof(T_P2P_PROTOCOL)]; // p2p send protocol
        T_MULTICAST_MODEL             _multicast_model; // all-sided model
        MemoryAllocator < sizeof(allocation_t), 16 > _allocator;
      }; // XMI::Protocol::P2PMcastProto class
    };   // XMI::Protocol::MPI namespace
  };   // XMI::Protocol namespace
};     // XMI namespace
#undef TRACE_DEVICE
#endif //__devices_mpi_mpimulticastprotocol_h__
