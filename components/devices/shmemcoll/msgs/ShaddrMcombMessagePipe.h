/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmemcoll/ShaddrMcombMessagePipe.h
 * \brief ???
 */

#ifndef __components_devices_shmemcoll_msgs_ShaddrMcombMessagePipe_h__
#define __components_devices_shmemcoll_msgs_ShaddrMcombMessagePipe_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
//#include "quad_sum.h"
//#include "16way_sum.h"
#include "../4way_sum.h"
#include "../2way_sum.h"
#include "../opt_copy_a2.h"
#include "../8way_sum.h"
#include "assert.h"
#include "BaseMessage.h"


#if 0
inline int quad_double_sum_8way(double* dest, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, uint64_t num )
{

  quad_double_sum_4way(dest, src0, src1, src2, src3,num);
  quad_double_sum_4way(dest, dest, src4, src5, src6, num);
  quad_double_sum_2way(dest, dest, src7, num);

  return 0;

}
#endif

#if 1
inline int quad_double_sum_16way(double* dest, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double* src9, double* src10,
    double* src11,  double* src12, double* src13, double* src14, double* src15, uint64_t num )
{

  quad_double_sum_8way(dest, src0, src1, src2, src3, src4, src5, src6, src7, num);
  quad_double_sum_8way(dest, dest, src8, src9, src10, src11, src12, src13, src14, num);
  quad_double_sum_2way(dest, dest, src15, num);

  return 0;
}
#endif

#if 0
inline int quad_double_sum_16way(double* dest, double* src0, double *src1, double* src2, double* src3,
    double* src4, double* src5, double* src6, double* src7, double* src8, double* src9, double* src10,
    double* src11,  double* src12, double* src13, double* src14, double* src15, uint64_t num )
{

  quad_double_sum_4way(dest, src0, src1, src2, src3,num);
  quad_double_sum_4way(dest, dest, src4, src5, src6, num);
  quad_double_sum_4way(dest, dest, src7, src8, src9, num);
  quad_double_sum_4way(dest, dest, src10, src11, src12, num);
  quad_double_sum_4way(dest, dest, src13, src14, src15, num);

  return 0;
}
#endif
#ifndef TRACE_ERR
#define TRACE_ERR(x)  //fprintf x
#endif

#ifndef PAMI_ASSERT
#define PAMI_ASSERT(x) PAMI_assert(x)
#endif


namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {

//#define	CHUNK_SIZE 	1024
//#define	CHUNK_SIZE	2048
#define	CHUNK_SIZE 	4096
//#define	CHUNK_SIZE 	8192
#define NUM_DBLS_PER_CHUNK (CHUNK_SIZE/sizeof(double))
#define NUM_CHUNKS(bytes)	((bytes%CHUNK_SIZE == 0) ? (bytes/CHUNK_SIZE):(bytes/CHUNK_SIZE + 1))

#define SHADDR_SRCBUF(x) 	((double*)(mcomb_control->GlobalAddressTable.src_bufs[x]))
#define SHADDR_DSTBUF(x) 	((double*)(mcomb_control->GlobalAddressTable.dst_bufs[x]))

#define ALIGNED_SRCBUF(x,y)	((((uint64_t)(mcomb_control->GlobalAddressTable.src_bufs[x])) & ((uint64_t)y)) == 0)
#define ALIGNED_DSTBUF(x,y)	((((uint64_t)(mcomb_control->GlobalAddressTable.dst_bufs[x])) & ((uint64_t)y)) == 0)

#define CURRENT_ITER	(mcomb_control->current_iter)
#define BLOCK_ITER	2

      struct McombControl
      {
        struct
        {
          void* src_bufs[NUM_LOCAL_TASKS];
          void* dst_bufs[NUM_LOCAL_TASKS];
        }GlobalAddressTable;// __attribute__((__aligned__(128)));
        volatile uint16_t	chunks_done[NUM_LOCAL_TASKS];
        volatile uint16_t	chunks_copied[NUM_LOCAL_TASKS];
        volatile uint16_t	current_iter;
      };

      template <class T_Device, class T_Desc>
        class ShaddrMcombMessagePipe : public BaseMessage<T_Device, T_Desc>
      {
        protected:

          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            ShaddrMcombMessagePipe * msg = (ShaddrMcombMessagePipe *) cookie;
            return msg->advance();
          };

          inline void advance_4way_sum(Shmem::McombControl* mcomb_control, unsigned _local_rank, unsigned _npeers,
              double* dst, size_t bytes)
          {
            /* local ranks other than 0 do the following quad sum */
            unsigned iter;

            for (iter=0; iter < NUM_CHUNKS(bytes)-1; iter++){
              if ((iter%(_npeers-1) +1) == _local_rank){

                quad_double_sum_4way(dst+ iter* NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(0)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(1)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(2)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(3)+iter*NUM_DBLS_PER_CHUNK, NUM_DBLS_PER_CHUNK);
                mcomb_control->chunks_done[_local_rank] += 1;
              }
              TRACE_ERR((stderr,"dst[%u]:%f\n", iter*NUM_DBLS_PER_CHUNK, dst[iter*NUM_DBLS_PER_CHUNK]));
            }

            /* summing last chunk which can be of any size */
            if ((iter%(_npeers-1) +1) == _local_rank){
              if (bytes%CHUNK_SIZE == 0)
              {
                quad_double_sum_4way(dst+ iter* NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(0)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(1)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(2)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(3)+iter*NUM_DBLS_PER_CHUNK, NUM_DBLS_PER_CHUNK);

              }
              else
              {
                for (unsigned i =0; i < (bytes%CHUNK_SIZE)/sizeof(double); i++)
                {
                  dst[i+iter*NUM_DBLS_PER_CHUNK] = SHADDR_SRCBUF(0)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(1)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(2)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(3)[i+iter*NUM_DBLS_PER_CHUNK];
                }
              }
              mcomb_control->chunks_done[_local_rank] += 1;
            }

          }

          inline void advance_8way_sum(Shmem::McombControl* mcomb_control, unsigned _local_rank, unsigned _npeers,
              double* dst, size_t bytes)
          {
            /* local ranks other than 0 do the following quad sum */
            unsigned iter;

            for (iter=0; iter < NUM_CHUNKS(bytes)-1; iter++){
              if ((iter%(_npeers-1) +1) == _local_rank){
                //unsigned long long t1 = PAMI_Wtimebase();

                quad_double_sum_8way(dst+ iter* NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(0)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(1)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(2)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(3)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(4)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(5)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(6)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(7)+iter*NUM_DBLS_PER_CHUNK, NUM_DBLS_PER_CHUNK);
                //unsigned long long t2 = PAMI_Wtimebase();
                // fprintf(stderr,"[%d]cycles:%lld, bytes:%zd\n", _local_rank,(t2-t1), NUM_DBLS_PER_CHUNK*sizeof(double));
                mcomb_control->chunks_done[_local_rank] += 1;
              }
            }

            /* summing last chunk which can be of any size */
            if ((iter%(_npeers-1) +1) == _local_rank){
              if (bytes%CHUNK_SIZE == 0)
              {
                quad_double_sum_8way(dst+ iter* NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(0)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(1)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(2)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(3)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(4)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(5)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(6)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(7)+iter*NUM_DBLS_PER_CHUNK, NUM_DBLS_PER_CHUNK);

              }
              else
              {
                for (unsigned i =0; i < (bytes%CHUNK_SIZE)/sizeof(double); i++)
                {
                  dst[i+iter*NUM_DBLS_PER_CHUNK] = SHADDR_SRCBUF(0)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(1)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(2)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(3)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(4)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(5)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(6)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(7)[i+iter*NUM_DBLS_PER_CHUNK];
                }
              }
              mbar();
              mcomb_control->chunks_done[_local_rank] += 1;
            }


          }

          inline void advance_16way_sum(Shmem::McombControl* mcomb_control, unsigned _local_rank, unsigned _npeers,
              double* dst, size_t bytes)
          {
            /* local ranks other than 0 do the following quad sum */
            unsigned iter;

            for (iter=0; iter < NUM_CHUNKS(bytes)-1; iter++){
              if ((iter%(_npeers-1) +1) == _local_rank){

                quad_double_sum_16way(dst+ iter* NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(0)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(1)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(2)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(3)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(4)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(5)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(6)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(7)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(8)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(9)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(10)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(11)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(12)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(13)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(14)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(15)+iter*NUM_DBLS_PER_CHUNK, NUM_DBLS_PER_CHUNK);
                mcomb_control->chunks_done[_local_rank] += 1;
              }
              TRACE_ERR((stderr,"dst[%u]:%f\n", iter*NUM_DBLS_PER_CHUNK, dst[iter*NUM_DBLS_PER_CHUNK]));
            }

            /* summing last chunk which can be of any size */
            if ((iter%(_npeers-1) +1) == _local_rank){
              if (bytes%CHUNK_SIZE == 0)
              {
                quad_double_sum_16way(dst+ iter* NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(0)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(1)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(2)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(3)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(4)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(5)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(6)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(7)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(8)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(9)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(10)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(11)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(12)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(13)+iter*NUM_DBLS_PER_CHUNK, SHADDR_SRCBUF(14)+iter*NUM_DBLS_PER_CHUNK,
                    SHADDR_SRCBUF(15)+iter*NUM_DBLS_PER_CHUNK, NUM_DBLS_PER_CHUNK);

              }
              else
              {
                for (unsigned i =0; i < (bytes%CHUNK_SIZE)/sizeof(double); i++)
                {
                  dst[i+iter*NUM_DBLS_PER_CHUNK] = SHADDR_SRCBUF(0)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(1)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(2)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(3)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(4)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(5)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(6)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(7)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(8)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(9)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(10)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(11)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(12)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(13)[i+iter*NUM_DBLS_PER_CHUNK] + SHADDR_SRCBUF(14)[i+iter*NUM_DBLS_PER_CHUNK] +
                    SHADDR_SRCBUF(15)[i+iter*NUM_DBLS_PER_CHUNK];
                }
              }
              mcomb_control->chunks_done[_local_rank] += 1;
            }

          }


          inline virtual pami_result_t advance ()
          {

            T_Desc* _my_desc = this->_my_desc;
            unsigned _npeers = __global.topology_local.size();

            unsigned _local_rank = this->_local_rank;

            pami_multicombine_t & mcomb_params = _my_desc->get_mcomb_params();
            size_t num_src_ranks = ((PAMI::Topology*)mcomb_params.data_participants)->size();

            /* Non blocking until all the peers arrive at the collective */
            if (_my_desc->arrived_peers() != (unsigned) num_src_ranks)
            {
              TRACE_ERR((stderr,"arrived_peers:%u waiting for:%u\n", _my_desc->arrived_peers(), (unsigned) num_src_ranks));
              return PAMI_EAGAIN;
            }
            TRACE_ERR((stderr, "all peers:%zu arrived, starting the blocking Mcomb protocol\n", num_src_ranks));

            /* Start the protocol here..blocking version since everyone arrived */

            TRACE_ERR((stderr,"size of desc:%ld sizeof mcomb_control:%ld", sizeof(T_Desc), sizeof(McombControl)));

            Shmem::McombControl* mcomb_control = (Shmem::McombControl*) _my_desc->get_buffer();
            assert(mcomb_control != NULL);

            PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue *)mcomb_params.results;
            double* dst = (double*)(rcv->bufferToConsume());
            size_t bytes = mcomb_params.count << pami_dt_shift[mcomb_params.dtype];

            TRACE_ERR((stderr, "bytes of data to be multicombined:%zu\n", bytes));

            const uint64_t 	alignment = 64;
            uint64_t	mask1	= 0;
	    mask1 = (alignment - 1);

            /* All nodes except master(local rank 0), do the math */

            if (_local_rank != 0){

              if (_npeers == 4)
              {

                /* Check for the alignment..Assuming L1 cache line alignment for now */
                PAMI_ASSERT(ALIGNED_SRCBUF(0,mask1) && ALIGNED_SRCBUF(1,mask1) &&ALIGNED_SRCBUF(2,mask1) &&ALIGNED_SRCBUF(3,mask1)) ;

                advance_4way_sum(mcomb_control, _local_rank, _npeers, dst, bytes);

              }
              else if (_npeers == 8){

                /* Check for the alignment..Assuming L1 cache line alignment for now */
                PAMI_ASSERT(ALIGNED_SRCBUF(0,mask1) && ALIGNED_SRCBUF(1,mask1) &&ALIGNED_SRCBUF(2,mask1) &&ALIGNED_SRCBUF(3,mask1)) ;
                PAMI_ASSERT(ALIGNED_SRCBUF(4,mask1) && ALIGNED_SRCBUF(5,mask1) &&ALIGNED_SRCBUF(6,mask1) &&ALIGNED_SRCBUF(7,mask1)) ;

                advance_8way_sum(mcomb_control, _local_rank, _npeers, dst, bytes);
              }
              else if (_npeers == 16){

                /* Check for the alignment..Assuming L1 cache line alignment for now */
                PAMI_ASSERT(ALIGNED_SRCBUF(0,mask1) && ALIGNED_SRCBUF(1,mask1) &&ALIGNED_SRCBUF(2,mask1) &&ALIGNED_SRCBUF(3,mask1)) ;
                PAMI_ASSERT(ALIGNED_SRCBUF(4,mask1) && ALIGNED_SRCBUF(5,mask1) &&ALIGNED_SRCBUF(6,mask1) &&ALIGNED_SRCBUF(7,mask1)) ;
                PAMI_ASSERT(ALIGNED_SRCBUF(8,mask1) && ALIGNED_SRCBUF(9,mask1) &&ALIGNED_SRCBUF(10,mask1) &&ALIGNED_SRCBUF(11,mask1)) ;
                PAMI_ASSERT(ALIGNED_SRCBUF(12,mask1) && ALIGNED_SRCBUF(13,mask1) &&ALIGNED_SRCBUF(14,mask1) &&ALIGNED_SRCBUF(15,mask1)) ;

                advance_16way_sum(mcomb_control, _local_rank, _npeers, dst, bytes);

              }
              else{

                fprintf(stderr,"sum not yet supported\n");
                exit(0);

              }
            }

            /* Reduction over...start gathering the results, local_rank == 0, the master, gathers the results */
            else{
              unsigned iter, my_peer, loops=0;
              for (iter= (unsigned)CURRENT_ITER; (iter < NUM_CHUNKS(bytes)-1) && (iter < (unsigned)(CURRENT_ITER+BLOCK_ITER)); iter++,loops++)
              {
                my_peer = iter%(_npeers-1)+1;
                while (mcomb_control->chunks_done[my_peer] <= mcomb_control->chunks_copied[my_peer]){};

                {
                  quad_double_copy((dst + NUM_DBLS_PER_CHUNK*iter),
                      (SHADDR_DSTBUF(my_peer)+NUM_DBLS_PER_CHUNK*iter), NUM_DBLS_PER_CHUNK);
                  rcv->produceBytes(CHUNK_SIZE);
                  mcomb_control->chunks_copied[my_peer] += 1;
                }
              }

              if (loops > 0)
              {
                CURRENT_ITER+=loops;
                TRACE_ERR((stderr,"CURRENT_ITER:%d loops:%u\n",CURRENT_ITER, loops));
                return PAMI_EAGAIN;		//breaking the advance so that the master can advance network
              }

              /* copying last chunk */
              my_peer = CURRENT_ITER %(_npeers-1)+1;
              while (mcomb_control->chunks_done[my_peer] <= mcomb_control->chunks_copied[my_peer]){};
              mem_sync();


              if (bytes%CHUNK_SIZE == 0)
              {
                /*memcpy((void*)(dst + NUM_DBLS_PER_CHUNK*iter),
                  (void*)(SHADDR_DSTBUF(my_peer)+NUM_DBLS_PER_CHUNK*iter), CHUNK_SIZE);*/
                quad_double_copy((dst + NUM_DBLS_PER_CHUNK*CURRENT_ITER),
                    (SHADDR_DSTBUF(my_peer)+NUM_DBLS_PER_CHUNK*CURRENT_ITER), NUM_DBLS_PER_CHUNK);
                rcv->produceBytes(CHUNK_SIZE);
              }
              else
              {
                memcpy((void*)(dst + NUM_DBLS_PER_CHUNK*CURRENT_ITER),
                    (void*)(SHADDR_DSTBUF(my_peer)+NUM_DBLS_PER_CHUNK*CURRENT_ITER), bytes%CHUNK_SIZE);
                rcv->produceBytes(bytes%CHUNK_SIZE);
              }
            }

            _my_desc->signal_done();
            //while (_my_desc->in_use()){}; //wait for everyone to signal done
            _my_desc->set_my_state(Shmem::DONE);
            mcomb_params.cb_done.function(this->_context, mcomb_params.cb_done.clientdata, PAMI_SUCCESS);
            return PAMI_SUCCESS;

            }

            public:
          inline ShaddrMcombMessagePipe (pami_context_t context, T_Desc* my_desc, unsigned local_rank) :
            BaseMessage<T_Device, T_Desc>(context, my_desc, ShaddrMcombMessagePipe::__advance, (void*)this, local_rank)
            {
              TRACE_ERR((stderr, "<> ShaddrMcombMessagePipe::ShaddrMcombMessagePipe()\n"));
            };


          };  // PAMI::Device::ShaddrMcombMessagePipe class

      };
    };    // PAMI::Device namespace
  };      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShaddrMcombMessagePipe_h__

  //
  // astyle info    http://astyle.sourceforge.net
  //
  // astyle options --style=gnu --indent=spaces=2 --indent-classes
  // astyle options --indent-switches --indent-namespaces --break-blocks
  // astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
  //
