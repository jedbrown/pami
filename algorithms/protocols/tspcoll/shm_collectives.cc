#include "shm_collectives.h"

extern "C" void __xlupc_barrier (int, int);
extern "C" int XLPGAS_CAU_SHM_AVAIL;

typedef xlpgas::local::FixedLeader<xlpgas::Wait> FL;

char* xlpgas_shm_buf;
char* xlpgas_shm_buf_bcast;

void xlpgas::shm_init(int nthreads) {
  int key = 11223;
  int fl_size = sizeof(FL::State) * nthreads;
  int fl_id = shmget (key, fl_size, IPC_CREAT | 0600 );
  xlpgas_shm_buf = (char *)shmat (fl_id, NULL, 0);
  if (xlpgas_shm_buf == (void *) -1) {
    XLPGAS_CAU_SHM_AVAIL=3; //ERROR; cau/shmem will be disabled
  }
  memset (xlpgas_shm_buf, 0, fl_size);
  shmctl (fl_id, IPC_RMID, NULL); 

  int fl_id_bc = shmget (key+1, fl_size, IPC_CREAT | 0600 );
  xlpgas_shm_buf_bcast = (char *)shmat (fl_id_bc, NULL, 0);
  if (xlpgas_shm_buf_bcast == (void *) -1) {
    XLPGAS_CAU_SHM_AVAIL=4; //ERROR; cau/shmem will be disabled
  }
  memset (xlpgas_shm_buf_bcast, 0, fl_size);
  shmctl (fl_id_bc, IPC_RMID, NULL);
}
