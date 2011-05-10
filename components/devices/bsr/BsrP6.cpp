/*!
 * \file BsrP6.cpp
 *
 * \brief Implementation of BsrP6 class.
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
#include "BsrP6.h"
#include "lapi_itrace.h"
#include "Arch.h"

/*!
 * \brief The name of BSR library to use.
 */
const char* BSRP6_KE_LOADABLE = "libbsr.a(shr_64.o)";

bool BsrP6::bsr_func_loaded = false;
BsrP6::_bsr_func_t BsrP6::bsr_func;

/*!
 * \brief Default constructor.
 */
BsrP6::BsrP6() {
    bsr_fd      = -1;
    bsr_granule = 0;
    bsr_id_cnt  = 0;
    bsr_att_cnt = 0;
    bsr_addr    = NULL;
}

/*!
 * \brief Default destructor.
 */
BsrP6::~BsrP6() {
    ITRC(IT_BSR, "BsrP6: In ~BsrP6()\n");
    CleanUp();
}

void BsrP6::CleanUp()
{
    ITRC(IT_BSR, "BsrP6: In CleanUp()\n");
    // set status
    status = NOT_READY;

    if (!bsr_func_loaded || bsr_fd < 0)
        return;

    bsr_granule = 0;
    if (bsr_addr) {
        if (shm) {
            // do we need synchronize here?
            // to make sure the state update to SHM is done before
            // we destroy the SHM.
            mem_barrier();
            mem_isync();
            // detach from bsr, if attached before
            for (int i = 0; i < bsr_att_cnt; ++i) {
                (*(bsr_func.bsr_detach))(bsr_fd, (void*)(bsr_addr[i]));
                ITRC(IT_BSR,
                        "BsrP6: bsr_detach is done for bsr_addr[%d]=0x%p\n",
                        i, bsr_addr[i]);
            }
            // decrease the bsr_setup_ref from shm
            if (bsr_id_cnt > 0 && bsr_att_cnt == bsr_id_cnt) {
                fetch_and_add((atomic_p)&shm->bsr_setup_ref, -1);
            }
            if (is_leader) {
                // leader has to wait to unallocate BSR IDs before
                // free the SHM segment
                if (progress_cb) {
                    while (shm->bsr_setup_ref > 0) {
                        (*progress_cb)(progress_cb_info);
                    }
                } else while (shm->bsr_setup_ref > 0);

                for (int i = 0; i < bsr_id_cnt; ++i) {
                    (*(bsr_func.bsr_unallocate))(bsr_fd, shm->bsr_ids[i]);
                    ITRC(IT_BSR,
                            "BsrP6: bsr_unallocate is done for bsr_id=%d\n",
                            shm->bsr_ids[i]);
                }
            }
            bsr_att_cnt = bsr_id_cnt = 0;
            ShmDestroy();
            shm = NULL;
        }
        delete [] bsr_addr;
        bsr_addr = NULL;
    }

    (*(bsr_func.bsr_close))(bsr_fd);
    ITRC(IT_BSR, "BsrP6: bsr_close is done for bsr_fd=%d\n", bsr_fd);
    bsr_fd = -1;
}

SharedArray::RC BsrP6::Init(const unsigned int member_cnt,
        const unsigned int key, const unsigned int job_key,
        const bool is_leader, const int member_id, const unsigned char init_val)
{
    RC              rc;
    BSR_SETUP_STATE state;

#ifndef __64BIT__
    ITRC(IT_BSR, "BsrP6: Not supported on 32Bit applications\n");
    return NOT_AVAILABLE;
#endif
#ifdef _LAPI_LINUX
    ITRC(IT_BSR, "BsrP6: BSR is not supported on LINUX\n");
    return NOT_AVAILABLE;
#endif

    // if it is already initialized, then do nothing.
    if (status == READY) {
        ITRC(IT_BSR, "BsrP6: Already initialized with %d members\n",
                member_cnt);
        return SUCCESS;
    }

    this->is_leader  = is_leader;
    this->member_cnt = member_cnt;
    ITRC(IT_BSR, "BsrP6: initializing with %d members\n", member_cnt);

    // Check availability of BSR hardware.
    ///\TODO Do we have other way to check BSR hardware availabity?
    rc = LoadDriverFunc();
    if (SUCCESS != rc) {
        ITRC(IT_BSR, "BsrP6: LoadDriverFunc() failed\n");
        return rc;
    }

    // open BSR device
    bsr_fd = (*(bsr_func.bsr_open))();
    if (bsr_fd < 0) {
        ITRC(IT_BSR, "BsrP6: bsr_open failed\n");
        return FAILED;
    }

    // Query the granuity
    bsr_granule = (*(bsr_func.bsr_query_granule))(bsr_fd);
    if (bsr_granule <= 0) {
        ITRC(IT_BSR, "BsrP6: bsr_granule failed (%d returned)\n",
                bsr_granule);
        return FAILED;
    }

    // calculate the number of BSR IDs needed
    bsr_id_cnt = (member_cnt + bsr_granule - 1)/bsr_granule;
    if (bsr_id_cnt <= 0) {
        ITRC(IT_BSR, "BsrP6: got invalid bsr_id_cnt=%d\n", bsr_id_cnt);
        return FAILED;
    }

    // allocate space to hold bsr addresses
    try {
        bsr_addr = new volatile unsigned char * [bsr_id_cnt];
    } catch (std::bad_alloc) {
        bsr_addr = NULL;
        ITRC(IT_BSR, "BsrP6: Out of memory\n");
        CleanUp();
        return FAILED;
    }

    /*
     * Allocate and setup one shared memory block for communicating
     * among the tasks on the same node.\n
     * The shared memory key is pre-defined.
     */
    unsigned int num_bytes = sizeof(Shm) + bsr_id_cnt * sizeof(int);

    rc = ShmSetup(key, num_bytes, 300);
    if (SUCCESS != rc) {
        ITRC(IT_BSR, "BsrP6: ShmSetup failed\n");
        CleanUp();
        return rc;
    }

    // attach the shared memory
    shm = (Shm *)shm_seg;

    if (is_leader) {
        int allocated_id_cnt = 0;
        // Allocate the BSR memory block, and get BSR IDs
        for (int i = 0; i < bsr_id_cnt; ++i) {
            shm->bsr_ids[i] = (*(bsr_func.bsr_allocate))(bsr_fd, bsr_granule);
            if (-1 != shm->bsr_ids[i]) {
                ++allocated_id_cnt;
                // attach to bsr memory
                bsr_addr[i] = (volatile unsigned char*)
                    (*(bsr_func.bsr_attach))(bsr_fd, shm->bsr_ids[i]);
                if (NULL == bsr_addr[i]) {
                    ITRC(IT_BSR,
                            "BsrP6: bsr_attach failed for bsr_id[%d]=%d\n",
                            i, shm->bsr_ids[i]);
                    shm->bsr_setup_state = ST_FAIL;
                    CleanUp();
                    return FAILED;
                }
                ITRC(IT_BSR,
                        "BsrP6: bsr_attach successed for bsr_id[%d]=%d "
                        "bsr_addr[%d]=%p\n",
                        i, shm->bsr_ids[i], i, bsr_addr[i]);
                ++bsr_att_cnt;
                // initialize BSR memory to 0
                memset((void*)(bsr_addr[i]), 0, bsr_granule);
            } else {
                // when bse_allocate failed. We reset bsr_id_cnt to reflect
                // number of successfully allocated id on leader.
                // CleanUp function needs this to unallocate the BSR IDs.
                bsr_id_cnt = allocated_id_cnt;

                ITRC(IT_BSR, "BsrP6: Failed to get BSR ID\n");
                shm->bsr_setup_state = ST_FAIL;
                CleanUp();
                return FAILED;
            }
        }

        shm->bsr_setup_ref = 1;
        // make sure BSR IDs are set before we change the state
        mem_barrier();

        // move to next state; notify the others that BSR IDs are available
        shm->bsr_setup_state = ST_ATTACHED;
        ITRC(IT_BSR, "BsrP6: leader bsr_setup is done (bsr_setup_ref=%d)\n",
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
            ITRC(IT_BSR, "BsrP6: bsr setup failed; abording ...\n");
            CleanUp();
            return FAILED;
        }

        // make sure we load the BSR IDs after the state is changed
        mem_isync();

        // attach to bsr memory
        for (int i = 0; i < bsr_id_cnt; ++i) {
            bsr_addr[i] = (volatile unsigned char*)
                (*(bsr_func.bsr_attach))(bsr_fd, shm->bsr_ids[i]);
            if (NULL == bsr_addr[i]) {
                ITRC(IT_BSR, "BsrP6: bsr_attach failed for bsr_id[%d]=%d\n",
                        i, shm->bsr_ids[i]);
                shm->bsr_setup_state = ST_FAIL;
                CleanUp();
                return FAILED;
            }
            ITRC(IT_BSR,
                    "BsrP6: bsr_attach successed for bsr_id[%d]=%d "
                    "bsr_addr[%d]=%p\n",
                    i, shm->bsr_ids[i], i, bsr_addr[i]);
            ++bsr_att_cnt;
        }
        // increase the count
        fetch_and_add ((atomic_p)&shm->bsr_setup_ref, 1);
        ITRC(IT_BSR, "BsrP6: follower bsr_setup is done (bsr_setup_ref=%d)\n",
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
        ITRC(IT_BSR, "BsrP6: bsr setup failed; abording ...\n");
        CleanUp();
        return FAILED;
    }

    ITRC(IT_BSR, "BsrP6: initialied successfully\n");
    status = READY;
    return SUCCESS;
}

unsigned char      BsrP6::Load1(const int offset) const
{
    if (bsr_granule > offset) {
        return bsr_addr[0][offset];
    } else {
        return bsr_addr[offset/bsr_granule][offset%bsr_granule];
    }
}

unsigned short     BsrP6::Load2(const int offset) const
{
#ifdef POWER_ARCH
    // maybe not worth to do this in Load2
    /*
    if (bsr_id_cnt == 1 && !(offset & 0x1) && (offset+1)<bsr_granule) {
        return ((unsigned short*)(bsr_addr[0]))[offset];
    } else {
    */
#endif /* POWER_ARCH */
    unsigned short val;
    unsigned char *ptr = (unsigned char*)&val;
    ptr[0] = Load1(offset);
    ptr[1] = Load1(offset+1);
    return val;
#ifdef POWER_ARCH
    /*
    }
    */
#endif /* POWER_ARCH */
}

unsigned int       BsrP6::Load4(const int offset) const
{
#ifdef POWER_ARCH
    if (!(offset & 0x3) && (offset+3)<bsr_granule) {
        return ((unsigned int*)
                (bsr_addr[offset/bsr_granule]))[offset%bsr_granule];
    } else {
#endif /* POWER_ARCH */
        unsigned int val;
        unsigned char *ptr = (unsigned char*)&val;
        ptr[0] = Load1(offset);
        ptr[1] = Load1(offset+1);
        ptr[2] = Load1(offset+2);
        ptr[3] = Load1(offset+3);
        return val;
#ifdef POWER_ARCH
    }
#endif /* POWER_ARCH */
}

unsigned long long BsrP6::Load8(const int offset) const
{
#ifdef POWER_ARCH
    if (!(offset & 0x7) && (offset+7)<bsr_granule) {
        return ((unsigned long long*)
                (bsr_addr[offset/bsr_granule]))[offset%bsr_granule];
    } else {
#endif /* POWER_ARCH */
        unsigned long long val;
        unsigned char *ptr = (unsigned char*)&val;
        ptr[0] = Load1(offset);
        ptr[1] = Load1(offset+1);
        ptr[2] = Load1(offset+2);
        ptr[3] = Load1(offset+3);
        ptr[4] = Load1(offset+4);
        ptr[5] = Load1(offset+5);
        ptr[6] = Load1(offset+6);
        ptr[7] = Load1(offset+7);
        return val;
#ifdef POWER_ARCH
    }
#endif /* POWER_ARCH */
}

void BsrP6::Store1(const int offset, const unsigned char val)
{
    mem_isync();
    if (bsr_granule > offset) {
        bsr_addr[0][offset] = val;
    } else {
        bsr_addr[offset/bsr_granule][offset%bsr_granule] = val;
    }
    mem_barrier();
}

void BsrP6::Store2(const int offset, const unsigned short val)
{
    assert(0 && "BsrP6::Store2() not supported");
}

void BsrP6::Store4(const int offset, const unsigned int val)
{
    assert(0 && "BsrP6::Store4() not supported");
}

void BsrP6::Store8(const int offset, const unsigned long long val)
{
    assert(0 && "BsrP6::Store8() not supported");
}

// Internal helper function to display dynamic linking errors.
static void show_dlerror(const char* msg) {
    const char* msg_to_use = (msg == NULL)?"Dynamic linking error":msg;
    char* err_str = dlerror();
    ITRC(IT_BSR, "%s (%s)\n", msg_to_use, (err_str == NULL)?"unknown":err_str);
}

/*!
 * \brief Load P6 driver functions.
 */
SharedArray::RC BsrP6::LoadDriverFunc() {
    if (bsr_func_loaded)
        return SUCCESS;

    // Load P6 BSR driver functions
#ifdef _LAPI_LINUX
    void* hndl = dlopen(BSRP6_KE_LOADABLE, RTLD_NOW|RTLD_GLOBAL);
#else
    void* hndl = dlopen(BSRP6_KE_LOADABLE, RTLD_NOW|RTLD_GLOBAL|RTLD_MEMBER);
#endif
    if (hndl == NULL) {
        show_dlerror("BsrP6: dlopen failed");
        return FAILED;
    }

    bsr_func.bsr_open = (_bsr_open_t) dlsym(hndl, "bsr_open");
    if (bsr_func.bsr_open == NULL) {
        show_dlerror("BsrP6: dlsym bsr_open failed");
        dlclose(hndl);
        return FAILED;
    }

    bsr_func.bsr_close = (_bsr_close_t) dlsym(hndl, "bsr_close");
    if (bsr_func.bsr_close == NULL) {
        show_dlerror("BsrP6: dlsym bsr_close failed");
        dlclose(hndl);
        return FAILED;
    }

    bsr_func.bsr_allocate = (_bsr_allocate_t) dlsym(hndl, "bsr_allocate");
    if (bsr_func.bsr_allocate == NULL) {
        show_dlerror("BsrP6: dlsym bsr_allocate failed");
        dlclose(hndl);
        return FAILED;
    }

    bsr_func.bsr_unallocate = (_bsr_unallocate_t) dlsym(hndl, "bsr_unallocate");
    if (bsr_func.bsr_unallocate == NULL) {
        show_dlerror("BsrP6: dlsym bsr_unallocate failed");
        dlclose(hndl);
        return FAILED;
    }

    bsr_func.bsr_attach = (_bsr_attach_t) dlsym(hndl, "bsr_attach");
    if (bsr_func.bsr_attach == NULL) {
        show_dlerror("BsrP6: dlsym bsr_attach failed");
        dlclose(hndl);
        return FAILED;
    }

    bsr_func.bsr_detach = (_bsr_detach_t) dlsym(hndl, "bsr_detach");
    if (bsr_func.bsr_detach == NULL) {
        show_dlerror("BsrP6: dlsym bsr_detach failed");
        dlclose(hndl);
        return FAILED;
    }

    bsr_func.bsr_query_granule =
        (_bsr_query_granule_t) dlsym(hndl, "bsr_query_granule");
    if (bsr_func.bsr_query_granule == NULL) {
        show_dlerror("BsrP6: dlsym bsr_query_granule failed");
        dlclose(hndl);
        return FAILED;
    }

    bsr_func_loaded = true;
    return SUCCESS;
}
