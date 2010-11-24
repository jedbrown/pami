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
#include "math/Memcpy.x.h"

#undef TRACE_ERR

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif
//#define COPY_BY_CHUNKS

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
#define MCST_CHUNK_SIZE	8192
	  /*struct McstControl
	  {
		void* glob_src_buffer;
		volatile uint16_t	incoming_bytes;	
	  }__attribute__((__aligned__(128)));*/
	  struct McstControl
	  {
		volatile void* glob_src_buffer;
		volatile uint32_t	incoming_bytes;	
	  };

      template <class T_Device, class T_Desc>
      class McstMessageShaddr : public McstMessage<T_Device, T_Desc>
      {
		public:
          static pami_result_t advanceMcstShaddr (pami_context_t context, void * cookie)
          {
            McstMessageShaddr * msg = (McstMessageShaddr *) cookie;
            return msg->__advanceMcstShaddr();
          };
        protected:
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            McstMessageShaddr * msg = (McstMessageShaddr *) cookie;
            //return msg->advance();
            return msg->__advanceMcstShaddr();
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
				  //fprintf(stderr,"bytes_to_consume:%zd\n", bytes_to_consume);
					mcst_control->incoming_bytes += bytes_to_consume;
					this->_bytes_consumed+= bytes_to_consume;
					TRACE_ERR((stderr,"_bytes_consumed:%zd bytes:%u\n", bytes_to_consume, bytes));
					return PAMI_EAGAIN;
				}
			}
			else
			{
			  	if (this->_master_desc->get_state() != INIT){
                     return PAMI_EAGAIN;
                }
				else
				{
					TRACE_ERR((stderr,"glob va address :%p\n", mcst_control->glob_src_buffer));
				}

				if (this->_bytes_consumed == bytes)
				{
					TRACE_ERR((stderr,"receiver consumed all bytes signalling done\n"));
					_master_desc->signal_done();
					this->setStatus (PAMI::Device::Done);
					return PAMI_SUCCESS;

				}
				//TRACE_ERR((stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed));
				if (mcst_control->incoming_bytes > this->_bytes_consumed)
				{
				  fprintf(stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed);
					size_t	bytes_to_copy = mcst_control->incoming_bytes - this->_bytes_consumed;
					mybuf = rcv->bufferToProduce();
					void*	glob_src_buf = mcst_control->glob_src_buffer;
					opt_bgq_memcpy((void*)((char*)mybuf+this->_bytes_consumed), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
					//memcpy((void*)((char*)mybuf+this->_bytes_consumed), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
					this->_bytes_consumed+= bytes_to_copy;
					TRACE_ERR((stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed));
					rcv->produceBytes(bytes_to_copy);

				}
				return PAMI_EAGAIN;
			}
			return PAMI_SUCCESS;
          }

          inline pami_result_t __advanceMcstShaddr ()
          {
			
			T_Desc* _my_desc = this->_my_desc;
            T_Desc* _master_desc = this->_master_desc;

			unsigned master = _my_desc->get_master();
				
			void* mybuf;

			Shmem::McstControl* mcst_control = (Shmem::McstControl*) _master_desc->get_buffer();
			assert(mcst_control != NULL);

			pami_multicast_t & mcast_params = _my_desc->get_mcast_params();
            PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue*) mcast_params.dst;
            PAMI::PipeWorkQueue *src = (PAMI::PipeWorkQueue*) mcast_params.src;
			//fprintf(stderr,"src pwq:%p\n", src);

			unsigned bytes = mcast_params.bytes; 
			//fprintf(stderr,"calling advanceMcstShaddr\n");

			if (master){
				TRACE_ERR((stderr,"bytes consumed:%zd bytesAvailableToConsume:%zd\n", this->_bytes_consumed,src->bytesAvailableToConsume()));
				if (this->_bytes_consumed == bytes)
				{
					if (_master_desc->in_use()) return PAMI_EAGAIN; //wait for everyone to signal done
					TRACE_ERR((stderr,"everyone arrived calling done\n"));
					_master_desc->reset();
					mcst_control->incoming_bytes = 0;
					mcst_control->glob_src_buffer = NULL;
					_master_desc->set_state(FREE);
					//_master_desc->signal_master_done();
					mem_barrier();
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
			  	if ((this->_master_desc->get_state() != INIT) && (this->seq_num != this->_master_desc->get_seq_id())){
                     return PAMI_EAGAIN;
                }
				else
				{
					//TRACE_ERR((stderr,"glob va address :%p\n", mcst_control->glob_src_buffer));
					//fprintf(stderr,"seq_num:%lu\n", this->_master_desc->get_seq_id());
				}

				if (this->_bytes_consumed == bytes)
				{
					_master_desc->signal_done();
					//while (_master_desc->get_master_done() == 0) {}; //wait for the master to be done
					this->setStatus (PAMI::Device::Done);

					return PAMI_SUCCESS;
				}
				TRACE_ERR((stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed));
				if (mcst_control->incoming_bytes > this->_bytes_consumed)
				{
				        mem_sync();
					TRACE_ERR((stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed));
					size_t	bytes_to_copy = mcst_control->incoming_bytes - this->_bytes_consumed;
					mybuf = rcv->bufferToProduce();
					volatile void*	glob_src_buf = mcst_control->glob_src_buffer;
					TRACE_ERR((stderr,"copying from gva:%p bytes_consumed:%zd incoming_bytes:%d bytes_to_copy:%zd\n", glob_src_buf,this->_bytes_consumed, mcst_control->incoming_bytes, bytes_to_copy));
					assert(glob_src_buf != NULL);

#ifdef COPY_BY_CHUNKS
					/* check if bytes_to_copy is a multiple of CHUNK_SIZE */
					if (bytes - this->_bytes_consumed >= MCST_CHUNK_SIZE)
					{
						size_t chunks_to_copy = bytes_to_copy/MCST_CHUNK_SIZE;
						if (chunks_to_copy != 0) //atleast one chunk received
						{
							opt_bgq_memcpy((void*)((char*)mybuf), (void*)((char*)glob_src_buf+this->_bytes_consumed), 
											chunks_to_copy*MCST_CHUNK_SIZE);
							this->_bytes_consumed+= chunks_to_copy*MCST_CHUNK_SIZE;
							rcv->produceBytes(chunks_to_copy*MCST_CHUNK_SIZE);
						}
					}
					else
					{
						opt_bgq_memcpy((void*)((char*)mybuf), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
					//memcpy((void*)((char*)mybuf), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
						this->_bytes_consumed+= bytes_to_copy;
						TRACE_ERR((stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed));
						rcv->produceBytes(bytes_to_copy);
					}

#else
					Core_memcpy((void*)((char*)mybuf), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
					//opt_bgq_memcpy((void*)((char*)mybuf), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
					//memcpy((void*)((char*)mybuf), (void*)((char*)glob_src_buf+this->_bytes_consumed), bytes_to_copy);
					this->_bytes_consumed+= bytes_to_copy;
					TRACE_ERR((stderr,"incoming_bytes:%d, _bytes_consumed:%zd\n", mcst_control->incoming_bytes, this->_bytes_consumed));
					rcv->produceBytes(bytes_to_copy);
#endif

				}
				return PAMI_EAGAIN;
			}
			return PAMI_EAGAIN;
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
