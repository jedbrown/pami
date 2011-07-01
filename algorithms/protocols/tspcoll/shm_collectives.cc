/**
 * \file algorithms/protocols/tspcoll/shm_collectives.cc
 * \brief ???
 */
#include "shm_collectives.h"

extern "C" void __xlupc_barrier (int, int);
extern "C" int XLPGAS_CAU_SHM_AVAIL;

typedef xlpgas::local::FixedLeader<xlpgas::Wait>    FL;
typedef xlpgas::local::FixedLeaderLB<xlpgas::Wait>  FL_LARGE;

char*  xlpgas_shm_buf;
char*  xlpgas_shm_buf_bcast;
char*  xlpgas_shm_buf_lg_bcast;

void xlpgas::shm_init(int nctxt) {
  int key = 11223;

  if(nctxt>0){
    //shared memory mode; CAU/SHMEM will be disabled for now;
    XLPGAS_CAU_SHM_AVAIL=6; //ERROR; cau/shmem will be disabled
    return;
  }

  char      *env_str = NULL;
  env_str   = getenv("MP_COMMON_TASKS");
  int nthreads = atoi(env_str)+1;

  int fl_size = sizeof(FL::State) * nthreads;
  int fl_id = shmget (key, fl_size, IPC_CREAT | 0600 );
  xlpgas_shm_buf = (char *)shmat (fl_id, NULL, 0);
  if (xlpgas_shm_buf == (void *) -1) {
    XLPGAS_CAU_SHM_AVAIL=3; //ERROR; cau/shmem will be disabled
  }
  memset (xlpgas_shm_buf, 0, fl_size);

  int fl_id_bc = shmget (key+1, fl_size, IPC_CREAT | 0600 );
  xlpgas_shm_buf_bcast = (char *)shmat (fl_id_bc, NULL, 0);
  if (xlpgas_shm_buf_bcast == (void *) -1) {
    XLPGAS_CAU_SHM_AVAIL=4; //ERROR; cau/shmem will be disabled
  }
  memset (xlpgas_shm_buf_bcast, 0, fl_size);

  //large message bcast
  int fl_lg_size = sizeof(FL_LARGE::State) * nthreads + 2*SHM_BUF_SIZE;
  int fl_id_lgbc = shmget (key+2, fl_lg_size, IPC_CREAT | 0600 );
  xlpgas_shm_buf_lg_bcast = (char *)shmat (fl_id_lgbc, NULL, 0);
  if (xlpgas_shm_buf_lg_bcast == (void *) -1) {
    XLPGAS_CAU_SHM_AVAIL=5; //ERROR; cau/shmem will be disabled
  }
  memset ((void*)xlpgas_shm_buf_lg_bcast, 0, fl_lg_size);
  if(XLPGAS_CAU_SHM_AVAIL>=3) printf("ERROR while allocating SHMEM\n");

  xlpgas_tsp_barrier (XLPGAS_MYSMPTHREAD);

  shmctl (fl_id, IPC_RMID, NULL);
  shmctl (fl_id_bc, IPC_RMID, NULL);
  shmctl (fl_id_lgbc, IPC_RMID, NULL);

}
