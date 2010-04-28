/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemWork.h
 * \brief ???
 */

#ifndef __components_devices_shmem_ShmemWork_h__
#define __components_devices_shmem_ShmemWork_h__

#include <sys/uio.h>

#include <pami.h>

#include "util/common.h"
#include "components/devices/generic/AdvanceThread.h"
#include "components/devices/generic/Message.h"

#undef TRACE_ERR
#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
      class Work : public PAMI::Device::Generic::GenericThread
      {
        public:
        
          inline Work (pami_work_function   work_func,
                       void               * work_cookie) :
            PAMI::Device::Generic::GenericThread (work_func, work_cookie)
          {
          }
      };  // PAMI::Device::Shmem::Work class
      
      template <class T_Device>
      class RecPacketWork : public Work
      {
        protected:

          static pami_result_t __advance_with_callback (pami_context_t context, void * cookie)
          {
            RecPacketWork * work = (RecPacketWork *) cookie;
            return work->advance_with_callback (context);
          };

          inline pami_result_t advance_with_callback (pami_context_t context)
          {
            if (_sequence <= _device->lastRecSequenceId(_fnum))
            {
              // Invoke the work completion callback
              _done_fn (context, _done_cookie, PAMI_SUCCESS);

              // return 'success' which will remove the work object from the work queue.
              return PAMI_SUCCESS;
            }

            return PAMI_EAGAIN;
          };

          static pami_result_t __advance_with_status (pami_context_t context, void * cookie)
          {
            RecPacketWork * work = (RecPacketWork *) cookie;
            return work->advance_with_status (context);
          };

          inline pami_result_t advance_with_status (pami_context_t context)
          {
            if (_sequence <= _device->lastRecSequenceId(_fnum))
            {
              // Set the associated message status to "done". This will remove
              // the message from the head of the message queue and invoke any
              // callback
              _msg->setStatus (PAMI::Device::Done);

              // Return 'success' which will remove the work object from the work queue.
              return PAMI_SUCCESS;
            }

            return PAMI_EAGAIN;
          };

        
        public:
        
          inline RecPacketWork (T_Device            * device,
                                size_t                sequence,
                                size_t                fnum,
                                pami_event_function   fn,
                                void                * cookie) :
            Work (__advance_with_callback, this),
            _device (device),
            _sequence (sequence),
            _fnum (fnum),
            _done_fn (fn),
            _done_cookie (cookie)
          {
          }
        
          inline RecPacketWork (T_Device                * device,
                                size_t                    sequence,
                                size_t                    fnum,
                                Generic::GenericMessage * msg) :
            Work (__advance_with_status, this),
            _device (device),
            _sequence (sequence),
            _fnum (fnum),
            _msg (msg)
          {
          }
          
          inline void setSequenceId (size_t sequence)
          {
            _sequence = sequence;
          }
          
        protected:
        
          T_Device                * _device;
          size_t                    _sequence;
          size_t                    _fnum;
          pami_event_function       _done_fn;
          void                    * _done_cookie;
          Generic::GenericMessage * _msg;
      };  // PAMI::Device::Shmem::RecPacketWork class

    };    // PAMI::Device::Shmem namespace
  };      // PAMI::Device namespace
};        // PAMI namespace
#undef TRACE_ERR
#endif // __components_devices_shmem_ShmemWork_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
