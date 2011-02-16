/*!
 * \file Bsr.cpp
 *
 * \brief Implementation of Bsr class.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <new>
#ifdef _LAPI_LINUX
#include "lapi_linux.h"
#else
#include <sys/atomic_op.h>
#endif
#include "Bsr.h"
#include "lapi_itrace.h"
#include "Arch.h"

/*!
 * \brief Default constructor.
 */
Bsr::Bsr() {
    bsr_key             = -1;
    bsr_id              = -1;
    bsr_size            = 0;
    bsr_addr            = NULL;
    is_bsr_attached     = false;
}

/*!
 * \brief Default destructor.
 */
Bsr::~Bsr() {
    ITRC(IT_BSR, "Bsr: In ~Bsr()\n");
    CleanUp();
}

void Bsr::CleanUp()
{
    ITRC(IT_BSR, "Bsr: In CleanUp()\n");
    // set status
    status = NOT_READY;

    if (bsr_addr) {
        if (shm) {
            // do we need synchronize here?
            // to make sure the state update to SHM is done before
            // we destory the SHM.
            mem_barrier();
            mem_isync();
            // detach from bsr, if attached before
            if (is_bsr_attached) {
                shmdt(bsr_addr);
            }
            // decrease the bsr_setup_ref from shm
            fetch_and_add((atomic_p)&shm->bsr_setup_ref, -1);

            if (is_leader) {
                // leader has to wait to unallocate BSR IDs before
                // free the SHM segment
                if (progress_cb) {
                    while (shm->bsr_setup_ref > 0) {
                        (*progress_cb)(progress_cb_info);
                    }
                } else while (shm->bsr_setup_ref > 0);

                shmctl(bsr_id, IPC_RMID, 0);

                ITRC(IT_BSR, "Bsr: bsr_unallocate is done for bsr_id=%d\n",
                        shm->bsr_id);
            }

            is_bsr_attached = false;
            ShmDestory();
            shm = NULL;
        }
        bsr_addr = NULL;
    }
    bsr_key = -1;
    bsr_id = -1;
}

SharedArray::RC Bsr::Init(const unsigned int member_cnt,
        const unsigned int key, const bool is_leader)
{
#ifdef _LAPI_LINUX
    ITRC(IT_BSR, "Bsr: BSR is not supported on LINUX\n");
    return NOT_AVAILABLE;
#endif

    RC              rc;
    BSR_SETUP_STATE state;

    // if it is already initialized, then do nothing.
    if (status == READY) {
        ITRC(IT_BSR, "Bsr: Already initialized with %d members\n",
                member_cnt);
        return SUCCESS;
    }

    // Check availability of BSR hardware.
    struct vminfo bsr_info = {0};
    if (vmgetinfo(&bsr_info, VMINFO, sizeof(bsr_info)) != 0) {
        perror("vmgetinfo() unexpectedly failed");
        return FAILED;
    }

    // Check availability of BSR memory.
    if (bsr_info.bsr_mem_free < PAGESIZE) {
        ITRC(IT_BSR, "Bsr: initializing with %d members\n", member_cnt);
        return FAILED;
    } else {
        ITRC(IT_BSR, "Bsr: %u bytes available BSR memory\n",
                bsr_info.bsr_mem_free);
    }

    this->is_leader  = is_leader;
    this->member_cnt = member_cnt;
    ITRC(IT_BSR, "Bsr: initializing with %d members\n", member_cnt);

    // Allocate and setup one shared memory block for communicating
    // among the tasks on the same node.
    rc = ShmSetup(key, sizeof(Shm), 300);
    if (rc != SUCCESS) {
        if (is_leader)
            ITRC(IT_BSR, "Bsr: leader ShmSetup with key %u failed\n", key);
        else
            ITRC(IT_BSR, "Bsr: non-leader ShmSetup with key %u failed\n", key);

        CleanUp();
        return rc;
    }

    // attach the shared memory
    shm = (Shm *)shm_seg;

    bsr_size = PAGESIZE;
    if (is_leader) {
        // generate a unique key for BSR region
        FILE *pf;
        char tok_path[64];
        sprintf(tok_path, "/tmp/bsr_%u", key);
        pf = fopen(tok_path, "w");
        if (pf != NULL) {
            ITRC(IT_BSR, "Bsr: BSR create token file passed at %s\n", tok_path);
            fclose(pf);
        } else {
            ITRC(IT_BSR, "Bsr: BSR create token file failed at %s\n", tok_path);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        }

        bsr_key = ftok(tok_path, key);
        remove(tok_path);
        if (bsr_key != -1) {
            ITRC(IT_BSR, "Bsr: create BSR key from token file passed at %s\n", tok_path);
        } else {
            ITRC(IT_BSR, "Bsr: create BSR key from token file failed at %s\n", tok_path);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        }

        // create shared memory
        bsr_id = shmget(bsr_key, bsr_size, IPC_CREAT|IPC_EXCL|0600);
        if (bsr_id != -1) { // allocation succeeded
            ITRC(IT_BSR, "Bsr: BSR master (BSR key=0x%x, BSR id=%d) allocation passed.\n",
                    bsr_key, bsr_id);
        } else { // allocation failed
            ITRC(IT_BSR, "Bsr: shmget to allocate BSR region failed with errno %u.\n",
                    errno);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        }

        // request BSR memory for the shm region
        if (shmctl(bsr_id, SHM_BSR, NULL)) {
            ITRC(IT_BSR, "Bsr: shmctl(SHM_BSR) failed with errno=%u\n", errno);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        } else {
            ITRC(IT_BSR, "Bsr: shmctl(SHM_BSR) passed.\n");
        }

        // attach to BSR shm region
        bsr_addr = (unsigned char*)shmat(bsr_id, 0, 0);
        if (bsr_addr != NULL) {
            ITRC(IT_BSR, "Bsr: leader BSR attatch passed at 0x%p\n", bsr_addr);
        } else {
            ITRC(IT_BSR, "Bsr: leader BSR attatch failed at 0x%p\n", bsr_addr);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        }

        // update reference count and BSR state
        is_bsr_attached = true;
        shm->bsr_id = bsr_id;
        shm->bsr_setup_ref = 1;
        shm->bsr_setup_state = ST_ATTACHED;
        ITRC(IT_BSR, "Bsr: leader BSR setup passed (bsr_setup_ref=%d)\n",
                shm->bsr_setup_ref);
    } else {
        // waiting for leader to change the setup_state to ST_ATTACHED or
        // ST_FAILE
        state = shm->bsr_setup_state;
        if (progress_cb) {
            while (ST_ATTACHED != state && ST_FAIL != state) {
                (*progress_cb)(progress_cb_info);
                state = shm->bsr_setup_state;
            }
        } else {
            while (ST_ATTACHED != state && ST_FAIL != state) {
                state = shm->bsr_setup_state;
            }
        }

        if (ST_FAIL == state) {
            ITRC(IT_BSR, "Bsr: non-leader BSR setup failed, abording ...\n");
            CleanUp();
            return FAILED;
        }

        // attach to BSR shm region
        bsr_id = shm->bsr_id;
        bsr_addr = (unsigned char*)shmat(bsr_id, 0, 0);
        if (bsr_addr != NULL) {
            ITRC(IT_BSR, "Bsr: non-leader BSR attatch passed at 0x%p\n", bsr_addr);
        } else {
            ITRC(IT_BSR, "Bsr: non-leader BSR attatch failed at 0x%p\n", bsr_addr);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        }

        // increase the count
        is_bsr_attached = true;
        fetch_and_add ((atomic_p)&shm->bsr_setup_ref, 1);
        ITRC(IT_BSR, "Bsr: non-leader BSR setup passed (bsr_setup_ref=%d)\n",
                shm->bsr_setup_ref);
    }

    // wait everyone reaches here; also check if there is any task failed
    unsigned int cnt = shm->bsr_setup_ref;
    state = shm->bsr_setup_state;
    if (progress_cb) {
        while (cnt < member_cnt && ST_FAIL != state) {
            (*progress_cb)(progress_cb_info);
            state = shm->bsr_setup_state;
            cnt = shm->bsr_setup_ref;
        }
    } else {
        while (cnt < member_cnt && ST_FAIL != state) {
            state = shm->bsr_setup_state;
            cnt = shm->bsr_setup_ref;
        }
    }

    if (state == ST_FAIL) {
        ITRC(IT_BSR, "Bsr: BSR setup failed; abording ...\n");
        CleanUp();
        return FAILED;
    }

    ITRC(IT_BSR, "Bsr: initialied successfully\n");
    status = READY;
    return SUCCESS;
}

unsigned char      Bsr::Load1(const int offset) const
{
    return bsr_addr[offset];
}

unsigned short     Bsr::Load2(const int offset) const
{
    return ((unsigned short*)(bsr_addr))[offset];
}

unsigned int       Bsr::Load4(const int offset) const
{
    return ((unsigned int*)(bsr_addr))[offset];
}

unsigned long long Bsr::Load8(const int offset) const
{
    return ((unsigned long long*)(bsr_addr))[offset];
}

void Bsr::Store1(const int offset, const unsigned char val)
{
    mem_isync();
    bsr_addr[offset] = val;
    mem_barrier();
}

void Bsr::Store2(const int offset, const unsigned short val)
{
    mem_isync();
    ((unsigned short*)bsr_addr)[offset] = val;
    mem_barrier();
}

void Bsr::Store4(const int offset, const unsigned int val)
{
    assert(0 && "Bsr::Store4() not supported");
}

void Bsr::Store8(const int offset, const unsigned long long val)
{
    assert(0 && "Bsr::Store8() not supported");
}
