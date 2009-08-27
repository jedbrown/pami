/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2008                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemBaseMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_shmembasemessage_h__
#define __components_devices_shmem_shmembasemessage_h__

#include <sys/uio.h>

#include "xmi.h"

#include "queueing/Queue.h"

namespace XMI
{
  namespace Device
  {
    template <class T_Packet>
    class ShmemBaseMessage : public Queueing::QueueElem
    {
      public:
        inline ShmemBaseMessage (xmi_event_function   fn,
                                 void               * cookie,
                                 uint8_t           dispatch_id,
                                 void            * metadata,
                                 size_t            metasize,
                                 void            * src,
                                 size_t            bytes,
                                 bool              packed) :
            Queueing::QueueElem (),
            _cb (cb),
            _iov (&__iov[0]),
            _tiov (1),
            _niov (0),
            _nbytes (0),
            _dispatch_id (dispatch_id),
            _remote_completion (false)
        {
          __iov[0].iov_base = src;
          __iov[0].iov_len  = bytes;

          memcpy(_metadata, metadata, metasize);
        };

        inline ShmemBaseMessage (xmi_event_function   fn,
                                 void               * cookie,
                                 uint8_t           dispatch_id,
                                 void            * metadata,
                                 size_t            metasize,
                                 void            * src0,
                                 size_t            bytes0,
                                 void            * src1,
                                 size_t            bytes1,
                                 bool              packed) :
            Queueing::QueueElem (),
            _cb (cb),
            _iov (&__iov[0]),
            _tiov (2),
            _niov (0),
            _nbytes (0),
            _dispatch_id (dispatch_id),
            _remote_completion (false)
        {
          __iov[0].iov_base = src0;
          __iov[0].iov_len  = bytes0;
          __iov[1].iov_base = src1;
          __iov[1].iov_len  = bytes1;

          memcpy(_metadata, metadata, metasize);
        };

        inline ShmemBaseMessage (xmi_event_function   fn,
                                 void               * cookie,
                                 uint8_t           dispatch_id,
                                 void            * metadata,
                                 size_t            metasize,
                                 struct iovec    * iov,
                                 size_t            niov,
                                 bool              packed) :
            Queueing::QueueElem (),
            _cb (cb),
            _iov (iov),
            _tiov (niov),
            _niov (0),
            _nbytes (0),
            _dispatch_id (dispatch_id),
            _remote_completion (false)
        {
          memcpy(_metadata, metadata, metasize);
        };


        inline ShmemBaseMessage (xmi_event_function   fn,
                                 void               * cookie,
                                 uint8_t           dispatch_id,
                                 void            * metadata,
                                 size_t            metasize) :
            Queueing::QueueElem (),
            _cb (cb),
            _iov (&__iov[0]),
            _tiov (0),
            _niov (0),
            _nbytes (0),
            _dispatch_id (dispatch_id),
            _remote_completion (false)
        {
          memcpy(_metadata, metadata, metasize);
        };


        inline int executeCallback (CM_Error_t * error = NULL)
        {
          if (_cb.function)
            {
              _cb.function (_cb.clientdata, error);
            }

          return 0;
        };

        inline void next (void **addr, size_t & bytes, size_t max)
        {
          *addr = (void *)(((uint8_t *)_iov[_niov].iov_base) + _nbytes);

          // Return minimum of the bytes remaining in this iov and the
          // maximum packet payload.
          bytes = MIN((_iov[_niov].iov_len - _nbytes), max);
          _nbytes += bytes;

          if (_nbytes == _iov[_niov].iov_len)
            {
              _nbytes = 0;
              _niov++;
            }
        };

        inline bool done ()
        {
          return (_tiov == _niov) ? true : false;
        };

        inline uint8_t getDispatchId ()
        {
          return _dispatch_id;
        }

        inline void * getMetadata ()
        {
          return (void *) &_metadata;
        }

        inline bool isRemoteCompletionRequired ()
        {
          return _remote_completion;
        }

        inline void enableRemoteCompletion ()
        {
          _remote_completion = true;
        }

        inline size_t getSequenceId ()
        {
          return _sequence_id;
        }

        inline void setSequenceId (size_t sequence)
        {
          _sequence_id = sequence;
        }


      protected:
        CM_Callback_t   _cb;
        struct iovec    * _iov;
        size_t            _tiov;
        size_t            _niov;
        size_t            _nbytes;

        uint8_t _dispatch_id;
        size_t  _sequence_id;
        bool    _remote_completion;

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
