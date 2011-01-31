/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/msgs/ShortMcombMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_msgs_ShortMcombMessage_h__
#define __components_devices_shmemcoll_msgs_ShortMcombMessage_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
#include "assert.h"
#include "../a2qpx_nway_sum.h"

#undef TRACE_ERR

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf(stderr,"%s:%d\n",__FILE__,__LINE__); fprintf x
#endif

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

#define SHMEMBUF(x)	((double*) _my_desc->get_buffer(x))
#define SHMEMBUF_SHORT(x)	((double*) my_desc->get_buffer(x))

      template <class T_Device, class T_Desc>
        class ShortMcombMessage: public BaseMessage<T_Device, T_Desc>
        {
          public:
            //currently optimized to a many-to-one combine
            static inline pami_result_t short_msg_advance(T_Desc* my_desc, pami_multicombine_t* mcomb_params,
                unsigned npeers, unsigned local_rank, unsigned task)
            {

              if (local_rank == 0)
              {

                size_t num_src_ranks = ((PAMI::Topology*)mcomb_params->data_participants)->size();
                while (my_desc->arrived_peers() != (unsigned)num_src_ranks) {};
                TRACE_ERR((stderr, "all peers:%zu arrived, starting the blocking Shmem Mcomb protocol\n", num_src_ranks));
                PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue *)mcomb_params->results;
                size_t bytes = mcomb_params->count << pami_dt_shift[mcomb_params->dtype];
                double* dst = (double*)(rcv->bufferToConsume());

                if (npeers == 4)
                {

                  for (unsigned i = 0; i < mcomb_params->count; i++)
                    SHMEMBUF_SHORT(0)[i] = SHMEMBUF_SHORT(0)[i] + SHMEMBUF_SHORT(1)[i] + SHMEMBUF_SHORT(2)[i] + SHMEMBUF_SHORT(3)[i];

                }
                else if (npeers == 8)
                {
                  for (unsigned i = 0; i < mcomb_params->count; i++)
                    SHMEMBUF_SHORT(0)[i] = SHMEMBUF_SHORT(0)[i] + SHMEMBUF_SHORT(1)[i] + SHMEMBUF_SHORT(2)[i] + SHMEMBUF_SHORT(3)[i] +
                      SHMEMBUF_SHORT(4)[i] + SHMEMBUF_SHORT(5)[i] + SHMEMBUF_SHORT(6)[i] + SHMEMBUF_SHORT(7)[i] ;
                  
    

                }
                else if (npeers == 16)
                {
                  for (unsigned i = 0; i < mcomb_params->count; i++)
                    SHMEMBUF_SHORT(0)[i] = SHMEMBUF_SHORT(0)[i] + SHMEMBUF_SHORT(1)[i] + SHMEMBUF_SHORT(2)[i] + SHMEMBUF_SHORT(3)[i] +
                      SHMEMBUF_SHORT(4)[i] + SHMEMBUF_SHORT(5)[i] + SHMEMBUF_SHORT(6)[i] + SHMEMBUF_SHORT(7)[i] +
                      SHMEMBUF_SHORT(8)[i] + SHMEMBUF_SHORT(9)[i] + SHMEMBUF_SHORT(10)[i] + SHMEMBUF_SHORT(11)[i] +
                      SHMEMBUF_SHORT(12)[i] + SHMEMBUF_SHORT(13)[i] + SHMEMBUF_SHORT(14)[i] + SHMEMBUF_SHORT(15)[i];
                }
                else
                {
                  fprintf(stderr, "sum not yet supported\n");
                }

                char* src = (char*) my_desc->get_buffer(0);
                char* my_dst = (char*)(dst);
                memcpy((void*)my_dst, (void*)src, bytes);
                rcv->produceBytes(bytes);
                TRACE_ERR((stderr, "Finished gathering results, signalling done\n"));
              }
                my_desc->signal_done();

              return PAMI_SUCCESS;
            }

          //protected:
            public:
            // invoked by the thread object
            /// \see SendQueue::Message::_work
            static pami_result_t __advance (pami_context_t context, void * cookie)
            {
              ShortMcombMessage * msg = (ShortMcombMessage *) cookie;
              return msg->advance();
            }

            //This advance is for reduction to root locally in a node
            inline virtual pami_result_t advance ()
            {

              T_Desc* _my_desc = this->_my_desc;
              pami_multicombine_t & mcomb_params = _my_desc->get_mcomb_params();
              size_t num_src_ranks = ((PAMI::Topology*)mcomb_params.data_participants)->size();

              //if (_my_desc->arrived_peers() != (unsigned) num_src_ranks)
              //  return PAMI_EAGAIN;
              //while (_my_desc->arrived_peers() != (unsigned)num_src_ranks) {};

              unsigned _npeers = __global.topology_local.size();
              unsigned _task = __global.mapping.task();
              unsigned _local_rank = __global.topology_local.rank2Index(_task);

              /* Start the protocol here..blocking version since everyone arrived */
              PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue *)mcomb_params.results;
              size_t bytes = mcomb_params.count << pami_dt_shift[mcomb_params.dtype];
              double* dst = rcv?(double*)(rcv->bufferToConsume()):(double*)NULL;


              if (_local_rank == 0)
              {

                if (_my_desc->arrived_peers() != (unsigned) num_src_ranks)
                  return PAMI_EAGAIN;

                if (_npeers == 4)
                {

                  /*for (unsigned i = 0; i < mcomb_params.count; i++)
                    SHMEMBUF(0)[i] = SHMEMBUF(0)[i] + SHMEMBUF(1)[i] + SHMEMBUF(2)[i] + SHMEMBUF(3)[i];*/
                  quad_double_sum_4way(SHMEMBUF(0),  SHMEMBUF(0), SHMEMBUF(1), SHMEMBUF(2), SHMEMBUF(3), mcomb_params.count);

                }
                else if (_npeers == 8)
                {
                  /*for (unsigned i = 0; i < mcomb_params.count; i++)
                    SHMEMBUF(0)[i] = SHMEMBUF(0)[i] + SHMEMBUF(1)[i] + SHMEMBUF(2)[i] + SHMEMBUF(3)[i] +
                    SHMEMBUF(4)[i] + SHMEMBUF(5)[i] + SHMEMBUF(6)[i] + SHMEMBUF(7)[i] ;*/
                  quad_double_sum_8way(SHMEMBUF(0),  SHMEMBUF(0), SHMEMBUF(1), SHMEMBUF(2), SHMEMBUF(3),
                                        SHMEMBUF(4), SHMEMBUF(5), SHMEMBUF(6), SHMEMBUF(7), mcomb_params.count);

                }
                else if (_npeers == 16)
                {
                  /*for (unsigned i = 0; i < mcomb_params.count; i++)
                    SHMEMBUF(0)[i] = SHMEMBUF(0)[i] + SHMEMBUF(1)[i] + SHMEMBUF(2)[i] + SHMEMBUF(3)[i] +
                      SHMEMBUF(4)[i] + SHMEMBUF(5)[i] + SHMEMBUF(6)[i] + SHMEMBUF(7)[i] +
                      SHMEMBUF(8)[i] + SHMEMBUF(9)[i] + SHMEMBUF(10)[i] + SHMEMBUF(11)[i] +
                      SHMEMBUF(12)[i] + SHMEMBUF(13)[i] + SHMEMBUF(14)[i] + SHMEMBUF(15)[i];*/
                  quad_double_sum_16way(SHMEMBUF(0), SHMEMBUF(0), SHMEMBUF(1), SHMEMBUF(2), SHMEMBUF(3),
                      SHMEMBUF(4), SHMEMBUF(5), SHMEMBUF(6), SHMEMBUF(7), SHMEMBUF(8), SHMEMBUF(9), SHMEMBUF(10),
                      SHMEMBUF(11), SHMEMBUF(12), SHMEMBUF(13), SHMEMBUF(14), SHMEMBUF(15), mcomb_params.count);

                }
                else
                {
                  fprintf(stderr, "sum not yet supported\n");
                }

                _my_desc->signal_flag();
              }

              /* Reduction over...start gathering the results */
              if (((PAMI::Topology*)mcomb_params.results_participants)->isRankMember(_task))
              {
                PAMI_assert(dst);
                while (_my_desc->get_flag() == 0) {}; //wait till reduction is done
                char* src = (char*) _my_desc->get_buffer(0);
                char* my_dst = (char*)(dst);
                memcpy((void*)my_dst, (void*)src, bytes);
                rcv->produceBytes(bytes);

                TRACE_ERR((stderr, "Finished gathering results, signalling done\n"));
              }

              _my_desc->signal_done();
              _my_desc->set_my_state(Shmem::DONE);
              mcomb_params.cb_done.function(this->_context, mcomb_params.cb_done.clientdata, PAMI_SUCCESS);
              return PAMI_SUCCESS;

            }

          public:
            inline ShortMcombMessage (pami_context_t context, T_Desc* desc, unsigned local_rank):
            BaseMessage<T_Device, T_Desc>(context, desc, ShortMcombMessage::__advance, (void*)this, local_rank)

          {
            TRACE_ERR((stderr, "<> Shmem::ShortMcombMessage\n"));
          };

            /*pami_context_t                      _context;
            T_Desc                              *_my_desc;
            PAMI::Device::Generic::GenericThread _work;*/

        };  // PAMI::Device::McombMessageShmem class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShortMcombMessageShmem_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
