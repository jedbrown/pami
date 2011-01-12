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

#ifndef __components_devices_shmemcoll_msgs_ShortMcstMessage_h__
#define __components_devices_shmemcoll_msgs_ShortMcstMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
#include "BaseMessage.h"

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
        class ShortMcstMessage: public BaseMessage<T_Device, T_Desc> 
      {
        public:

          static inline pami_result_t short_msg_advance(T_Desc* my_desc, pami_multicast_t* mcast_params, unsigned master, unsigned local_rank)
          {
            void* mybuf;
            void* buf = (void*)my_desc->get_buffer();
            TRACE_ERR((stderr, "buf from master:%d address:%p\n", master, buf));

            PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue*) mcast_params->dst;
            unsigned bytes;

            /*if (((PAMI::Topology*)mcast_params->dst_participants)->isRankMember(__global.mapping.task()) && 
              (mcast_params->src_participants == NULL ||
              !((PAMI::Topology*)mcast_params->src_participants)->isRankMember(__global.mapping.task())))*/
            if (((PAMI::Topology*)mcast_params->dst_participants)->isRankMember(__global.mapping.task()) && (master != local_rank))
            {
              TRACE_ERR((stderr, "I am dst participant\n" ));
              while (my_desc->get_flag() == 0) {};
              TRACE_ERR((stderr, "root has arrived\n" ));
              bytes = mcast_params->bytes;
              mybuf = rcv->bufferToProduce();
              memcpy(mybuf, buf, bytes);
              rcv->produceBytes(bytes);
              TRACE_ERR((stderr, "produced bytes:%u to:%p\n", bytes, mybuf));
            }

            return PAMI_SUCCESS;
          }

        //protected:
        public:
          // invoked by the thread object
          /// \see SendQueue::Message::_work
          static  pami_result_t __advance (pami_context_t context, void * cookie)
          {
            ShortMcstMessage * msg = (ShortMcstMessage *) cookie;
            return msg->advance();
          };

          inline virtual pami_result_t advance ()
          {
            unsigned master = this->_my_desc->get_master();
            void* mybuf;
            void* buf = (void*) this->_my_desc->get_buffer();
            TRACE_ERR((stderr, "buf from master:%d address:%p\n", master, buf));

            pami_multicast_t & mcast_params = this->_my_desc->get_mcast_params();
            PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue*) mcast_params.dst;
            PAMI::PipeWorkQueue *src = (PAMI::PipeWorkQueue*) mcast_params.src;
            unsigned bytes;
            bytes = mcast_params.bytes;

            if (this->_local_rank == master){
              if ((bytes > 0) && (src->bytesAvailableToConsume() == 0))  
                return PAMI_EAGAIN; 

              TRACE_ERR((stderr,"root waiting for the data to arrive bytes available to consume:%zd\n",src->bytesAvailableToConsume()));
              mybuf = ((PAMI::PipeWorkQueue *)mcast_params.src)->bufferToConsume();
              memcpy(buf, mybuf, bytes);
              this->_my_desc->signal_flag();

              this->_my_desc->signal_done();
            }
            else
            {
              if (this->_my_desc->get_flag() == 0) 
                return PAMI_EAGAIN;
              //while (this->_my_desc->get_flag() == 0) {}

              mybuf = rcv->bufferToProduce();
              memcpy(mybuf, buf, bytes);
              rcv->produceBytes(bytes);
              TRACE_ERR((stderr, "produced bytes:%u to:%p\n", bytes, mybuf));
              this->_my_desc->signal_done();
            }

            this->_my_desc->set_my_state(Shmem::DONE);
            mcast_params.cb_done.function(this->_context, mcast_params.cb_done.clientdata, PAMI_SUCCESS);
            return PAMI_SUCCESS;
          }


        public:
          inline ShortMcstMessage (pami_context_t context, T_Desc* my_desc, unsigned local_rank) : 
            BaseMessage<T_Device, T_Desc>(context, my_desc,ShortMcstMessage::__advance,(void*)this,local_rank)
        {
          TRACE_ERR((stderr, "<> ShortMcstMessage::ShortMcstMessage()\n"));
        };


      };  // PAMI::Device::ShortMcstMessage class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemShortMcstMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
