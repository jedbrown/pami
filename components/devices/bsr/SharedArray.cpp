#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sched.h>
#include <sys/shm.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#ifdef _LAPI_LINUX
#include "lapi_linux.h"
#endif
#include "atomics.h"
#include "SharedArray.h"
#include "lapi_itrace.h"

SharedArray::SETUP_STATE SharedArray::PosixShmAllAttached()
{
    if (ctrl_block->ref_cnt < member_cnt) {
        return ST_ATTACHED;
    } else {
        //???
        assert(ctrl_block->ref_cnt == member_cnt);
        // shm_id is not needed; close it to prevent file descripter leak
        close(shm_id);
        shm_id = -1;
        shm_unlink(this->shm_str);
        return ST_PASSED;
    }
}

SharedArray::SETUP_STATE SharedArray::PosixShmAttach(const unsigned int size)
{
    // internally we allocate extra space for control block (at lease 1 cache
    // line)
    const unsigned int i_size =
        (sizeof(ctrl_block)+CACHE_LINE_SZ-1)/CACHE_LINE_SZ*CACHE_LINE_SZ;

    const unsigned int t_size = // total size
        i_size + (size+CACHE_LINE_SZ-1)/CACHE_LINE_SZ*CACHE_LINE_SZ;

    // create shared memory or retrieve the existing one
    shm_id = shm_open(shm_str, O_RDWR|O_CREAT|O_EXCL, 0600);
    if (shm_id != -1) {
        ITRC(IT_BSR, "SharedArray: as Posix SHM master (shm_id=%d)\n",
                shm_id);
    } else {
        if (EEXIST == errno) {
            shm_id = shm_open(shm_str, O_RDWR, 0600);
            ITRC(IT_BSR, "SharedArray: as Posix SHM slave (shm_id=%d)\n",
                    shm_id);
            if (shm_id == -1) {
                ITRC(IT_BSR, "SharedArray: Posix shm_open failed (errno=%d)\n", 
                        errno);
                perror("shm_open as slave failed");
                assert(0);
                return ST_FAILED;
            }
        } else {
            ITRC(IT_BSR, "SharedArray: Posix shm_open failed (errno=%d)\n", 
                    errno);
            perror("shm_open as master failed");
            assert(0);
            return ST_FAILED;
        }
    }

    if (ftruncate(shm_id, t_size) == -1) {
        close(shm_id);
        shm_unlink(shm_str);
        ITRC(IT_BSR, "SharedArray: Posix ftruncate failed, (errno=%d)\n", errno);
        perror("ftruncate failed");
        assert(0);
        return ST_FAILED;
    }

    shm_size = t_size;

    // attach to shared memory
    ctrl_block = (ShmCtrlBlock*)mmap(NULL, t_size, PROT_READ|PROT_WRITE,
            MAP_SHARED, shm_id, 0);

    if (ctrl_block != NULL && ctrl_block != (void*)-1) {
        shm_seg = (void*)((char*)ctrl_block + i_size);
        fetch_and_add((atomic_p)&ctrl_block->ref_cnt, 1);

        return PosixShmAllAttached();
    } else {
        ITRC(IT_BSR, "SharedArray: Posix mmap failed, errno = %d\n", errno);
        // shm_id is not needed; close it to prevent file descripter leak
        close(shm_id);
        shm_id = -1;
        shm_unlink(shm_str);
        perror("mmap failed");
        assert(0);
        return ST_FAILED;
    }
}

SharedArray::RC SharedArray::IsPosixShmSetupDone(const unsigned int size)
{
    bool advance = true;
    while (advance) {
        advance = false;
        switch (setup_state) {
            case ST_NONE:
                setup_state = PosixShmAttach(size);
                advance = (setup_state != ST_NONE);
                break;
            case ST_ATTACHED:
                setup_state = PosixShmAllAttached();
                advance = (setup_state != ST_ATTACHED);
                break;
            case ST_PASSED:
                ITRC(IT_BSR, 
                        "SharedArray: Posix ShmSetup PASSED at %p with %u members\n", 
                        ctrl_block, member_cnt);
                return SUCCESS;
            case ST_FAILED:
                return FAILED;
        }
    }
    // when neither failed nor passed and no more progress
    return PROCESSING;
}

SharedArray::RC SharedArray::PosixShmDestroy()
{
    if (ctrl_block) {
        shm_seg = NULL;
        // decrement ref_cnt can cause timing issue
        // therefore should ***NOT*** be performed
        // fetch_and_add((atomic_p)&ctrl_block->ref_cnt, -1);
        munmap(ctrl_block, shm_size);
        shm_size = 0;
        // if ctrl_block is not NULL; shm_unlink is called already
        ctrl_block = NULL;
        ITRC(IT_BSR, "SharedArray: PosixShmDestroy finished\n");
    }
    ITRC(IT_BSR, "SharedArray: PosixShmDestroy finished without being initialized.\n");
    return SUCCESS;
}

SharedArray::~SharedArray()
{
    ITRC(IT_BSR, "~SharedArray() called 0x%p\n", this);
    PosixShmDestroy();
};
