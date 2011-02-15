/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/msgs/BaseMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_msgs_BaseMessage_h__
#define __components_devices_shmemcoll_msgs_BaseMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
//#include "opt_copy_a2.h"
#include "math/Memcpy.x.h"

#undef TRACE_ERR

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf(stderr,"%s:%d\n",__FILE__,__LINE__); fprintf x
#endif
//#define COPY_BY_CHUNKS

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {


      template <class T_Device>
        class BaseMessage : public PAMI::Queue::Element
      {
        public:
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            BaseMessage * msg = (BaseMessage *) cookie;
            return msg->advance();
          };

        protected:
          virtual pami_result_t advance() = 0;

        public:
          inline BaseMessage (): PAMI::Queue::Element() 

        {
          TRACE_ERR((stderr, "<> BaseMessage::BaseMessage()\n"));
        };

          inline BaseMessage (pami_context_t context, typename T_Device::CollectiveFifo::Descriptor* my_desc, pami_work_function work_fn, void* cookie, unsigned local_rank) : PAMI::Queue::Element(),
_local_rank(local_rank), _bytes_consumed(0), _context(context), _my_desc(my_desc), _work(work_fn, cookie)

        {
          TRACE_ERR((stderr, "<> BaseMessage::BaseMessage()\n"));
        };

          virtual ~BaseMessage() {};

          unsigned                            _local_rank;
          unsigned                            _bytes_consumed;          
          pami_context_t                      _context;
          typename T_Device::CollectiveFifo::Descriptor     *_my_desc;
          PAMI::Device::Generic::GenericThread _work;

      };  // PAMI::Device::BaseMessage class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_BaseMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
