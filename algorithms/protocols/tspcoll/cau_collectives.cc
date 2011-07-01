/**
 * \file algorithms/protocols/tspcoll/cau_collectives.cc
 * \brief ???
 */
#include "cau_collectives.h"

#ifndef RC0
#define RC0(S) { \
    int rc = S; \
    if (rc != 0) { \
        printf(#S " failed with rc %d, line %d\n", rc, __LINE__); \
        exit(-1); \
    } \
}
#endif

//extern pami_context_t  xlpgas_pami_ctxt[256];

int  base_group_id  = 0; // Cau group id
enum { XLPGAS_RECV_MCAST = 66, XLPGAS_RECV_REDUCE = 67 };
const int XLPGAS_CAU_POLL_CNT   = 10000;


// global LAPI resources
lapi_handle_t       lapi_handle;
//global if CAU avail or not
int XLPGAS_CAU_SHM_AVAIL;



long instance_id;
int64_t* mcast_data;
long mcast_sent;
long mcast_received;

struct mcast_hdr_t {
  uint src;
  uint seq;
};

void *recv_mcast(lapi_handle_t *hndl, void *uhdr, uint *uhdr_len,
        ulong *msg_len, compl_hndlr_t **comp_h, void **uinfo)
{
    lapi_return_info_t &ret_info = *(lapi_return_info_t *)msg_len;
    mcast_hdr_t  &mcast_hdr = *(mcast_hdr_t *)uhdr;
    assert( *uhdr_len == sizeof(mcast_hdr) );

    ++mcast_received;

    int64_t *data = (int64_t *)ret_info.udata_one_pkt_ptr;
    uint group_id = ret_info.src;
    //print the data
    //printf("L%d BCAST",XLPGAS_MYNODE);
    //for(int i=0;i<4;++i){
    //  printf("=%d",((int64_t*)data)[i]);
    //}
    //printf("\n");
    //memcpy((int64_t *)mcast_data, 
    //	   data, ret_info.msg_len
    //	   );
    *mcast_data = *data;
    return NULL;
}

void on_mcast_sent(lapi_handle_t *hndl, void *cookie)
{
  ++ mcast_sent;
}

////////////////////////////////////////////////////////////
long reduce_sent;
long reduce_received;

struct reduce_hdr_t {
    cau_reduce_op_t op;
    uint seq;
};

int64_t* reduce_data;
int64_t temp_reduce_data;

void on_reduce_sent(lapi_handle_t *hndl, void *cookie)
{
  ++reduce_sent;
}


void xlpgas::reduce_op(int64_t *dst, int64_t *src, const cau_reduce_op_t& op)
{
    switch (op.operand_type) {
        case CAU_SIGNED_INT:
            reduce_fixed_point(*(int32_t *)dst, *(int32_t *)src, op.operation);
            break;
        case CAU_UNSIGNED_INT:
            reduce_fixed_point(*(uint32_t *)dst, *(uint32_t *)src, op.operation);
            break;
        case CAU_SIGNED_LONGLONG:
            reduce_fixed_point(*(int64_t *)dst, *(int64_t *)src, op.operation);
            break;
        case CAU_UNSIGNED_LONGLONG:
            reduce_fixed_point(*(uint64_t *)dst, *(uint64_t *)src, op.operation);
            break;
        case CAU_FLOAT:
            reduce_floating_point(*(float *)dst, *(float *)src, op.operation);
            break;
        case CAU_DOUBLE:
            reduce_floating_point(*(double *)dst, *(double *)src, op.operation);
            break;
        default: assert(!"Bogus reduce operand type");
    }
  }


void *recv_reduce(lapi_handle_t *hndl, void *uhdr, uint *uhdr_len,
        ulong *msg_len, compl_hndlr_t **comp_h, void **uinfo)
{
    lapi_return_info_t &ret_info = *(lapi_return_info_t *)msg_len;
    reduce_hdr_t  &reduce_hdr = *(reduce_hdr_t *)uhdr;
    assert( *uhdr_len == sizeof(reduce_hdr) );

    ++reduce_received; //reduce_hdr.seq;
    int64_t *data = (int64_t *)ret_info.udata_one_pkt_ptr;
    uint group_id = ret_info.src;

    temp_reduce_data = *data;
    return NULL;
}




void xlpgas::cau_init(void) {
  //decide if cau available
  XLPGAS_CAU_SHM_AVAIL=0;//cau available and must be called

  char           *env_str = NULL;
  env_str   = getenv("MP_COMMON_TASKS");
  int ncaus = atoi(env_str);
  
  if(ncaus == 0) XLPGAS_CAU_SHM_AVAIL = 1; //one node; cau can be bypassed but it will use shmem
  else {
    //query if env set for cau;
    env_str=NULL;
    env_str   = getenv("MP_SHARED_MEMORY");
    if( strcmp(env_str, "yes") != 0)  XLPGAS_CAU_SHM_AVAIL = 2;//cau/shmem not available
    
    env_str=NULL;
    env_str   = getenv("MP_COLLECTIVE_GROUPS");
    int ngroups=0;
    if(env_str !=NULL) ngroups = atoi(env_str);
    if( ngroups == 0)  XLPGAS_CAU_SHM_AVAIL = 2;//cau/shmem not available
  }

  //memcpy(&lapi_handle, (char*)(xlpgas_pami_ctxt[0]), sizeof(lapi_handle_t));
  lapi_handle = 0;
  RC0( LAPI_Addr_set(lapi_handle, ( void * )recv_mcast, XLPGAS_RECV_MCAST) );
  RC0( LAPI_Addr_set(lapi_handle, ( void * )recv_reduce, XLPGAS_RECV_REDUCE) );

  mcast_sent = 0;
  mcast_received = 0;
  reduce_sent = 0;
  reduce_received = 0;
  instance_id = 0;  
  RC0( LAPI_Gfence(lapi_handle) );
}


void xlpgas::cau_fast_allreduce(int64_t* dest, int64_t* src, cau_reduce_op_t& op, int ctxt){

  //printf("L[%d,%d] inside cau all reduce lapi_handle=%d bgid=%d\n",
  //	 XLPGAS_MYNODE,XLPGAS_MYSMPTHREAD,
  //	 lapi_handle,base_group_id);

  instance_id++;

  mcast_data  = dest;
  reduce_data = src;

  int ROOT=0;
  int data_size = 1;

  if(XLPGAS_MYNODE != ROOT) {
    //printf("Non Root sends\n");
    reduce_hdr_t  reduce_hdr;
    reduce_hdr.op     = op;
    reduce_hdr.seq    = 1;

    //printf("L%d Reduce  Send",XLPGAS_MYNODE);
    //for(int i=0;i<4;++i){
    //  printf("@%d",((int64_t*)reduce_data)[i]);
    //}
    //printf("\n Totalsize=%d\n",data_size*sizeof(int64_t));
    
    RC0( LAPI_Cau_reduce(lapi_handle, base_group_id, 
			 XLPGAS_RECV_REDUCE, &reduce_hdr, sizeof(reduce_hdr), 
			 reduce_data, 
			 data_size * sizeof(int64_t), op, 
			 on_reduce_sent, (void *)reduce_hdr.seq) );
    ++reduce_received;
    lapi_msg_info_t msg_info;
    bzero(&msg_info, sizeof(msg_info));
    while (reduce_sent < instance_id) {
      //xlpgas_tsp_wait (ctxt);
      RC0( LAPI_Msgpoll(lapi_handle, XLPGAS_CAU_POLL_CNT, &msg_info) );
    }
  }

  if(XLPGAS_MYNODE == ROOT) {
    //printf("L%d Reduce root wait for results\n", XLPGAS_MYNODE);
    //Root receives
    ++reduce_sent; 
    *mcast_data=*reduce_data;
    lapi_msg_info_t msg_info;
    bzero(&msg_info, sizeof(msg_info));
    while (reduce_received < instance_id) {
      //xlpgas_tsp_wait (ctxt);
      RC0( LAPI_Msgpoll(lapi_handle, XLPGAS_CAU_POLL_CNT, &msg_info) );
    }
    xlpgas::reduce_op(mcast_data, &temp_reduce_data, op);       
  }
  //printf("L%d reduce done\n",XLPGAS_MYNODE);


  //BCAST
  mcast_hdr_t  mcast_hdr;
  // initialize multicast header
  mcast_hdr.src    = ROOT; //root for multicast
  mcast_hdr.seq    = 1;
  
  
  if(XLPGAS_MYNODE == ROOT){
    //printf("L%d MCAST sent\n",XLPGAS_MYNODE);
    ++mcast_received;
    RC0( LAPI_Cau_multicast(lapi_handle, base_group_id, 
			    XLPGAS_RECV_MCAST, &mcast_hdr, sizeof(mcast_hdr), 
			    mcast_data, data_size * sizeof(int64_t), 
			    on_mcast_sent, (void *)mcast_hdr.seq) );
    
    //wait until sent
    lapi_msg_info_t msg_info;
    bzero(&msg_info, sizeof(msg_info));
    while (mcast_sent < instance_id) {
      //xlpgas_tsp_wait (ctxt);
      RC0( LAPI_Msgpoll(lapi_handle, XLPGAS_CAU_POLL_CNT, &msg_info) );
    }
  }
  else {
    ++mcast_sent;
    //all others wait for data to be received
    lapi_msg_info_t msg_info;
    bzero(&msg_info, sizeof(msg_info));
    while (mcast_received < instance_id) {
      // xlpgas_tsp_wait (ctxt);
      RC0( LAPI_Msgpoll(lapi_handle, XLPGAS_CAU_POLL_CNT, &msg_info) );
    }
    //printf("L%d MCAST RECV\n",XLPGAS_MYNODE);
  }
  
  //printf("L%d FINALREDUCE:",XLPGAS_MYNODE);
  //for(int i=0;i<4;++i){
  //  printf("=%d",((int64_t*)mcast_data)[i]);
  //}
  //printf("\n");
}

cau_reduce_op_t xlpgas::cau_op_dtype(xlpgas_ops_t      op,
			  xlpgas_dtypes_t   dtype){

  cau_reduce_op_t cau_op;
  
  switch (dtype)
    {
    case XLPGAS_DT_int:    cau_op.operand_type = CAU_SIGNED_INT; break;
    case XLPGAS_DT_word:   cau_op.operand_type = CAU_UNSIGNED_INT; break;
    case XLPGAS_DT_llg:    cau_op.operand_type = CAU_SIGNED_LONGLONG;break;
    case XLPGAS_DT_dwrd:   cau_op.operand_type = CAU_UNSIGNED_LONGLONG; break;
    case XLPGAS_DT_dbl:    cau_op.operand_type = CAU_DOUBLE; break;
    case XLPGAS_DT_flt:    cau_op.operand_type = CAU_FLOAT; break;
    default: xlpgas_fatalerror (-1,"xlpgas::cau_fast_allreduce :: data type not supported");
    }

  switch(op) {
    case XLPGAS_OP_ADD:    cau_op.operation = CAU_SUM; break;
    case XLPGAS_OP_AND: 
    case XLPGAS_OP_LOGAND: cau_op.operation = CAU_AND; break;
    case XLPGAS_OP_OR: 
    case XLPGAS_OP_LOGOR:  cau_op.operation = CAU_OR; break; 
    case XLPGAS_OP_XOR:    cau_op.operation = CAU_XOR; break; 
    case XLPGAS_OP_MAX:	   cau_op.operation = CAU_MAX; break; 
    case XLPGAS_OP_MIN:	   cau_op.operation = CAU_MIN; break; 
    case XLPGAS_OP_NOP:	   cau_op.operation = CAU_NOP; break; 
    default:		
      xlpgas_fatalerror (-1, "xlpgas::cau_fast_allreduce :: OP not implemented");
    }
  
  return cau_op;
}
