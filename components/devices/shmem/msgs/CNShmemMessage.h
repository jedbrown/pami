/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/msgs/CNShmemMessage.h
 * \brief ???
 */

#ifndef __components_devices_shmem_msgs_CNShmemMessage_h__
#define __components_devices_shmem_msgs_CNShmemMessage_h__

//#include "ShaddrMcombMessagePipe.h"

#undef TRACE_ERR

#ifndef TRACE_ERR
#define TRACE_ERR(x) // fprintf x
#endif
#include "../a2qpx_nway_sum.h"
#include "components/devices/shmem/CNShmemDesc.h"

namespace PAMI
{
  namespace Device
  {
    namespace Shmem
    {
        //This class has routines for advancing short/medium/long multicombines and multicasts
        class CNShmemMessage 
        {
          protected:

//#define	ChunkSize 	4096
//#define	ChunkSize 	512
#define	ChunkSize   1024
#define NumDblsPerChunk (ChunkSize/sizeof(double))
#define NumChunks(bytes)	((bytes%ChunkSize == 0) ? (bytes/ChunkSize):(bytes/ChunkSize + 1))
#define G_Srcs(x) 	((double*)(_controlB->GAT.srcbufs[x]))
#define G_Dsts(x) 	((double*)(_controlB->GAT.dstbufs[x]))
#define P_Dsts(x) 	((double*)(_controlB->phybufs[x]))
#define S_Bufs(x)       ((double*)(my_desc->get_buffer(x)))
#define G_Counter 	((uint64_t*)(_controlB->GAT.counter_addr))
#define MinChunkSize  64
#define ShmBufSize  SHORT_MSG_CUTOFF

            inline void advance_4way_sum(unsigned _local_rank, unsigned _npeers, size_t bytes, unsigned offset_dbl)
            {
              /* local ranks other than 0 do the following quad sum */
              unsigned chunk;

              for (chunk=0; chunk < NumChunks(bytes)-1; chunk++){
                if ((chunk%(_npeers-1) +1) == _local_rank){

                  quad_double_sum_4way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, NumDblsPerChunk);
                  _controlB->chunk_done[_local_rank] = chunk;
                }
                TRACE_ERR((stderr,"_rcvbuf[%u]:%f\n", chunk*NumDblsPerChunk, _rcvbuf[chunk*NumDblsPerChunk]));
              }

              /* summing last chunk which can be of any size */
              if ((chunk%(_npeers-1) +1) == _local_rank){
                if (bytes%ChunkSize == 0)
                {
                  quad_double_sum_4way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, NumDblsPerChunk);

                }
                else
                {
                  quad_double_sum_4way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, (bytes%ChunkSize)/sizeof(double));
                }
                _controlB->chunk_done[_local_rank] = chunk;
              }

            }

            inline void advance_8way_sum(unsigned _local_rank, unsigned _npeers, size_t bytes, unsigned offset_dbl)
            {
              /* local ranks other than 0 do the following quad sum */
              unsigned chunk;

              for (chunk=0; chunk < NumChunks(bytes)-1; chunk++){
                if ((chunk%(_npeers-1) +1) == _local_rank){

                  quad_double_sum_8way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(4)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(5)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(6)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(7)+chunk*NumDblsPerChunk+offset_dbl, NumDblsPerChunk);
                  _controlB->chunk_done[_local_rank] = chunk;
                }
              }

              /* summing last chunk which can be of any size */
              if ((chunk%(_npeers-1) +1) == _local_rank){
                if (bytes%ChunkSize == 0)
                {
                  quad_double_sum_8way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(4)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(5)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(6)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(7)+chunk*NumDblsPerChunk+offset_dbl, NumDblsPerChunk);
                }
                else
                {
                  quad_double_sum_8way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(4)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(5)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(6)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(7)+chunk*NumDblsPerChunk+offset_dbl, (bytes%ChunkSize)/sizeof(double));
                }
                mbar();
                _controlB->chunk_done[_local_rank] = chunk;
              }
            }

            inline void advance_16way_sum(unsigned _local_rank, unsigned _npeers, size_t bytes, unsigned offset_dbl)
            {
              /* local ranks other than 0 do the following quad sum */
              unsigned chunk;

              for (chunk=0; chunk < NumChunks(bytes)-1; chunk++){
                if ((chunk%(_npeers-1) +1) == _local_rank){

                  quad_double_sum_16way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(4)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(5)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(6)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(7)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(8)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(9)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(10)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(11)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(12)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(13)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(14)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(15)+chunk*NumDblsPerChunk+offset_dbl, NumDblsPerChunk);
                  _controlB->chunk_done[_local_rank] = chunk;
                }
              }

              /* summing last chunk which can be of any size */
              if ((chunk%(_npeers-1) +1) == _local_rank){
                if (bytes%ChunkSize == 0)
                {
                  quad_double_sum_16way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(4)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(5)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(6)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(7)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(8)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(9)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(10)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(11)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(12)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(13)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(14)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(15)+chunk*NumDblsPerChunk+offset_dbl, NumDblsPerChunk);

                }
                else
                {
                  quad_double_sum_16way(_rcvbuf+ chunk* NumDblsPerChunk+offset_dbl, G_Srcs(0)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(1)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(2)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(3)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(4)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(5)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(6)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(7)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(8)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(9)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(10)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(11)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(12)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(13)+chunk*NumDblsPerChunk+offset_dbl, G_Srcs(14)+chunk*NumDblsPerChunk+offset_dbl,
                      G_Srcs(15)+chunk*NumDblsPerChunk+offset_dbl, (bytes%ChunkSize)/sizeof(double));
                }
                _controlB->chunk_done[_local_rank] = chunk;
              }
            }


            //The buffer is partitioned among "npeers" in a balanced manner depending on the "MinChunkSize", total_bytes
            // Used for short to medium messages

            inline  bool  get_partition_info(unsigned npeers, unsigned local_rank, unsigned total_bytes, unsigned* offset_b,
                unsigned* chunk_size_b)
            {
              unsigned num_min_chunks = total_bytes/MinChunkSize;
              unsigned end_bytes  = total_bytes%MinChunkSize;
              unsigned  rounds =  num_min_chunks/npeers; 
              unsigned remaining_chunks = num_min_chunks%npeers;

              
              if (local_rank < remaining_chunks)
              {
                *chunk_size_b= (rounds+1)*MinChunkSize;
                *offset_b = (rounds+1)*MinChunkSize*local_rank;
                if ((local_rank == remaining_chunks-1) && (rounds == 0)) 
                  *chunk_size_b += end_bytes;
                return true;
              }
              else
              {
                if (rounds == 0) return false;
                *chunk_size_b= rounds*MinChunkSize;
                *offset_b = rounds*MinChunkSize*local_rank + MinChunkSize*remaining_chunks;
                if (local_rank == npeers-1)  
                  *chunk_size_b += end_bytes;
                return true;
              }
              
            }

          public:

            //Very short multicast..synchronization is done via a flag
            inline static pami_result_t very_short_msg_multicast(CNShmemDesc* my_desc, PipeWorkQueue *dpwq, unsigned total_bytes, unsigned npeers, unsigned local_rank,
                                                                   uint64_t* counter_addr, uint64_t &counter_curr)
            {
              void* buf = (void*)my_desc->get_buffer();
              void* rcvbuf = dpwq->bufferToProduce();

              if (local_rank == 0)
              {
                if (*counter_addr == counter_curr)
                  return PAMI_EAGAIN;

                mem_sync();
                my_desc->signal_flag();
              }
              else
              {
                if (my_desc->get_flag() == 0) 
                  return PAMI_EAGAIN;
              }

              memcpy(rcvbuf, buf, total_bytes);
              return PAMI_SUCCESS;
            }

            //Short multicast..data is taken from a shared buffer after CN deposits into it, supports pipelining
            inline pami_result_t short_msg_multicast(unsigned total_bytes, unsigned npeers, unsigned local_rank, uint64_t* counter_addr, uint64_t &counter_curr)
            {
              uint64_t  bytes_arrived =0;
              void* buf = (void*)_my_desc->get_buffer();
              unsigned bytes_so_far = total_bytes - counter_curr;

              if (local_rank == 0)
              {
                if (*counter_addr == counter_curr)
                  return PAMI_EAGAIN;

                bytes_arrived = counter_curr - *counter_addr;
                mem_sync();
                _controlB->bytes_incoming+= (unsigned)bytes_arrived;

                //memcpy((char*)_rcvbuf + bytes_so_far, (char*)buf + bytes_so_far, bytes_arrived);
                Core_memcpy((char*)_rcvbuf + bytes_so_far, (char*)buf + bytes_so_far, bytes_arrived);
              }
              else
              {
                if (_controlB->bytes_incoming > bytes_so_far)
                {
                  bytes_arrived = _controlB->bytes_incoming - bytes_so_far;
                  //memcpy((char*)_rcvbuf + bytes_so_far, (char*)buf + bytes_so_far, bytes_arrived);
                  Core_memcpy((char*)_rcvbuf + bytes_so_far, (char*)buf + bytes_so_far, bytes_arrived);
                }
              }

              counter_curr -= bytes_arrived;
              if (counter_curr)
                return PAMI_EAGAIN;

              return PAMI_SUCCESS;
            }

            // Large message multicast..data is read directly from the master's receive buffer. supports pipelining
            inline pami_result_t large_msg_multicast(unsigned total_bytes, unsigned npeers, unsigned local_rank, volatile uint64_t* counter_addr, uint64_t &counter_curr)
            {
              uint64_t  bytes_arrived =0;
              void* buf = (void*)G_Dsts(0);
              unsigned bytes_so_far = total_bytes - counter_curr;

              /* Non blocking until all the peers arrive at the collective */
              if (_my_desc->arrived_peers() != (unsigned) npeers)
              {
                TRACE_ERR((stderr,"arrived_peers:%u waiting for:%u\n", _my_desc->arrived_peers(), (unsigned) npeers));
                return PAMI_EAGAIN;
              }

              if (local_rank == 0)
              {
                if (*counter_addr == counter_curr)
                  if (counter_curr != 0) 
                    return PAMI_EAGAIN;

                bytes_arrived = counter_curr - *counter_addr;
                mem_sync();
                _controlB->bytes_incoming+= (unsigned)bytes_arrived;
                counter_curr -= bytes_arrived;
              }
              else
              {
                bytes_arrived = _controlB->bytes_incoming - bytes_so_far;
                
                if (bytes_arrived > 0)
                {
                  Core_memcpy((char*)_rcvbuf + bytes_so_far, (char*)buf + bytes_so_far, bytes_arrived);
                  counter_curr -= bytes_arrived;
                }
              }

              if (counter_curr)
                return PAMI_EAGAIN;
     
              _my_desc->signal_done();
              if (local_rank == 0)
                while (_my_desc->in_use()){};

              return PAMI_SUCCESS;
            }

            // Combining very short messages..the messages are fit into L2 cachelines to minimise the number of L2 loads
            inline static pami_result_t very_short_msg_combine(CNShmemDesc *my_desc, unsigned total_bytes, unsigned npeers, unsigned local_rank,
                                                              bool& done_flag)
            {

              if (my_desc->arrived_peers() != npeers)
                return PAMI_EAGAIN;


              if (local_rank == 0)
              {
                unsigned  count = total_bytes/sizeof(double);
                if (total_bytes <= 32)
                {
                  double* shm_buf = (double*)(my_desc->get_buffer());

                  if (npeers == 4)
                  {
                    quad_double_sum_4way( shm_buf, shm_buf, shm_buf+count, shm_buf+2*count, shm_buf+3*count, count); 
                  }
                  else if (npeers == 8)
                  {
                    quad_double_sum_8way( shm_buf, shm_buf, shm_buf+count, shm_buf+2*count, shm_buf+3*count,
                        shm_buf+4*count,  shm_buf+5*count,shm_buf+6*count,shm_buf+7*count, count); 
                  }
                  else if (npeers == 16)
                  {
                    quad_double_sum_16way( shm_buf, shm_buf, shm_buf+count, shm_buf+2*count, shm_buf+3*count,
                        shm_buf+4*count,  shm_buf+5*count,shm_buf+6*count,shm_buf+7*count, shm_buf+8*count,
                        shm_buf+9*count,shm_buf+10*count,  shm_buf+11*count, shm_buf+12*count,  
                        shm_buf+13*count, shm_buf+14*count, shm_buf+15*count, count); 
                  }
                  else
                  {
                    fprintf(stderr,"%s:%u npeers %u sum not yet supported\n",__FILE__,__LINE__,npeers);
                  }
                }
                else
                {
                  if (npeers == 4)
                  {
                    quad_double_sum_4way( S_Bufs(0), S_Bufs(0), S_Bufs(1), S_Bufs(2), S_Bufs(3), count); 
                  }
                  else if (npeers == 8)
                  {
                    quad_double_sum_8way( S_Bufs(0), S_Bufs(0), S_Bufs(1), S_Bufs(2), S_Bufs(3), S_Bufs(4), S_Bufs(5), S_Bufs(6), S_Bufs(7),  count); 
                  }
                  else if (npeers == 16)
                  {
                    quad_double_sum_16way( S_Bufs(0), S_Bufs(0), S_Bufs(1), S_Bufs(2), S_Bufs(3), S_Bufs(4), S_Bufs(5), S_Bufs(6), S_Bufs(7), S_Bufs(8), S_Bufs(9), S_Bufs(10), S_Bufs(11), S_Bufs(12), S_Bufs(13), S_Bufs(14), S_Bufs(15), count); 
                  }
                  else
                  {
                    fprintf(stderr,"%s:%u npeers %u sum not yet supported\n",__FILE__,__LINE__,npeers);
                  }

                }

              }
                done_flag = true;
              return PAMI_SUCCESS;
            }

            // multicombine with reads from the global virtual addresses of the buffers and writes to a shmem buffer
            // which is then injected into the Collective network
            inline pami_result_t short_msg_combine(unsigned total_bytes, unsigned npeers, unsigned local_rank, bool& done_flag)
            {

              /* Non blocking until all the peers arrive at the collective */
              if (_my_desc->arrived_peers() != npeers)
                return PAMI_EAGAIN;

              unsigned offset_b=0, chunk_size_b=0, offset_dbl, chunk_size_dbl;
              bool  is_participant;
              is_participant  = get_partition_info(npeers, local_rank, total_bytes, &offset_b, &chunk_size_b);
              offset_dbl  = offset_b >> 3;
              chunk_size_dbl = chunk_size_b >> 3;

              if (npeers == 4)
              {
                if (is_participant)
                  quad_double_sum_4way((double*)_controlB->buffer+ offset_dbl, G_Srcs(0)+offset_dbl, 
                      G_Srcs(1)+offset_dbl, G_Srcs(2)+offset_dbl, G_Srcs(3)+offset_dbl, chunk_size_dbl);
              }
              else if (npeers == 8)
              {
                if (is_participant)
                  quad_double_sum_8way((double*)_controlB->buffer+ offset_dbl, G_Srcs(0)+offset_dbl, 
                      G_Srcs(1)+offset_dbl, G_Srcs(2)+offset_dbl, G_Srcs(3)+offset_dbl, G_Srcs(4)+offset_dbl,
                      G_Srcs(5)+offset_dbl, G_Srcs(6)+offset_dbl, G_Srcs(7)+offset_dbl, chunk_size_dbl);
              }
              else if (npeers == 16)
              {
                if (is_participant)
                {
                  quad_double_sum_16way((double*)_controlB->buffer+ offset_dbl, G_Srcs(0)+offset_dbl, 
                      G_Srcs(1)+offset_dbl, G_Srcs(2)+offset_dbl, G_Srcs(3)+offset_dbl, G_Srcs(4)+offset_dbl,
                      G_Srcs(5)+offset_dbl, G_Srcs(6)+offset_dbl, G_Srcs(7)+offset_dbl, G_Srcs(8)+offset_dbl,
                      G_Srcs(9)+offset_dbl, G_Srcs(10)+offset_dbl, G_Srcs(11)+offset_dbl, G_Srcs(12)+offset_dbl,
                      G_Srcs(13)+offset_dbl, G_Srcs(14)+offset_dbl, G_Srcs(15)+offset_dbl, chunk_size_dbl);
                }
              }
              else
              {
                fprintf(stderr,"%s:%u npeers %u sum not yet supported\n",__FILE__,__LINE__,npeers);
              }

              _my_desc->signal_done();
              while (_my_desc->in_use()){};

              done_flag = true;
              return PAMI_SUCCESS;
            }

            //Only the non-master peers participate in this..data is combined directly in chunks, from the peers' buffers using
            //Global Virtual addresses on the node and each peer updates its chunk index once done
            inline pami_result_t large_msg_combine_peers (size_t total_bytes, unsigned npeers, unsigned local_rank, unsigned offset_dbl)
            {
              /* Non blocking until all the peers arrive at the collective */
              if (_my_desc->arrived_peers() != (unsigned) npeers)
              {
                TRACE_ERR((stderr,"arrived_peers:%u waiting for:%u\n", _my_desc->arrived_peers(), (unsigned) npeers));
                return PAMI_EAGAIN;
              }


              /* All nodes except master(local rank 0), do the math */
              if (npeers == 4)
                advance_4way_sum(local_rank, npeers, total_bytes, offset_dbl);
              else if (npeers == 8)
                advance_8way_sum(local_rank, npeers, total_bytes, offset_dbl);
              else if (npeers == 16)
                advance_16way_sum(local_rank, npeers, total_bytes, offset_dbl);
              else
              {
                fprintf(stderr,"%s:%u npeers %u sum not yet supported\n",__FILE__,__LINE__,npeers);
                exit(0);
              }

              return PAMI_SUCCESS;
            }


            //The master process gets the next chunk to inject into Collective network
            inline void* next_injection_buffer (uint64_t *bytes_available, unsigned total_bytes, unsigned npeers)
            {
              unsigned  my_peer = (_chunk_for_injection)%(npeers-1)+1;
              char* rbuf = (char*) P_Dsts(my_peer);

              if (_controlB->chunk_done[my_peer] >= _chunk_for_injection)
              { 
                *bytes_available  = (_chunk_for_injection < (NumChunks(total_bytes)-1)) ? (ChunkSize):
                  (total_bytes - ChunkSize*(NumChunks(total_bytes)-1));
                return (void*)(rbuf + _chunk_for_injection*ChunkSize);
              }
              return  NULL;  
            }


            //Update the chunk index once injection is complete
            inline  void  injection_complete() { ++_chunk_for_injection;  }


            inline CNShmemMessage () {};
            inline CNShmemMessage (CNShmemDesc* my_desc, uint32_t length):_my_desc(my_desc),_total_bytes(length),_chunk_for_injection(0) {};

            inline ~CNShmemMessage() {};

            //Initialize all the Virtual,Global and Physical addreses required in the operation
            //Initialize the shmem descriptor used for staging data and synchronization
            inline void init (void* srcbuf, void* rcvbuf, void* srcbuf_gva, void* rcvbuf_gva, void* rcvbuf_phy, void* shmbuf_phy, unsigned local_rank)
            {

              _srcbuf = (double*)srcbuf;
              _rcvbuf = (double*)rcvbuf;

              void* buf = _my_desc->get_buffer();
              _controlB = (ControlBlock*)buf;

              _controlB->GAT.srcbufs[local_rank] = srcbuf_gva;
              _controlB->GAT.dstbufs[local_rank] = rcvbuf_gva;
              _controlB->phybufs[local_rank] = rcvbuf_phy;
              _shm_phy_addr = shmbuf_phy;

              if (local_rank == 0)
              {
                _controlB->bytes_incoming=0;
                for (unsigned i=0; i < __global.mapping.tSize(); i++)
                {
                  _controlB->chunk_done[i] = -1;
                }
              }
            };

            void*     _shm_phy_addr;
          private:

            CNShmemDesc    *_my_desc;
            unsigned  _total_bytes;
            double*     _srcbuf;
            double*     _rcvbuf;
            uint16_t _chunk_for_injection;
            ControlBlock* _controlB;


        };  // PAMI::Device::CNShmemMessage class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_CNShmemMessage_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
