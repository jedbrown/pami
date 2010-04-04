/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/shmem/ShmemMcombMessageShaddr.h
 * \brief ???
 */

#ifndef __components_devices_shmem_McombMessageShaddr_h__
#define __components_devices_shmem_McombMessageShaddr_h__

#include <errno.h>
#include <sys/uio.h>

#include "Arch.h"
#include "sys/pami.h"
//#include "components/devices/shmem/ShmemMessage.h"
#include "components/devices/shmemcoll/McombMessage.h"
//#include "components/devices/shmem/ShmemCollDesc.h"
//#include "quad_sum.h"
//#include "16way_sum.h"
#include "4way_sum.h"
#include "2way_sum.h"
#include "opt_copy_a2.h"
#include "8way_sum.h"
#include "assert.h"


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
#define	CHUNK_SIZE	2048
#define	INITIAL_CHUNK_SIZE	2048
//#define	CHUNK_SIZE 	8192
#define NUM_DBLS_PER_CHUNK (CHUNK_SIZE/sizeof(double))
#define NUM_CHUNKS(bytes)	((bytes%CHUNK_SIZE == 0) ? (bytes/CHUNK_SIZE):(bytes/CHUNK_SIZE + 1))

#define SHADDR_SRCBUF(x) 	((double*)(mcomb_control->GlobalAddressTable.src_bufs[x]))
#define SHADDR_DSTBUF(x) 	((double*)(mcomb_control->GlobalAddressTable.dst_bufs[x]))

#define ALIGNED_SRCBUF(x,y)	((((uint64_t)(mcomb_control->GlobalAddressTable.src_bufs[x])) & ((uint64_t)y)) == 0)
#define ALIGNED_DSTBUF(x,y)	((((uint64_t)(mcomb_control->GlobalAddressTable.dst_bufs[x])) & ((uint64_t)y)) == 0)

	  struct McombControl
	  {
	  	struct 
	  	{
			void* src_bufs[NUM_LOCAL_TASKS];
			void* dst_bufs[NUM_LOCAL_TASKS];
	  	}GlobalAddressTable __attribute__((__aligned__(128))); 	
		volatile uint16_t	chunks_done[NUM_LOCAL_TASKS];	
		volatile uint16_t	chunks_copied[NUM_LOCAL_TASKS];	
	  };
	
      template <class T_Device, class T_Desc>
      class McombMessageShaddr : public McombMessage<T_Device, T_Desc>
      {
        protected:

          static pami_result_t __advance (pami_context_t context, void * cookie)
          {
            McombMessageShaddr * msg = (McombMessageShaddr *) cookie;
            return msg->advance();
          };

		  inline void advance_4way_sum(Shmem::McombControl* mcomb_control, unsigned _local_rank, unsigned _npeers, 
										double* dst, size_t bytes)
		  {
				/* local ranks other than 0 do the following quad sum */
				unsigned iter=0,  bytes_so_far= 0;
				size_t chunk_size = INITIAL_CHUNK_SIZE;

				while (bytes_so_far< bytes){
					for (iter=1; iter <= 3; iter++){
					chunk_size = (bytes-bytes_so_far > chunk_size )? chunk_size: (bytes-bytes_so_far);
					if (iter == _local_rank){
					if (chunk_size%1024 == 0)//checking for multiple of 1024 bytes only
					{ 
						quad_double_sum_4way(dst+ bytes_so_far/sizeof(double), SHADDR_SRCBUF(0)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(1)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(2)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(3)+bytes_so_far/sizeof(double), chunk_size/sizeof(double));
					}
					else
					{
						for (unsigned i =0; i < chunk_size/sizeof(double); i++)
						{
							dst[i+bytes_so_far/sizeof(double)] = SHADDR_SRCBUF(0)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(1)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(2)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(3)[i+bytes_so_far/sizeof(double)]; 
						}

					}	
					mcomb_control->chunks_done[_local_rank] += 1;
					}
					bytes_so_far += chunk_size;
					TRACE_ERR((stderr,"dst[%u]:%f\n", iter*NUM_DBLS_PER_CHUNK, dst[iter*NUM_DBLS_PER_CHUNK]));
					if (bytes_so_far == bytes) break;
					}
					chunk_size*=2;
				}	
		  }			

		  inline void advance_8way_sum(Shmem::McombControl* mcomb_control, unsigned _local_rank, unsigned _npeers, 
										double* dst, size_t bytes)
		  {
				/* local ranks other than 0 do the following quad sum */
				unsigned iter=0,  bytes_so_far= 0;
				size_t chunk_size = INITIAL_CHUNK_SIZE;

				while (bytes_so_far< bytes){
					for (iter=1; iter <= 7; iter++){
					chunk_size = (bytes-bytes_so_far > chunk_size )? chunk_size: (bytes-bytes_so_far);
					if (iter == _local_rank){
					if (chunk_size%1024 == 0)//checking for multiple of 1024 bytes only
					{ 
						quad_double_sum_8way(dst+ bytes_so_far/sizeof(double), SHADDR_SRCBUF(0)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(1)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(2)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(3)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(4)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(5)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(6)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(7)+bytes_so_far/sizeof(double), chunk_size/sizeof(double));
					}
					else
					{
						for (unsigned i =0; i < chunk_size/sizeof(double); i++)
						{
							dst[i+bytes_so_far/sizeof(double)] = SHADDR_SRCBUF(0)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(1)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(2)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(3)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(4)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(5)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(6)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(7)[i+bytes_so_far/sizeof(double)];
						}

					}	
					mcomb_control->chunks_done[_local_rank] += 1;
					}
					bytes_so_far += chunk_size;
					TRACE_ERR((stderr,"dst[%u]:%f\n", iter*NUM_DBLS_PER_CHUNK, dst[iter*NUM_DBLS_PER_CHUNK]));
					if (bytes_so_far == bytes) break;
					}
					chunk_size*=2;
				}	

		  }

		  inline void advance_16way_sum(Shmem::McombControl* mcomb_control, unsigned _local_rank, unsigned _npeers, 
										double* dst, size_t bytes)
		  {
				/* local ranks other than 0 do the following quad sum */
				unsigned iter=0,  bytes_so_far= 0;
				size_t chunk_size = INITIAL_CHUNK_SIZE;

				while (bytes_so_far< bytes){
					for (iter=1; iter <= 15; iter++){
					chunk_size = (bytes-bytes_so_far > chunk_size )? chunk_size: (bytes-bytes_so_far);
					if (iter == _local_rank){
					if (chunk_size%1024 == 0)//checking for multiple of 1024 bytes only
					{ 
						quad_double_sum_16way(dst+ bytes_so_far/sizeof(double), SHADDR_SRCBUF(0)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(1)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(2)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(3)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(4)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(5)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(6)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(7)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(8)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(9)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(10)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(11)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(12)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(13)+bytes_so_far/sizeof(double), SHADDR_SRCBUF(14)+bytes_so_far/sizeof(double), 
										SHADDR_SRCBUF(15)+bytes_so_far/sizeof(double), chunk_size/sizeof(double));
					}
					else
					{
						for (unsigned i =0; i < chunk_size/sizeof(double); i++)
						{
							dst[i+bytes_so_far/sizeof(double)] = SHADDR_SRCBUF(0)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(1)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(2)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(3)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(4)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(5)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(6)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(7)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(8)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(9)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(10)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(11)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(12)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(13)[i+bytes_so_far/sizeof(double)] + SHADDR_SRCBUF(14)[i+bytes_so_far/sizeof(double)] + 
										SHADDR_SRCBUF(15)[i+bytes_so_far/sizeof(double)];
						}

					}	
					mcomb_control->chunks_done[_local_rank] += 1;
					}
					bytes_so_far += chunk_size;
					TRACE_ERR((stderr,"dst[%u]:%f\n", iter*NUM_DBLS_PER_CHUNK, dst[iter*NUM_DBLS_PER_CHUNK]));
					if (bytes_so_far == bytes) break;
					}
					chunk_size*=2;
				}	

		  }


          inline pami_result_t advance ()
          {


			TRACE_ERR((stderr, "in Mcomb advance\n"));
			T_Desc* _my_desc = this->_my_desc;
			T_Desc* _master_desc = this->_master_desc;
			unsigned _npeers = this->_npeers;
			unsigned _local_rank = this->_local_rank;
			//unsigned _task = this->_task;

			pami_multicombine_t & mcomb_params = _my_desc->get_mcomb_params();
			size_t num_src_ranks = ((PAMI::Topology*)mcomb_params.data_participants)->size();

			//unsigned long long t1, t2;
			//t1  = PAMI_Wtimebase();
			//while (_master_desc->arrived_peers() != (unsigned) num_src_ranks){}
			//	fprintf(stderr,"[%d]arrived peers:%u num_src_ranks:%zd", _local_rank, _master_desc->arrived_peers(), num_src_ranks);
			
			/* Non blocking until all the peers arrive at the collective */
			if (_master_desc->arrived_peers() != (unsigned) num_src_ranks)
			{
				 TRACE_ERR((stderr,"arrived_peers:%u waiting for:%u\n", _master_desc->arrived_peers(), (unsigned) num_src_ranks));
				 return PAMI_EAGAIN;
			}
			TRACE_ERR((stderr, "all peers:%zu arrived, starting the blocking Mcomb protocol\n", num_src_ranks));

			/* Start the protocol here..blocking version since everyone arrived */

			TRACE_ERR((stderr,"size of desc:%ld sizeof mcomb_control:%ld", sizeof(T_Desc), sizeof(McombControl)));

			Shmem::McombControl* mcomb_control = (Shmem::McombControl*) _master_desc->get_buffer();
			assert(mcomb_control != NULL);

			PAMI::PipeWorkQueue *rcv = (PAMI::PipeWorkQueue *)mcomb_params.results;
			double* dst = (double*)(rcv->bufferToConsume());
			size_t bytes = mcomb_params.count << pami_dt_shift[mcomb_params.dtype];

			TRACE_ERR((stderr, "bytes of data to be multicombined:%zu\n", bytes));

			const uint64_t 	alignment = 64;
			const uint64_t	mask1	= (alignment - 1);

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
				//unsigned long long t1 = PAMI_Wtimebase();	
		   		advance_8way_sum(mcomb_control, _local_rank, _npeers, dst, bytes);
				//unsigned long long t2 = PAMI_Wtimebase();
				 //fprintf(stderr,"[%d]cycles:%lld, bytes:%zd\n", _local_rank,(t2-t1), bytes);
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
				size_t iter = 0, bytes_so_far = 0;
				size_t chunk_size = INITIAL_CHUNK_SIZE;
				while (bytes_so_far< bytes){
					for (iter=1; iter <= 15; iter++){
					chunk_size = (bytes-bytes_so_far > chunk_size )? chunk_size: (bytes-bytes_so_far);
					while (mcomb_control->chunks_done[iter] <= mcomb_control->chunks_copied[iter]){};

					if (chunk_size%1024 == 0)//checking for multiple of 1024 bytes only
					{ 
						quad_double_copy((dst + bytes_so_far/sizeof(double)), 
							 	  (SHADDR_DSTBUF(iter)+bytes_so_far/sizeof(double)), chunk_size/sizeof(double));
						rcv->produceBytes(chunk_size);
					}
					else
					{
						memcpy((void*)(dst + bytes_so_far/sizeof(double)), 
						 	  (void*)(SHADDR_DSTBUF(iter)+bytes_so_far/sizeof(double)), chunk_size/sizeof(double));
						rcv->produceBytes(chunk_size);
					}	
					mcomb_control->chunks_copied[iter] += 1;
					bytes_so_far += chunk_size;
					TRACE_ERR((stderr,"dst[%u]:%f\n", iter*NUM_DBLS_PER_CHUNK, dst[iter*NUM_DBLS_PER_CHUNK]));
					if (bytes_so_far == bytes) break;
					}
					chunk_size*=2;
				}	

			}

			_master_desc->signal_done();
			while (_master_desc->in_use()){}; //wait for everyone to signal done
			//t2 = PAMI_Wtimebase();
			//fprintf(stderr,"[%d]cycles:%lld, bytes:%zd\n", _local_rank,(t2-t1), bytes);
			this->setStatus (PAMI::Device::Done);
			return PAMI_SUCCESS;

          }
			

        public:
          inline McombMessageShaddr (T_Device *device, T_Desc* desc, T_Desc* matched_desc) :
              	 McombMessage<T_Device, T_Desc> (device, desc, matched_desc, McombMessageShaddr::__advance, this)
          {
            TRACE_ERR((stderr, "<> McombMessageShaddr::McombMessageShaddr()\n"));
          };


      };  // PAMI::Device::McombMessageShaddr class

    };
  };    // PAMI::Device namespace
};      // PAMI namespace
#undef TRACE_ERR
#endif  // __components_devices_shmem_ShmemMcombMessageShaddr_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
