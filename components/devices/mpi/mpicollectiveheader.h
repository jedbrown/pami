/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/mpi/mpicollectiveheader.h
 * \brief Various MPI headers
 * \todo CRTP'ify it to define the common header interface
 */

#ifndef __components_devices_mpi_mpicollectiveheader_h__
#define __components_devices_mpi_mpicollectiveheader_h__
#ifndef DISABLE_COLLDEVICE

#include "components/devices/mpi/mpimessage.h"
#include "PipeWorkQueue.h"
#include "Topology.h"
#include "Global.h"
#include "util/ccmi_debug.h"

#ifndef TRACE_ADAPTOR
  #define TRACE_ADAPTOR(x) //fprintf x
#endif

#ifndef TRACE_DEVICE
  #define TRACE_DEVICE(x) //fprintf x
#endif

namespace XMI
{
  namespace Device
  {
    typedef enum
    {
      NONE      = 0,
      MULTICAST,
      MULTISYNC,
      MULTICOMBINE,
      MANYTOMANY
    } protocol_t;

    template <class T_Multicast>
    class MPIMulticastHeader : public QueueElem
    {
      #define HDR_LEN 8 // temp - until I fix alignment issues with structs
      static const unsigned MAX_HEADER = 512;
    private:
      struct header_t
      {
        uint32_t   _root;         // source/root
        uint16_t _id;             // dispatch id
        // annoying union because I'm limited to 8 bytes long_long allreduce :(
        //   dispatch headers put header length in these two bytes (so we can pull metadata)
        //   subsequent headers put the connection id in these two bytes
        union
        {
          uint16_t _connection_id;// connection id of most headers
          uint16_t _length;       //of remaining dispatch header (in quads)
        } _u;
      } *_header;
      char _buffer[MAX_HEADER];
      struct msginfo_t // the user's msginfo (in _buffer)
      {
        uint32_t            sndlen;         /**< total send length expected */
        uint32_t            msgcount;       /**< meta info count*/
        char                msginfo;        /**< A extra info field to be sent with the message.
                                                  This might include information about
                                                  the data being sent, for one-sided. */

      } *_msginfo;
      T_Multicast* _msg;
      protocol_t   _protocol;
      unsigned _collectiveBytesAvailable;
    public:

      ~MPIMulticastHeader()
      {
        // \todo delete referenced objects or return to a pool?
        TRACE_DEVICE((stderr,":%d:~MPIMulticastHeader()\n",__LINE__));
      }
      MPIMulticastHeader(protocol_t protocol=MULTICAST):  QueueElem() ,
       _header((struct header_t *)_buffer),
      _msginfo((struct msginfo_t*)(_buffer+HDR_LEN)),
      _msg(NULL),
      _protocol(protocol),
      _collectiveBytesAvailable(0)
      {
        // For simplicity, I'm using MPI_UNSIGNED_LONG_LONG for the header so assert it...
        COMPILE_TIME_ASSERT(sizeof(unsigned long long) == sizeof(struct header_t));
        _header->_id=0;
        _header->_root=0;
        _header->_u._length=0;
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() buffer %p, header %p,msginfo %p, msginfo->msginfo %p, msgcount %d\n",
                      __LINE__,_buffer, _header, _msginfo, &_msginfo->msginfo,_msginfo->msgcount));
      }

      MPIMulticastHeader(T_Multicast *msg, protocol_t protocol=MULTICAST):  QueueElem() ,
       _header((struct header_t *)_buffer),
      _msginfo((struct msginfo_t*)(_buffer+HDR_LEN)),
      _msg(msg),
      _protocol(protocol),
      _collectiveBytesAvailable(0)
      {
        XMI_assert(_msg != NULL);
        _header->_id=_msg->id();
        _header->_root=_msg->root();
        _header->_u._length =_msg->msgcount() + 1 ; // metadata + 1 quad for my header
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() id %zd\n",__LINE__,_msg->id()));
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() buffer %p, header %p, msginfo %p, msginfo->msginfo %p/%p, msgcount %d/%d\n",
                      __LINE__,_buffer, _header, _msginfo,&_msginfo->msginfo,_msg->msginfo(),_msginfo->msgcount,_msg->msgcount()));
      }
      MPIMulticastHeader(xmi_multicast_t *minfo, size_t dispatch_id, size_t task_id):  QueueElem() ,
       _header((struct header_t *)_buffer),
      _msginfo((struct msginfo_t*)(_buffer+HDR_LEN)),
      _protocol(MULTICAST),
      _collectiveBytesAvailable(0)
      {
        _msg = (T_Multicast*) new T_Multicast(minfo, dispatch_id, task_id);
        XMI_assert(_msg != NULL);
        _header->_id=_msg->id();
        _header->_root=_msg->root();
        _header->_u._length=_msg->msgcount() + 1 ; // metadata + 1 quad for my header
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() id %zd,%zd/%zd\n",__LINE__,dispatch_id,minfo->dispatch,_msg->id()));
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() buffer %p, header %p, msginfo %p, msginfo->msginfo %p/%p, msgcount %d/%d\n",
                      __LINE__,_buffer, _header, _msginfo,&_msginfo->msginfo,_msg->msginfo(),_msginfo->msgcount,_msg->msgcount()));
      }
      MPIMulticastHeader(MPIMulticastHeader* header, size_t context, size_t dispatch_id, size_t task_id, size_t rcvlen, xmi_pipeworkqueue_t *rcvpwq, xmi_callback_t cb_done):  QueueElem() ,
       _header((struct header_t *)_buffer),
      _msginfo((struct msginfo_t*)(_buffer+HDR_LEN)),
      _protocol(MULTICAST),
      _collectiveBytesAvailable(0)
      {
        xmi_multicast_t minfo;
        minfo.context = context;
        minfo.dispatch = dispatch_id;
        // who cares now? minfo.hints =
        minfo.cb_done = cb_done;
        minfo.connection_id = header->connection_id();
        minfo.roles = 0;
        minfo.bytes = rcvlen;
        minfo.src = NULL;
        minfo.src_participants = (xmi_topology_t*) new XMI::Topology(header->root());
        minfo.dst = rcvpwq;
        minfo.dst_participants = (xmi_topology_t*) new XMI::Topology(task_id);
        minfo.msginfo = header->msginfo();
        minfo.msgcount = header->msgcount();
        _msg = (T_Multicast*) new T_Multicast(&minfo, dispatch_id, task_id);
        XMI_assert(_msg != NULL);
        _header->_id=_msg->id();
        _header->_root=_msg->root();
        _header->_u._length=_msg->msgcount() + 1 ; // metadata + 1 quad for my header
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() id %zd,%zd/%zd\n",__LINE__,dispatch_id,minfo.dispatch,_msg->id()));
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() buffer %p, header %p, msginfo %p, msginfo->msginfo %p/%p, msgcount %d/%d\n",
                      __LINE__,_buffer, _header, _msginfo,&_msginfo->msginfo,_msg->msginfo(),_msginfo->msgcount,_msg->msgcount()));
      }
      bool operator==(const MPIMulticastHeader& r)
      {
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() operator== (root() %zd==%zd) && (id() %zd==%zd)\n",__LINE__,root(),r.root(),id(),r.id()));
        return((root() == r.root()) && (id() == r.id()));
      }
/*      MPIMulticastHeader(MPIMulticastHeader* header, size_t dispatch_id):  QueueElem() ,
       _header((struct header_t *)_buffer),
      _msginfo((struct msginfo_t*)(_buffer+HDR_LEN)),
        _msg(header->_msg),
      _protocol(MULTICAST),
      _collectiveBytesAvailable(0)
      {
        XMI_assert(_msg != NULL);
        _header->_id=dispatch_id;
        _header->_root=_msg->root();
        _header->_u._connection_id = _msg->connection_id();

        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() id %zd/%zd\n",__LINE__,dispatch_id,_msg->id()));

      }
 */
      /* let the compile fail if misused
      MPIMulticastHeader(xmi_multisync_t *minfo):  QueueElem() ,
      _protocol(MULTISYNC)
      {
        XMI_assert(_protocol == MULTICAST);
      }
      MPIMulticastHeader(xmi_multicombine_t *minfo):  QueueElem() ,
      _protocol(MULTICOMBINE)
      {
        XMI_assert(_protocol == MULTICAST);
      }
      MPIMulticastHeader(xmi_manytomany_t *minfo):  QueueElem() ,
      _protocol(MANYTOMANY)
      {
        XMI_assert(_protocol == MULTICAST);
      }
      */
      void setId(uint16_t dispatch_id)
      {
        XMI_assert(_msg != NULL);
        _header->_id=dispatch_id;
        _msg->setId(dispatch_id);

        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader() id %zd/%zd\n",__LINE__,dispatch_id,_msg->id()));

      }
      void resetFullHeader()
      {
        XMI_assert(_msg != NULL);
        // reset u._length from dispatch header to u._connection for subsequent headers and
        // fill in remaining (long) header
        TRACE_DEVICE((stderr,"resetFullHeader %p, %p\n",this,_msg));
        _header->_u._connection_id = _msg->connection_id();
        _msginfo->sndlen = _msg->bytes();
        _msginfo->msgcount = _msg->msgcount();
        TRACE_DEVICE((stderr,"resetFullHeader connection_id %d, %p, %p, %p, msgcount %d/%d, sndlen %d\n",
                      _header->_u._connection_id,_msginfo,&_msginfo->msginfo,_msg->msginfo(),_msginfo->msgcount,_msg->msgcount()*sizeof(xmi_quad_t),_msginfo->sndlen));
        memcpy(&_msginfo->msginfo,_msg->msginfo(),_msg->msgcount()*sizeof(xmi_quad_t));
        for(unsigned i = 0; i < _msginfo->msgcount*4; i+=4)
        {
          TRACE_DEVICE((stderr, "<%#.8X><%d:%d>msginfo %#.8X %#.8X %#.8X %#.8X\n",
                        (unsigned)(&_msginfo->msginfo),
                        i+1, _msginfo->msgcount,
                        ((unsigned*)&_msginfo->msginfo)[i],
                        ((unsigned*)&_msginfo->msginfo)[i+1],
                        ((unsigned*)&_msginfo->msginfo)[i+2],
                        ((unsigned*)&_msginfo->msginfo)[i+3]
                       ));
        }
      }
      void reset()
      {
        _header->_id=0;
        _header->_root=0;
        _protocol=0;
        _header->_u._length=0;
        TRACE_DEVICE((stderr,":%d:MPIMulticastHeader::reset()\n",__LINE__));
      }
      const bool isRoot()
      {
        return((_msg != NULL) && (_msg->isRoot()));
      }
      const size_t root() const
      {
        return _header->_root;
      }
      const size_t id() const
      {
        return _header->_id;
      }
      const size_t length()
      {
        return _header->_u._length;
      }
      const size_t connection_id() const
      {
        return _header->_u._connection_id;
      }
      const protocol_t protocol()
      {
        return MULTICAST;
      }
      const bool ready()
      {
        return true;
      }
      const bool done()
      {
        return((_msg!=NULL) && _msg->done());
      }
      const void complete()
      {
        XMI_assert(_msg != NULL);
        _msg->complete();
      }
      const char* buffer()
      {
        return(char*) _buffer;
      }
      const xmi_quad_t* msginfo()
      {
        return (xmi_quad_t*)&_msginfo->msginfo; // \todo alignment problems
      }
      const unsigned msgcount()
      {
        return _msginfo->msgcount;
      }
      const size_t sndlen()
      {
        return _msginfo->sndlen;
      }
      char* header()
      {
        return(char*) _header;
      }
      const unsigned available()
      {
        XMI_assert(_msg != NULL);
        return(_msg->available());
      }
      const unsigned collectiveBytesAvailable()
      {
        return _collectiveBytesAvailable;
      }
      void collectiveBytesAvailable(unsigned available)
      {
        XMI_assert(_msg != NULL);
        _collectiveBytesAvailable=available;
        _msg->dataBuffer(available);
      }
      char* dataBuffer()
      {
        XMI_assert(_msg != NULL);
        return _msg->dataBuffer();
      }
/*
      {

        bool done;
        switch(_header->_protocol)
        {
        case MULTICAST:
          done = ((T_Multicast*) _msg)->...
          break;
        default:
          XMI_abortf("Unsupported message protocol %#X\n",_header->_protocol);
        }
        return done;
      }
*/
    private:
    };
  };
};

#endif // DISABLE_COLLDEVICE
#endif // __components_devices_mpi_mpicollectiveheader_h__
