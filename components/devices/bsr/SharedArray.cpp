#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sched.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#ifdef _LAPI_LINUX
#include "lapi_linux.h"
#else
#include <sys/atomic_op.h>
#endif

#include "SharedArray.h"
#include "lapi_itrace.h"

typedef struct timeval _timeval_t;

static inline
int ReadRealTime (_timeval_t* t) {
    return gettimeofday(t, NULL);
}

static inline
unsigned int TimeDiffInSec (const _timeval_t& end, const _timeval_t& start) {
    // returns the difference in whole seconds
    return ((end.tv_sec) - (start.tv_sec));
}

/*!
 * \file SharedArray.cpp
 *
 * \brief Implementation of the SharedArray class.
 *
 */

SharedArray::RC SharedArray::ShmSetup(const unsigned int shm_key,
        const unsigned int size, const unsigned int timeout)
{
    // internally we allocate extra space for control block (at lease 1 cache
    // line)
    const unsigned int i_size =
        (sizeof(ctrl_block)+CACHE_LINE_SZ-1)/CACHE_LINE_SZ*CACHE_LINE_SZ;

    const unsigned int t_size = // total size
        i_size + (size+CACHE_LINE_SZ-1)/CACHE_LINE_SZ*CACHE_LINE_SZ;

    // create shared memory or retrieve the existing one
    shm_id = shmget(shm_key, t_size, IPC_CREAT|IPC_EXCL|0600);
    if (shm_id != -1) {
        ITRC(IT_BAR, "SharedArray: as SHM master (key=0x%x, id=%d)\n", shm_key, shm_id);
    } else {
        if (EEXIST == errno) {
            ITRC(IT_BAR, "SharedArray: as SHM slave\n");
            shm_id = shmget(shm_key, 0, 0);
        } else {
            ITRC(IT_BAR, "SharedArray: shmget failed (errno=%d)\n", errno);
            return FAILED;
        }
        if (shm_id == -1) {
            ITRC(IT_BAR, "SharedArray: shmget failed, (errno=%d)\n", errno);
            return FAILED;
        }
    }

    // attach to shared memory
    ctrl_block = (ShmCtrlBlock*)shmat(shm_id, 0, 0);

    if (ctrl_block != NULL && ctrl_block != (void*)-1) {
        _timeval_t start_time = {0},
                   end_time   = {0};
        shm_seg = (void*)((char*)ctrl_block + i_size);

        fetch_and_add((atomic_p)&ctrl_block->ref_cnt, 1);
        ReadRealTime(&start_time);
        if (progress_cb) {
            while (ctrl_block->ref_cnt < member_cnt) {
                (*progress_cb)(progress_cb_info); // wait until all member arrived
                ReadRealTime(&end_time);
                unsigned int duration = TimeDiffInSec(end_time, start_time);
                if (timeout < duration) {
                    ITRC(IT_BAR, "SharedArray: ShmSetup timeout (dur:%d)\n", duration);
                    break;
                }
            }
        } else {
            while (ctrl_block->ref_cnt < member_cnt) {
                ReadRealTime(&end_time);
                unsigned int duration = TimeDiffInSec(end_time, start_time);
                if (timeout < duration) {
                    ITRC(IT_BAR, "SharedArray: ShmSetup timeout (dur:%d)\n", duration);
                    break;
                }
            }
        }
        // we ignore the return code of shmctl
        shmctl(shm_id, IPC_RMID, NULL);

        if (ctrl_block->ref_cnt != member_cnt) {
            // when failed, we decrease the ref_cnt and detach
            fetch_and_add((atomic_p)&ctrl_block->ref_cnt, -1);
            // we ignore the return code of shmdt
            shmdt((void*)ctrl_block);
            return FAILED;
        }
    } else {
        ITRC(IT_BAR, "SharedArray: shmat failed, errno = %d\n", errno);
        // we ignore the return code of shmctl
        shmctl(shm_id, IPC_RMID, NULL);
        return FAILED;
    }

    ITRC(IT_BAR, "SharedArray: ShmSetup successed\n");
    return SUCCESS;
}

SharedArray::RC SharedArray::ShmDestory()
{
    if (ctrl_block) {
        shm_seg = NULL;
        // decrease ref_cnt
        fetch_and_add((atomic_p)&ctrl_block->ref_cnt, -1);
        // detach from memory; we don't care the return code
        shmdt(ctrl_block);
        // remove the shm_id; we don't care the return code
        shmctl(shm_id, IPC_RMID, NULL);
        ctrl_block = NULL;
    }
    ITRC(IT_BAR, "SharedArray: ShmDestory finished\n");
    return SUCCESS;
}
