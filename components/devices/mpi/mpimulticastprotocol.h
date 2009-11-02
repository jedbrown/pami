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
/// \brief Defines base protocol class interface for multicast
///
#ifndef __devices_mpi_mpimulticastprotocol_h__
#define __devices_mpi_mpimulticastprotocol_h__

#include "sys/xmi.h"
#include <list>
#include "components/devices/mpi/mpiheader.h"

#ifndef TRACE_DEVICE
  #define TRACE_ADAPTOR(x) //fprintf x
#endif

namespace XMI
{
  namespace Protocol
  {
    ///
    /// \brief Base class for multicast implementations.
    /// \todo move somewhere common, expand and CRTP'ify it?
    class MulticastProtocol
    {
    public:

      ///
      /// \brief Base class constructor for point-to-point sends.
      ///
      inline MulticastProtocol ()
      {
        TRACE_ADAPTOR((stderr,":%d:MulticastProtocol\n",__LINE__));
      };

      ///
      /// \brief Base class destructor.
      ///
      /// \note Any class with virtual functions must define a virtual
      ///       destructor.
      ///
      virtual ~MulticastProtocol ()
      {
      };

      ///
      /// \brief Start a new multicast message.
      ///
      /// \param[in] mcastinfo
      ///
      virtual xmi_result_t multicast(xmi_multicast_t *mcastinfo) = 0;
    }; // XMI::Protocol::MulticastProtocol class

    namespace MPI
    {
      ///
      /// \brief Base class for multicast implementations.
      ///
      template <
      class T_Device, 
      class T_Header>
      class OneSidedMulticastProtocol : public MulticastProtocol
      {
      public:

        ///
        /// \brief Base class constructor for point-to-point sends.
        ///
        inline OneSidedMulticastProtocol ()
        {
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol\n",__LINE__));
        };
        inline OneSidedMulticastProtocol(size_t                     dispatch_id,       
                                         xmi_dispatch_multicast_fn  dispatch,
                                         void                     * cookie,
                                         T_Device                 * device,
                                         //size_t                     origin_task,
                                         xmi_context_t              context,
                                         size_t                     contextid,
                                         xmi_result_t             & status) :
        _dispatch_id(dispatch_id),
        _dispatch_fn(dispatch),
//          _task_id(origin_task),
        _context(context),
        _contextid(contextid),
        _task_id((size_t)__global.mapping.task()),
        _cookie(cookie),
        _device(device)
        {
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol\n",__LINE__));
          _dispatch_header_id = _device->registerRecvFunction (dispatch_id,
                                                               &dispatch_header,
                                                               this);
          _dispatch_data_id = _device->registerRecvFunction (dispatch_id,
                                                             &dispatch_advance,
                                                             this);
          T_Header *dummy = new T_Header();
          _device->enqueueDispatch(dummy);
          dummy = new T_Header();
          _device->enqueueData(dummy);
          status = XMI_SUCCESS;
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol status %d\n",__LINE__,status));
        }

        ///
        /// \brief Base class destructor.
        ///
        /// \note Any class with virtual functions must define a virtual
        ///       destructor.
        ///
        virtual ~OneSidedMulticastProtocol ()
        {
        };

        ///
        /// \brief Start a new multicast message.
        ///
        /// \param[in] mcastinfo
        ///
        xmi_result_t multicast(xmi_multicast_t *mcastinfo)
        {
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol id %zd, connection_id %d\n",__LINE__,mcastinfo->dispatch,mcastinfo->connection_id));
          // \todo Simplistic.  Need more thought on connection_id's...
          XMI_assert(mcastinfo->connection_id < (sizeof(_connection_table) - 1));
          XMI_assert(_connection_table[mcastinfo->connection_id]==NULL);
          T_Header* header = _connection_table[mcastinfo->connection_id] = new T_Header(mcastinfo,_dispatch_header_id,_task_id);
          _device->enqueueDispatch(header);
          return XMI_SUCCESS;
        }

        static void dispatch_header(void    *header,
                                    uint16_t length,
                                    void    *clientdata)
        {
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol:;dispatch_header\n",__LINE__));
          OneSidedMulticastProtocol<T_Device, T_Header> *client = (OneSidedMulticastProtocol<T_Device, T_Header> *)clientdata;
          client->dispatchHeader((T_Header*)header, length);
        }
        static void dispatch_advance(void    *header,
                                     uint16_t length,
                                     void    *clientdata)
        {
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol::dispatch_advance\n",__LINE__));
          OneSidedMulticastProtocol<T_Device, T_Header> *client = (OneSidedMulticastProtocol<T_Device, T_Header> *)clientdata;
          client->dispatchAdvance((T_Header*)header, length);
        }

        void dispatchHeader(T_Header *device_header, 
                            uint16_t  length)
        {
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol::dispatchHeader length %d device_header->id() %d, device_header->root() %zd, _cookie %p\n",__LINE__,
                         length, device_header->id(), device_header->root(), _cookie));
          _device->getDispatchHeader(device_header, length);
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol::dispatchHeader dispatch_id %d, msginfo %p, msgcount %d, connection_id %d, root %d, sndlen %d, _cookie %p\n",
                          __LINE__,_dispatch_id, device_header->msginfo(), device_header->msgcount(), device_header->connection_id(), device_header->root(), device_header->sndlen(), _cookie));

          size_t               rcvlen;
          xmi_pipeworkqueue_t *rcvpwq;
          xmi_callback_t       cb_done;

          T_Header *header = _connection_table[device_header->connection_id()];
          // \todo Simplistic.  Need more thought on connection_id's...
          if(header == NULL)
          {
            TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol::_dispatch_fn dispatch_id %d, msginfo %p, msgcount %d, connection_id %d, root %d, sndlen %d, _cookie %p\n",
                           __LINE__,_dispatch_id, device_header->msginfo(), device_header->msgcount(), device_header->connection_id(), device_header->root(), device_header->sndlen(), _cookie));
            _dispatch_fn(device_header->msginfo(), device_header->msgcount(), device_header->connection_id(), device_header->root(), device_header->sndlen(), _cookie, &rcvlen, &rcvpwq, &cb_done);
            TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol::_dispatch_fn dispatch_id %d, rcvlen %d, rcvpwq %p, cb_done.function %p, cb_done.clientdata %p\n",
                           __LINE__,_dispatch_id, rcvlen, rcvpwq, cb_done.function, cb_done.clientdata));
            _connection_table[device_header->connection_id()] = header = new T_Header(device_header, _context, _dispatch_data_id, _task_id, rcvlen, rcvpwq, cb_done);
          }
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol::dispatchHeader connection_table dispatch_id %d, msginfo %p, msgcount %d, connection_id %d, root %d, sndlen %d, _cookie %p\n",
                          __LINE__,_dispatch_id, header->msginfo(), header->msgcount(), header->connection_id(), header->root(), header->sndlen(), _cookie));

          if(header->root()==_task_id)
          {
            header->setId(_dispatch_data_id);
            _device->enqueueData(header);
          }
        }
        void dispatchAdvance(T_Header *device_header,
                             uint16_t  length)
        {
          TRACE_ADAPTOR((stderr,":%d:OneSidedMulticastProtocol::dispatchAdvance length %d device_header->msginfo() %p, device_header->msgcount() %d, device_header->connection_id() %d, device_header->root() %zd, device_header->sndlen() %zd, _cookie %p\n",__LINE__,
                         length, device_header->msginfo(), device_header->msgcount(), device_header->connection_id(), device_header->root(), device_header->sndlen(), _cookie));

          T_Header *header = _connection_table[device_header->connection_id()];
          XMI_assert(header);


//          if(header->protocol() == XMI::Device::MULTICAST)
          {
            if(_device->getBytesAvailable(header))
            {
              _device->multicast(header);
              if(header->done())
              {
                header->complete();
                if(!header->isRoot())
                  ;//delete header;
                _connection_table[device_header->connection_id()] = NULL;
              }
            }
          }
        }
        size_t                        _dispatch_data_id;
        size_t                        _dispatch_header_id;
        size_t                        _dispatch_id;
        xmi_dispatch_multicast_fn     _dispatch_fn;
        xmi_context_t                 _context;
        size_t                        _contextid;
        size_t                        _task_id;
        void                         *_cookie;
        T_Device                     *_device;
        T_Header*                    _connection_table[16];
      }; // XMI::Protocol::OneSidedMulticastProtocol class
      ///
      /// \brief Base class for multicast implementations.
      ///
      template <
      class T_Device, 
      class T_Header>
      class AllSidedMulticastProtocol : public MulticastProtocol
      {
      public:

        ///
        /// \brief Base class constructor for point-to-point sends.
        ///
        inline AllSidedMulticastProtocol ()
        {
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol\n",__LINE__));
        };
        inline AllSidedMulticastProtocol(size_t                     dispatch_id,       
                                         xmi_dispatch_multicast_fn  dispatch,
                                         void                     * cookie,
                                         T_Device                 * device,
                                         //size_t                     origin_task,
                                         xmi_context_t              context,
                                         size_t                     contextid,
                                         xmi_result_t             & status) :
        _dispatch_id(dispatch_id),
        _dispatch_fn(dispatch),
//          _task_id(origin_task),
        _task_id((size_t)__global.mapping.task()),
        _context(context),
        _contextid(contextid),
        _cookie(cookie),
        _device(device)
        {
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol\n",__LINE__));
          _dispatch_header_id = _device->registerRecvFunction (dispatch_id,
                                                               &dispatch_header,
                                                               this);
          _dispatch_data_id = _device->registerRecvFunction (dispatch_id,
                                                             &dispatch_advance,
                                                             this);
          /*
          T_Header *dummy = new T_Header();
          _device->enqueueDispatch(dummy);
          dummy = new T_Header();
          _device->enqueueData(dummy);
          status = XMI_SUCCESS;
          */
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol status %d\n",__LINE__,status));
        }

        ///
        /// \brief Base class destructor.
        ///
        /// \note Any class with virtual functions must define a virtual
        ///       destructor.
        ///
        virtual ~AllSidedMulticastProtocol ()
        {
        };

        ///
        /// \brief Start a new multicast message.
        ///
        /// \param[in] mcastinfo
        ///
        xmi_result_t multicast(xmi_multicast_t *mcastinfo)
        {
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol id %zd, connection_id %d\n",__LINE__,mcastinfo->dispatch,mcastinfo->connection_id));
          // \todo Simplistic.  Need more thought on connection_id's...
          XMI_assert(mcastinfo->connection_id < (sizeof(_connection_table) - 1));
          XMI_assert(_connection_table[mcastinfo->connection_id]==NULL);
          T_Header* header = _connection_table[mcastinfo->connection_id] = new T_Header(mcastinfo,_dispatch_header_id,_task_id);
          _device->enqueueDispatch(header);
          return XMI_SUCCESS;
        }

        static void dispatch_header(void    *header,
                                    uint16_t length,
                                    void    *clientdata)
        {
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol:;dispatch_header\n",__LINE__));
          AllSidedMulticastProtocol<T_Device, T_Header> *client = (AllSidedMulticastProtocol<T_Device, T_Header> *)clientdata;
          client->dispatchHeader((T_Header*)header, length);
        }
        static void dispatch_advance(void    *header,
                                     uint16_t length,
                                     void    *clientdata)
        {
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol::dispatch_advance\n",__LINE__));
          AllSidedMulticastProtocol<T_Device, T_Header> *client = (AllSidedMulticastProtocol<T_Device, T_Header> *)clientdata;
          client->dispatchAdvance((T_Header*)header, length);
        }

        void dispatchHeader(T_Header *device_header, 
                            uint16_t  length)
        {
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol::dispatchHeader length %d device_header->id() %d, device_header->root() %zd, _cookie %p\n",__LINE__,
                         length, device_header->id(), device_header->root(), _cookie));
          _device->getDispatchHeader(device_header, length);
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol::dispatchHeader dispatch_id %d, msginfo %p, msgcount %d, connection_id %d, root %d, sndlen %d, _cookie %p\n",
                          __LINE__,_dispatch_id, device_header->msginfo(), device_header->msgcount(), device_header->connection_id(), device_header->root(), device_header->sndlen(), _cookie));

          size_t               rcvlen;
          xmi_pipeworkqueue_t *rcvpwq;
          xmi_callback_t       cb_done;

          T_Header *header = _connection_table[device_header->connection_id()];
          XMI_assert(header == NULL);
          header->complete();
          return;
          // \todo Simplistic.  Need more thought on connection_id's...
          if(header == NULL)
          {
            TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol::_dispatch_fn dispatch_id %d, msginfo %p, msgcount %d, connection_id %d, root %d, sndlen %d, _cookie %p\n",
                           __LINE__,_dispatch_id, device_header->msginfo(), device_header->msgcount(), device_header->connection_id(), device_header->root(), device_header->sndlen(), _cookie));
            _dispatch_fn(device_header->msginfo(), device_header->msgcount(), device_header->connection_id(), device_header->root(), device_header->sndlen(), _cookie, &rcvlen, &rcvpwq, &cb_done);
            TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol::_dispatch_fn dispatch_id %d, rcvlen %d, rcvpwq %p, cb_done.function %p, cb_done.clientdata %p\n",
                           __LINE__,_dispatch_id, rcvlen, rcvpwq, cb_done.function, cb_done.clientdata));
            _connection_table[device_header->connection_id()] = header = new T_Header(device_header, _context, _dispatch_data_id, _task_id, rcvlen, rcvpwq, cb_done);
          }
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol::dispatchHeader connection_table dispatch_id %d, msginfo %p, msgcount %d, connection_id %d, root %d, sndlen %d, _cookie %p\n",
                          __LINE__,_dispatch_id, header->msginfo(), header->msgcount(), header->connection_id(), header->root(), header->sndlen(), _cookie));

          if(header->root()==_task_id)
          {
            header->setId(_dispatch_data_id);
            _device->enqueueData(header);
          }
        }
        void dispatchAdvance(T_Header *device_header,
                             uint16_t  length)
        {
          TRACE_ADAPTOR((stderr,":%d:AllSidedMulticastProtocol::dispatchAdvance length %d device_header->msginfo() %p, device_header->msgcount() %d, device_header->connection_id() %d, device_header->root() %zd, device_header->sndlen() %zd, _cookie %p\n",__LINE__,
                         length, device_header->msginfo(), device_header->msgcount(), device_header->connection_id(), device_header->root(), device_header->sndlen(), _cookie));

          T_Header *header = _connection_table[device_header->connection_id()];
          XMI_assert(header);


//          if(header->protocol() == XMI::Device::MULTICAST)
          {
            if(_device->getBytesAvailable(header))
            {
              _device->multicast(header);
              if(header->done())
              {
                header->complete();
                if(!header->isRoot())
                  ;//delete header;
                _connection_table[device_header->connection_id()] = NULL;
              }
            }
          }
        }
        size_t                        _dispatch_data_id;
        size_t                        _dispatch_header_id;
        size_t                        _dispatch_id;
        xmi_dispatch_multicast_fn     _dispatch_fn;
        xmi_context_t                 _context;
        size_t                        _contextid;
        size_t                        _task_id;
        void                         *_cookie;
        T_Device                     *_device;
        T_Header*                    _connection_table[16];
      }; // XMI::Protocol::AllSidedMulticastProtocol class
    };   // XMI::Protocol::MPI namespace
  };   // XMI::Protocol namespace
};     // XMI namespace

#endif //__devices_mpi_mpimulticastprotocol_h__





