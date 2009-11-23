/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file devices/mpi/mpimulticastprotocol.h
/// \brief Defines protocol classes for multicast
///
#ifndef __devices_mpi_mpimulticastprotocol_h__
#define __devices_mpi_mpimulticastprotocol_h__

#include "sys/xmi.h"
#include <list>
#include "components/devices/mpi/mpicollectiveheader.h"

#ifndef TRACE_ADAPTOR
  #define TRACE_ADAPTOR(x) //fprintf x
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
      class P2pDispatchMulticastProtocol 
      {
        typedef struct _p2p_header_
        {
          unsigned connection_id;
          size_t   bytes;
        } p2p_hdr_t;
      public:

        ///
        /// \brief Base class constructor
        ///
        inline P2pDispatchMulticastProtocol(size_t                     dispatch_id,       
                                            xmi_dispatch_multicast_fn  dispatch,
                                            void                     * cookie,
                                            T_P2P_DEVICE             & p2p_device,
                                            //size_t                     origin_task,
                                            xmi_client_t               client,
                                            xmi_context_t              context,
                                            size_t                     contextid,
                                            xmi_result_t             & status) :
        _dst_participants(__global.mapping.task()),
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
          TRACE_ADAPTOR((stderr,"<%#8.8X>P2pDispatchMulticastProtocol\n",(unsigned)this));
          xmi_dispatch_callback_fn fn; 
          fn.p2p = dispatch_p2p;
          new (&_p2p_protocol) T_P2P_PROTOCOL(dispatch_id, fn, (void*)this, 
                                              p2p_device,
                                              __global.mapping.task(),
                                              context, contextid, status);
          TRACE_ADAPTOR((stderr,"<%#8.8X>P2pDispatchMulticastProtocol status %d\n",(unsigned)this,status));
        }

        ///
        /// \brief Base class destructor.
        ///
        /// \note Any class with virtual functions must define a virtual
        ///       destructor.
        ///
//        virtual ~P2pDispatchMulticastProtocol ()
//        {
//        };

        ///
        /// \brief Start a new multicast message.
        ///
        /// \param[in] mcast
        ///
        xmi_result_t multicast(xmi_multicast_t *mcast)
        {
          TRACE_ADAPTOR((stderr,"<%#8.8X>P2pDispatchMulticastProtocol id %zd, connection_id %d\n",(unsigned)this,mcast->dispatch,mcast->connection_id));
          p2p_hdr_t header;
          header.bytes = mcast->bytes;
          header.connection_id = mcast->connection_id;

          xmi_send_immediate_t sendi;
          sendi.dispatch        = _dispatch_id;
          sendi.header.iov_base = (void*)&header;
          sendi.header.iov_len  = sizeof(header);
          sendi.data.iov_base   = (void*)mcast->msginfo;
          sendi.data.iov_len    = mcast->msgcount*sizeof(xmi_quad_t);

          ((XMI::Topology*)(mcast->dst_participants))->convertTopology(XMI_LIST_TOPOLOGY);
          size_t *rankList;  ((XMI::Topology*)(mcast->dst_participants))->rankList(&rankList);
          size_t  size    = ((XMI::Topology*)(mcast->dst_participants))->size();
          for(unsigned i = 0; i< size; ++i)
          {
            if(rankList[i]==_task_id) continue; // don't dispatch myself
            sendi.task = rankList[i];
            // Dispatch over p2p
            ((T_P2P_PROTOCOL*)&_p2p_protocol)->immediate(&sendi);
          }

          //This is an all-sided multicast
          if(mcast->request == NULL)
          {
            mcast->request  = new char[T_MULTICAST_MODEL::sizeof_msg]; // \todo memory leak
          }
          _multicast_model.postMulticast(mcast);
          return XMI_SUCCESS;
        }

        static void dispatch_p2p(xmi_context_t        context_hdl,  /**< IN:  communication context handle */
                                 size_t               context_idx,  /**< IN:  communication context index  */
                                 void               * cookie,       /**< IN:  dispatch cookie */
                                 xmi_task_t           task,         /**< IN:  source task     */
                                 void               * header,       /**< IN:  header address  */
                                 size_t               header_size,  /**< IN:  header size     */
                                 void               * data,         /**< IN:  address of XMI pipe  buffer, valid only if non-NULL        */
                                 size_t               data_size,    /**< IN:  number of byts of message data, valid regarldless of message type */
                                 xmi_recv_t         * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
        {
          TRACE_ADAPTOR((stderr,"<%#8.8X>P2pDispatchMulticastProtocol:;dispatch_p2p header size %zd, data size %zd\n",(unsigned)cookie, header_size, data_size));
          P2pDispatchMulticastProtocol<T_P2P_DEVICE,T_P2P_PROTOCOL,T_MULTICAST_MODEL> *p = (P2pDispatchMulticastProtocol<T_P2P_DEVICE,T_P2P_PROTOCOL,T_MULTICAST_MODEL> *)cookie;
          p->dispatch(context_hdl,    
                      context_idx, 
                      task,        
                      header,      
                      header_size, 
                      data,        
                      data_size,   
                      recv);        
        }
        void dispatch(xmi_context_t        context_hdl,  /**< IN:  communication context handle */
                      size_t               context_idx,  /**< IN:  communication context index  */
                      xmi_task_t           task,         /**< IN:  source task     */
                      void               * header,       /**< IN:  header address  */
                      size_t               header_size,  /**< IN:  header size     */
                      void               * data,         /**< IN:  address of XMI pipe  buffer, valid only if non-NULL        */
                      size_t               data_size,    /**< IN:  number of byts of message data, valid regarldless of message type */
                      xmi_recv_t         * recv)         /**< OUT: receive message structure, only needed if addr is non-NULL */
        {
          TRACE_ADAPTOR((stderr,"<%#8.8X>P2pDispatchMulticastProtocol:;dispatch_p2p header size %zd, data size %zd\n",(unsigned)this, header_size, data_size));

          xmi_multicast_t mcast;
          mcast.connection_id = ((p2p_hdr_t*)header)->connection_id;   
          mcast.bytes         = ((p2p_hdr_t*)header)->bytes;

          _dispatch_fn((xmi_quad_t*)data, (unsigned)data_size/sizeof(xmi_quad_t), mcast.connection_id, (size_t)task, mcast.bytes, _cookie, &mcast.bytes, &mcast.dst, &mcast.cb_done);

          mcast.client = _client;
          mcast.context  = _contextid;     
          mcast.request  = new char[T_MULTICAST_MODEL::sizeof_msg]; // \todo memory leak
          mcast.dispatch = _dispatch_id;        
          //mcast.hints = 0;           
          mcast.roles = 0;           
          mcast.src = NULL;             
          mcast.src_participants = (xmi_topology_t *) new XMI::Topology(task); // \todo another memory leak
          mcast.dst_participants = (xmi_topology_t *) &_dst_participants;
          mcast.msginfo = NULL;        
          mcast.msgcount = 0;         

          _multicast_model.postMulticast(&mcast);


        }
        XMI::Topology                 _dst_participants;
        size_t                        _dispatch_data_id;
        size_t                        _dispatch_header_id;
        size_t                        _dispatch_id;
        xmi_dispatch_multicast_fn     _dispatch_fn;
        xmi_client_t                  _client;
        xmi_context_t                 _context;
        size_t                        _contextid;
        size_t                        _task_id;
        void                         *_cookie;
        char                          _p2p_protocol[sizeof(T_P2P_PROTOCOL)];
        T_MULTICAST_MODEL             _multicast_model;
      }; // XMI::Protocol::P2pDispatchMulticastProtocol class
    };   // XMI::Protocol::MPI namespace
  };   // XMI::Protocol namespace
};     // XMI namespace

#endif //__devices_mpi_mpimulticastprotocol_h__







