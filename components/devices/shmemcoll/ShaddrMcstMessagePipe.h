/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemMcstMessageShaddr.h
 * \brief ???
 */

#ifndef __components_devices_shmem_McstMessageShaddr_h__
#define __components_devices_shmem_McstMessageShaddr_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
#include "components/devices/shmemcoll/McstMessage.h"
#include "opt_copy_a2.h"

#ifndef TRACE_ERR
#define TRACE_ERR(x)  fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
	  struct McstControl
	  {
		void* glob_src_buffer;
		volatile uint16_t	incoming_bytes;	
	  }__attribute__((__aligned__(128)));

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
			
			T_Desc* _my_desc = this->_my_desc;
            T_Desc* _master_desc = this->_master_desc;

			unsigned _my_index = __global.topology_local.rank2Index(__global.mapping.task());
			unsigned master = _my_desc->get_master();
				
			void* mybuf;

			Shmem::McstControl* mcst_control = (Shmem::McstControl*) _master_desc->get_buffer();
			assert(mcst_control != NULL);

			//PAMI::PipeWorkQueue *rcv = _my_desc->get_recv_pwq();
			pami_multicast_t & mcast_params = _my_desc->get_mcast_params();
            PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue*) mcast_params.dst;
            PAMI::PipeWorkQueue *src = (PAMI::PipeWorkQueue*) mcast_params.src;

			unsigned bytes = mcast_params.bytes; 

			if (_my_index == master){
				if (this->_bytes_consumed == bytes)
				{
					if (_master_desc->in_use()) return PAMI_EAGAIN; //wait for everyone to signal done
					TRACE_ERR((stderr,"everyone arrived calling done\n"));
					this->setStatus (PAMI::Device::Done);
					return PAMI_SUCCESS;

				}
				size_t bytes_to_consume = src->bytesAvailableToConsume() - this->_bytes_consumed;
				if (bytes_to_consume > 0) 
				{
					TRACE_ERR((stderr,"bytes_to_consume:%zd\n", bytes_to_consume));
					mcst_control->incoming_bytes += bytes_to_consume;
					this->_bytes_consumed+= bytes_to_consume;
					TRACE_ERR((stderr,"_bytes_consumed:%zd bytes:%u\n", bytes_to_consume, bytes));
					return PAMI_EAGAIN;
				}
			}
			else
			{
			  	if (this->_master_desc->get_state() != INIT){
                    TRACE_ERR((stderr,"matched desc is not in INIT state\n"));
                     return PAMI_EAGAIN;
                }
				if (this->_bytes_consumed == bytes)
				{
					_master_desc->signal_done();
					this->setStatus (PAMI::Device::Done);
					return PAMI_SUCCESS;

				}
				TRACE_ERR((stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed));
				if (mcst_control->incoming_bytes > this->_bytes_consumed)
				{
					TRACE_ERR((stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed));
					size_t	bytes_to_copy = mcst_control->incoming_bytes - this->_bytes_consumed;
					mybuf = rcv->bufferToProduce();
					void*	glob_src_buf = mcst_control->glob_src_buffer;
					opt_bgq_memcpy((void*)((char*)mybuf+this->_bytes_consumed), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
					//memcpy((void*)((char*)mybuf+this->_bytes_consumed), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
					this->_bytes_consumed+= bytes_to_copy;
					rcv->produceBytes(bytes_to_copy);

					return PAMI_EAGAIN;
				}
			}
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
