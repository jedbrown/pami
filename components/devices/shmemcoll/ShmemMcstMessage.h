/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShmemMcstMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShmemMcstMessage_h__
#define __components_devices_shmemcoll_ShmemMcstMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
//#include "components/devices/shmem/ShmemMessage.h"
#include "components/devices/shmemcoll/McstMessage.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

      template <class T_Device, class T_Desc>
      class McstMessageShmem : public McstMessage<T_Device, T_Desc>
      {
		public:

		  static inline pami_result_t short_msg_advance(T_Desc* master_desc, pami_multicast_t* mcast_params, unsigned master)
          {
			TRACE_ERR((stderr, "in Mcst advance\n"));

			void* mybuf;
			void* buf = (void*)master_desc->get_buffer(master);
			TRACE_ERR((stderr, "buf from master:%d address:%p\n", master, buf));

			PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue*) mcast_params->dst;
			unsigned bytes;

			 if (((PAMI::Topology*)mcast_params->dst_participants)->isRankMember(__global.mapping.task()))
			{
				TRACE_ERR((stderr,"I am dst participant\n" ));
				while (master_desc->get_flag()==0){};
				TRACE_ERR((stderr,"root has arrived\n" ));
				bytes = mcast_params->bytes;
				mybuf = rcv->bufferToProduce();
				memcpy(mybuf, buf, bytes);
				rcv->produceBytes(bytes);
				TRACE_ERR((stderr, "produced bytes:%u to:%p\n", bytes, mybuf));
				master_desc->signal_done();
			}

			return PAMI_SUCCESS;
          }

        protected:
          // invoked by the thread object
          /// \see SendQueue::Message::_work
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            McstMessageShmem * msg = (McstMessageShmem *) cookie;
            return msg->advance();
          };

          inline pami_result_t advance ()
          {
			TRACE_ERR((stderr, "in Mcst advance\n"));
			//unsigned _my_index = __global.topology_local.rank2Index(__global.mapping.task());
			unsigned master = this->_my_desc->get_master();

			void* mybuf;
			void* buf = (void*) this->_master_desc->get_buffer(master);
			TRACE_ERR((stderr, "buf from master:%d address:%p\n", master, buf));

			pami_multicast_t & mcast_params = this->_my_desc->get_mcast_params();

			PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue*) mcast_params.dst;
			unsigned bytes;

			/*if (_my_index == master){
				this->setStatus (PAMI::Device::Done);
				return PAMI_SUCCESS;
			}
			else{*/
			 if (((PAMI::Topology*)mcast_params.dst_participants)->isRankMember(__global.mapping.task()))
			 {
				if (this->_master_desc->get_state() != INIT){
					TRACE_ERR((stderr,"matched desc is not in INIT state\n"));
					 return PAMI_EAGAIN;
				}
				bytes = mcast_params.bytes;
				mybuf = rcv->bufferToProduce();
				memcpy(mybuf, buf, bytes);
				rcv->produceBytes(bytes);
				TRACE_ERR((stderr, "produced bytes:%u to:%p\n", bytes, mybuf));
				this->_master_desc->signal_done();
				/*this->setStatus (PAMI::Device::Done);
				return PAMI_SUCCESS;*/
			}

			this->setStatus (PAMI::Device::Done);
			return PAMI_SUCCESS;

            TRACE_ERR((stderr, "<< McstMessageShmem::advance(), return PAMI_EAGAIN\n"));
            //return PAMI_EAGAIN;
          }


        public:
          inline McstMessageShmem (T_Device *device, T_Desc* desc, T_Desc* master_desc) :
              McstMessage <T_Device, T_Desc> (device, desc, master_desc, McstMessageShmem::__advance, this)

          {
            TRACE_ERR((stderr, "<> McstMessageShmem::McstMessageShmem()\n"));
          };

      };  // PAMI::Device::McstMessageShmem class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemMcstMessageShmem_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
