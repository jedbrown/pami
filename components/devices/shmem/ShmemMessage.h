/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemMessage_h__
#define __components_devices_shmem_ShmemMessage_h__

#include <sys/uio.h>

#include "Memregion.h"

#include "sys/xmi.h"

#include "util/common.h"
#include "util/queue/Queue.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Packet>
    class ShmemMessage : public QueueElem
    {
      public:

        enum shmem_pkt_t
        {
          PTP,
          RMA
        };

        inline ShmemMessage (xmi_client_t client, size_t        context,
                             xmi_event_function   fn,
                             void               * cookie,
                             uint16_t             dispatch_id,
                             void               * metadata,
                             size_t               metasize,
                             void               * src,
                             size_t               bytes,
                             bool                 packed) :
            QueueElem (),
            _client (client),
            _context (context),
            _fn (fn),
            _cookie (cookie),
            _iov (&__iov[0]),
            _tiov (1),
            _niov (0),
            _nbytes (0),
            _dispatch_id (dispatch_id),
            _pkt_type(PTP)
        {
          __iov[0].iov_base = src;
          __iov[0].iov_len  = bytes;

          memcpy(_metadata, metadata, metasize);
        };

        inline ShmemMessage (xmi_client_t client, size_t        context,
                             xmi_event_function   fn,
                             void               * cookie,
                             uint16_t             dispatch_id,
                             void               * metadata,
                             size_t               metasize,
                             void               * src0,
                             size_t               bytes0,
                             void               * src1,
                             size_t               bytes1,
                             bool                 packed) :
            QueueElem (),
            _client (client),
            _context (context),
            _fn (fn),
            _cookie (cookie),
            _iov (&__iov[0]),
            _tiov (2),
            _niov (0),
            _nbytes (0),
            _dispatch_id (dispatch_id),
            _pkt_type(PTP)
        {
          __iov[0].iov_base = src0;
          __iov[0].iov_len  = bytes0;
          __iov[1].iov_base = src1;
          __iov[1].iov_len  = bytes1;

          memcpy(_metadata, metadata, metasize);
        };

        inline ShmemMessage (xmi_client_t client, size_t        context,
                             xmi_event_function   fn,
                             void               * cookie,
                             uint16_t             dispatch_id,
                             void               * metadata,
                             size_t               metasize,
                             struct iovec       * iov,
                             size_t               niov,
                             bool                 packed) :
            QueueElem (),
            _client (client),
            _context (context),
            _fn (fn),
            _cookie (cookie),
            _iov (iov),
            _tiov (niov),
            _niov (0),
            _nbytes (0),
            _dispatch_id (dispatch_id),
            _pkt_type(PTP)
        {
          memcpy(_metadata, metadata, metasize);
        };


        inline ShmemMessage (xmi_client_t client, size_t        context,
                             xmi_event_function   fn,
                             void               * cookie,
                             uint16_t             dispatch_id,
                             void               * metadata,
                             size_t               metasize) :
            QueueElem (),
            _client (client),
            _context (context),
            _fn (fn),
            _cookie (cookie),
            _iov (&__iov[0]),
            _tiov (0),
            _niov (0),
            _nbytes (0),
            _dispatch_id (dispatch_id),
           _pkt_type(PTP)
        {
          memcpy(_metadata, metadata, metasize);
        };

        inline ShmemMessage (xmi_event_function   fn,
                             void               * cookie,
                             Memregion          * local_memregion,
                             size_t               local_offset,
                             Memregion          * remote_memregion,
                             size_t               remote_offset,
                             size_t               bytes,
                             bool                 is_put) :
            QueueElem (),
            _fn (fn),
            _cookie (cookie),
            _pkt_type(RMA),
            _rma_local_memregion (local_memregion),
            _rma_remote_memregion (remote_memregion),
            _rma_local_offset (local_offset),
            _rma_remote_offset (remote_offset),
            _rma_bytes (bytes),
            _rma_is_put (is_put)
        {
        };

        inline int executeCallback (xmi_result_t status = XMI_SUCCESS)
        {
          if (_fn)
            {
              _fn (XMI_Client_getcontext(_client, _context), _cookie, status);
            }

          return 0;
        };

        inline void * next (size_t & bytes, size_t max)
        {
          void * addr = (void *)(((uint8_t *)_iov[_niov].iov_base) + _nbytes);

          // Return minimum of the bytes remaining in this iov and the
          // maximum packet payload.
          bytes = MIN((_iov[_niov].iov_len - _nbytes), max);
          _nbytes += bytes;

          if (_nbytes == _iov[_niov].iov_len)
            {
              _nbytes = 0;
              _niov++;
            }

          return addr;
        };

        inline bool done ()
        {
          return (_tiov == _niov) ? true : false;
        };

        inline uint16_t getDispatchId ()
        {
          return _dispatch_id;
        }

        inline void * getMetadata ()
        {
          return (void *) &_metadata;
        }

        inline size_t getSequenceId ()
        {
          return _sequence_id;
        }

        inline void setSequenceId (size_t sequence)
        {
          _sequence_id = sequence;
        }

        inline bool isRMAType()
        {
          return (_pkt_type == RMA);
        }

        inline bool getRMA (Memregion ** local_memregion,
                            size_t     & local_offset,
                            Memregion ** remote_memregion,
                            size_t     & remote_offset,
                            size_t     & bytes)
        {
          *local_memregion  = _rma_local_memregion;
          *remote_memregion = _rma_remote_memregion;
          local_offset      = _rma_local_offset;
          remote_offset     = _rma_remote_offset;
          bytes             = _rma_bytes;

          return _rma_is_put;
        }



      protected:
        xmi_client_t        _client;
        size_t        _context;

        // Client callback information.
        xmi_event_function   _fn;
        void               * _cookie;

        struct iovec    * _iov;
        size_t            _tiov;
        size_t            _niov;
        size_t            _nbytes;

        uint16_t _dispatch_id;
        size_t   _sequence_id;

        shmem_pkt_t		_pkt_type;

        Memregion * _rma_local_memregion;
        Memregion * _rma_remote_memregion;
        size_t _rma_local_offset;
        size_t _rma_remote_offset;
        size_t _rma_bytes;
        bool   _rma_is_put;

      private:
        struct iovec      __iov[2];
        uint8_t _metadata[T_Packet::headerSize_impl] __attribute__ ((aligned (16)));
    };
  };
};

#endif // __components_devices_shmem_shmembasemessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
