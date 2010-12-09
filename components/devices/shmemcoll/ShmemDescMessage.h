/**
 * \file components/devices/shmemcoll/ShmemDescMessage.h
 * \brief ???
 */
#ifndef __components_devices_shmemcoll_ShmemDescMessage_h__
#define __components_devices_shmemcoll_ShmemDescMessage_h__

#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

#include "components/devices/shmemcoll/ShmemMcstMessage.h"
#include "components/devices/shmemcoll/ShaddrMcstMessage.h"

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

      template <class T_Device, class T_Desc>
      class ShmemDescMessage : public SendQueue::Message
      {

        protected:
          // invoked by the thread object
          /// \see SendQueue::Message::_work
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            ShmemDescMessage * msg = (ShmemDescMessage *) cookie;
            return msg->advance();
          };

          inline pami_result_t advance ()
          {

            pami_result_t res = _device->postDescriptor(desc);

            if (res == PAMI_SUCCESS)
              {
                this->setStatus (PAMI::Device::Done);
                return PAMI_SUCCESS;
              }

            return PAMI_EAGAIN;

          }

        public:
          inline ShmemDescMessage (T_Device *device, pami_event_function done_fn, void *done_cookie) :
              SendQueue::Message (ShmemDescMessage::__advance, this, done_fn, done_cookie, device->getContextOffset()),
              _device (device)
          {}

          inline T_Desc & return_descriptor(void) {return desc;};

          T_Desc desc;
          T_Device* _device;

      };

      template <class T_Device, class T_Desc>
      class ShmemDescMessageWorld : public SendQueue::Message
      {

        protected:
          // invoked by the thread object
          /// \see SendQueue::Message::_work
          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            ShmemDescMessageWorld * msg = (ShmemDescMessageWorld *) cookie;
            return msg->advance();
          };

          inline pami_result_t advance ()
          {

            T_Desc * my_desc, * master_desc;

            //TRACE_ERR((stderr, "fetching descriptor of myself and master:%u\n", desc.get_master()));
            pami_result_t res = _device->getShmemWorldDesc(&my_desc, &master_desc, desc.get_master());

            if (res == PAMI_SUCCESS)
              {

                if (desc.get_type() == Shmem::MULTICAST)
                  {

                    pami_multicast_t & mcast = desc.get_mcast_params();
                    my_desc->set_mcast_params(&mcast);
                    my_desc->set_master(desc.get_master());
                    my_desc->set_dispatch_id(desc.get_dispatch_id());
                    my_desc->set_storage((void*)desc.get_storage());

                    //TRACE_ERR((stderr,"master:%u local_task:%u\n", desc.get_master(), _peer));
                    //short multicast and if master, copy the data inline into the descriptor
                    if (desc.get_master() == _peer)
                      {
                        void* buf = (void*) my_desc->get_buffer(desc.get_master());
                        void* mybuf = ((PAMI::PipeWorkQueue*)mcast.src)->bufferToConsume();
                        memcpy(buf, mybuf, mcast.bytes);
                        //TRACE_ERR((stderr,"copied bytes:%u from %p to %p data[0]:%u\n", desc.get_bytes(), mybuf, buf, ((unsigned*)buf)[0]));
                        ((PAMI::PipeWorkQueue*)mcast.src)->consumeBytes(mcast.bytes);
                      }

                    my_desc->set_state(Shmem::INIT);

                    if ( mcast.bytes <= Shmem::McstMessage<T_Device, T_Desc>::short_msg_cutoff )
                      {
                        Shmem::McstMessage<T_Device, T_Desc> * obj = (Shmem::McstMessage<T_Device, T_Desc> *) (my_desc->get_storage());
                        new (obj) Shmem::McstMessageShmem<T_Device, T_Desc> (_device, my_desc, master_desc);
                        _device->post(obj);
                      }
                    else
                      {
                        Shmem::McstMessageShaddr<T_Device, T_Desc> * obj = (Shmem::McstMessageShaddr<T_Device, T_Desc> *) (my_desc->get_storage());
                        new (obj) Shmem::McstMessageShaddr<T_Device, T_Desc> (_device, my_desc, master_desc);
                        _device->post(obj);
                      }

                    /* To do..signal arrived */

                    this->setStatus (PAMI::Device::Done);
                    return PAMI_SUCCESS;
                  }
              }

            return PAMI_EAGAIN;

          }

        public:
          inline ShmemDescMessageWorld (T_Device *device, pami_event_function done_fn, void *done_cookie, unsigned peer) :
              SendQueue::Message (ShmemDescMessageWorld::__advance, this, done_fn, done_cookie, device->getContextOffset()),
              _device (device), _peer (peer)
          {}

          inline T_Desc & return_descriptor(void) {return desc;};

          T_Desc desc;
          T_Device* _device;
          unsigned _peer;

      };
    }
  }
}


#endif
