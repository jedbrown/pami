/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShaddrMcstMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_ShaddrMcstMessage_h__
#define __components_devices_shmemcoll_ShaddrMcstMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
#include "components/devices/shmemcoll/McstMessage.h"
#include "opt_copy_a2.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

      template <class T_Device, class T_Desc>
      class McstMessageShaddr : public McstMessage<T_Device, T_Desc>
      {
        protected:
          // invoked by the thread object
          /// \see SendQueue::Message::_work
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            McstMessageShaddr * msg = (McstMessageShaddr *) cookie;
            return msg->advance();
          };


          inline pami_result_t advance ()
          {
			TRACE_ERR((stderr, "in Mcst advance\n"));

			T_Desc* _my_desc = this->_my_desc;
            T_Desc* _master_desc = this->_master_desc;

			//unsigned _my_index = __global.topology_local.rank2Index(__global.mapping.task());
			unsigned master = _my_desc->get_master();

			void* mybuf;
			void* buf = (void*) _master_desc->get_buffer(master);
			TRACE_ERR((stderr, "buf from master:%d address:%p\n", master, buf));

			//PAMI::PipeWorkQueue *rcv = _my_desc->get_recv_pwq();
			pami_multicast_t & mcast_params = _my_desc->get_mcast_params();
            PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue*) mcast_params.dst;

			unsigned bytes;

			/*if (_my_index == master){
				while (_master_desc->in_use()){};
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
				void* global_vaddr;
				memcpy(&global_vaddr, buf, sizeof(global_vaddr));
				TRACE_ERR((stderr,"global vaddr of master:%p\n", global_vaddr));
				//memcpy(mybuf, global_vaddr, bytes);
				opt_bgq_memcpy(mybuf, global_vaddr, bytes);
				rcv->produceBytes(bytes);
				TRACE_ERR((stderr, "produced bytes:%u to:%p\n", bytes, mybuf));
				_master_desc->signal_done();
				this->setStatus (PAMI::Device::Done);
				return PAMI_SUCCESS;
			}

			while (_master_desc->in_use()){};
			this->setStatus (PAMI::Device::Done);
			return PAMI_SUCCESS;
          }


        public:
          inline McstMessageShaddr (T_Device *device, T_Desc* desc, T_Desc* master_desc) :
             McstMessage<T_Device, T_Desc> (device, desc, master_desc, McstMessageShaddr::__advance, this)
          {
            TRACE_ERR((stderr, "<> McstMessageShaddr::McstMessageShaddr()\n"));
          };

      };  // PAMI::Device::McstMessageShaddr class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemMcstMessageShaddr_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
