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
#include <limits.h>
#include <time.h>
#ifdef _LAPI_LINUX
#include "lapi_linux.h"
#endif /* _LAPI_LINUX */
#include "atomics.h"
#include "Bsr.h"
#include "lapi_itrace.h"
#include "lapi_assert.h"
#include "Arch.h"


#ifdef _LAPI_LINUX
const char BSR_LIB_NAME[] = "libbsr.so";
typedef int   (*bsr_query_t) (unsigned*, uint32_t*, unsigned*, uint32_t*);
typedef int   (*bsr_alloc_t) (unsigned, unsigned*, unsigned*);
typedef int   (*bsr_free_t)  (unsigned);
typedef void* (*bsr_map_t)   (void*, unsigned, int, int, unsigned*);

// Internal helper function to display dynamic linking errors.
static void show_dlerror(const char* msg) {
    const char* msg_to_use = (msg == NULL)?"Dynamic linking error":msg;
    char* err_str = dlerror();
    ITRC(IT_BSR, "%s (%s)\n", msg_to_use, (err_str == NULL)?"unknown":err_str);
}

class BsrFunc {
    public:
        static bool loaded;
        BsrFunc() {
            bsr_query_t bsr_query = NULL;
            bsr_alloc_t bsr_alloc = NULL;
            bsr_free_t  bsr_free  = NULL;
            bsr_map_t   bsr_map   = NULL;
        };
        bool LoadFunc() {
            if (BsrFunc::loaded) return true;
            void* hndl = dlopen(BSR_LIB_NAME, RTLD_NOW|RTLD_GLOBAL);
            if (hndl == NULL) {
                show_dlerror("BsrFunc: dlopen failed");
                return false;
            }
            bsr_query = (bsr_query_t) dlsym(hndl, "bsr_query");
            if (NULL == bsr_query) {
                show_dlerror("BsrFunc: dlsym bsr_query failed");
                dlclose(hndl);
                return false;
            }
            bsr_alloc = (bsr_alloc_t) dlsym(hndl, "bsr_alloc");
            if (NULL == bsr_alloc) {
                show_dlerror("BsrFunc: dlsym bsr_alloc failed");
                dlclose(hndl);
                return false;
            }
            bsr_free = (bsr_free_t) dlsym(hndl, "bsr_free");
            if (NULL == bsr_free) {
                show_dlerror("BsrFunc: dlsym bsr_free failed");
                dlclose(hndl);
                return false;
            }
            bsr_map = (bsr_map_t) dlsym(hndl, "bsr_map");
            if (NULL == bsr_map) {
                show_dlerror("BsrFunc: dlsym bsr_map failed");
                dlclose(hndl);
                return false;
            }
            loaded = true;
            return true;
        };
        bsr_query_t bsr_query;
        bsr_alloc_t bsr_alloc;
        bsr_free_t  bsr_free;
        bsr_map_t   bsr_map;
} __bsr_func;
bool BsrFunc::loaded = false;
#endif /* _LAPI_LINUX */
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

#ifdef _LAPI_LINUX
    if (bsr_addr) {
        assert(shm != NULL);
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
            (*__bsr_func.bsr_free)(bsr_id);
        }
        is_bsr_attached = false;
        bsr_addr = NULL;
    }
    ShmDestroy();
    shm = NULL;
#else
    if (bsr_addr) {
        // do we need synchronize here?
        // to make sure the state update to SHM is done before
        // we destroy the SHM.
        mem_barrier();
        mem_isync();
        // detach from bsr, if attached before
        if (is_bsr_attached) {
            shmdt(bsr_addr);
        }
        // decrease the bsr_setup_ref from shm
        fetch_and_add((atomic_p)&shm->bsr_setup_ref, -1);

        bsr_addr = NULL;
        is_bsr_attached = false;
    }

    if (bsr_id != -1) {
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
    }
#if 0
    ShmDestroy();
#else
    PosixShmDestroy();
#endif
    shm = NULL;
#endif
    bsr_key = -1;
    bsr_id = -1;
}

SharedArray::RC Bsr::Init(const unsigned int mem_cnt,
        const unsigned int group_id, const unsigned int job_key,
        const bool leader, const int mem_id, const unsigned char init_val)
{
    RC              rc;
    BSR_SETUP_STATE state;

    // if it is already initialized, then do nothing.
    if (status == READY) {
        ITRC(IT_BSR, "Bsr: Already initialized with %d members\n",
                mem_cnt);
        return SUCCESS;
    }

    this->is_leader  = leader;
    this->member_cnt = mem_cnt;

    // Workaround before PNSD could provide unique keys
    char key_str[64];
    sprintf(key_str, "/BSR_shm_%d_%d", job_key, group_id);
    ITRC(IT_BSR, "Bsr::Init Unique key string for BSR bootstrap <%s>\n", key_str);

#ifdef _LAPI_LINUX 
    if (! __bsr_func.LoadFunc())
        return NOT_AVAILABLE;

    unsigned avail, total;
    uint32_t smask, fmask;
    int      libbsr_rc = 0;

    libbsr_rc = (*(__bsr_func.bsr_query))(&total, &smask, &avail, &fmask);
    if (0 != libbsr_rc || !fmask || total == 0 || avail < mem_cnt) {
        ITRC(IT_BSR, "BSR: no free BSRs total=%u smask=0x%x, avail=%u, fmask=0x%x, libbsr_rc=%d\n",
                total, smask, avail, fmask, libbsr_rc);
        return FAILED;
    } else {
        ITRC(IT_BSR, "BSR: bsr_query total=%u smask=0x%x, avail=%u, fmask=0x%x, libbsr_rc=%d\n",
                total, smask, avail, fmask, libbsr_rc);
    }
#else 
    // Check availability of BSR hardware.
    struct vminfo bsr_info = {0};
    if (vmgetinfo(&bsr_info, VMINFO, sizeof(bsr_info)) != 0) {
        perror("vmgetinfo() unexpectedly failed");
        return FAILED;
    }

    // determine BSR region size to allocate
    bsr_size = (mem_cnt%PAGESIZE) ? (mem_cnt/PAGESIZE)*PAGESIZE+PAGESIZE : mem_cnt;

    // Check availability of BSR memory and number of tasks
    if (bsr_info.bsr_mem_free < bsr_size) { 
        ITRC(IT_BSR, "Bsr: request %d bytes with %d members %lu bytes available BSR memory\n", 
                bsr_size, mem_cnt, bsr_info.bsr_mem_free);
        return FAILED;
    } else {
        ITRC(IT_BSR, "Bsr: request %d bytes with %d members %lu bytes available BSR memory\n",
                bsr_size, mem_cnt, bsr_info.bsr_mem_free);
    }
#endif /* _LAPI_LINUX */

    // Allocate and setup one shared memory block for communicating
    // among the tasks on the same node.
    // Workaround before PNSD could provide unique keys
#if 0
    rc = ShmSetup(key, sizeof(Shm), 300);
#else
    rc = PosixShmSetup(key_str, sizeof(Shm), 300);
#endif
    if (rc != SUCCESS) {
        if (leader) {
            ITRC(IT_BSR, "Bsr: leader ShmSetup with key %s failed rc %d\n",
                    key_str, rc);
        } else {
            ITRC(IT_BSR, "Bsr: non-leader ShmSetup with key %s failed\n",
                    key_str);
        }

        CleanUp();
        return rc;
    }

    // attach the shared memory
    shm = (Shm *)shm_seg;

#ifdef _LAPI_LINUX
    if (leader) {
        unsigned alloc_sz = 0;
        /* find the smallest BSR that larger than mem_cnt */
        for (int i = 0; i < (sizeof(fmask) * CHAR_BIT); ++i) {
            unsigned cur_sz = 1 << i;
            if (cur_sz & fmask && mem_cnt <= cur_sz) {
                alloc_sz = cur_sz;
                break;
            }
        }
        unsigned bsr_stride;
        libbsr_rc = (*(__bsr_func.bsr_alloc))(alloc_sz, &bsr_stride, (unsigned*)&bsr_id);
        if (libbsr_rc) {
            ITRC(IT_BSR, "BSR: bsr_alloc failed with rc = %d\n", libbsr_rc);
            shm->bsr_setup_state = ST_FAIL;
            return FAILED;
        }
        ITRC(IT_BSR, "BSR: (leader) bsr_alloc got bsr_id=%u stride=%u\n",
                bsr_id, bsr_stride);
        unsigned bsr_length;
        bsr_addr = (unsigned char *)(*(__bsr_func.bsr_map))(NULL, (unsigned)bsr_id, 0, 0, &bsr_length);
        if (NULL == bsr_addr || bsr_length < mem_cnt) {
            shm->bsr_setup_state = ST_FAIL;
            ITRC(IT_BSR, "BSR: (leader) bsr_map failed with bsr_id=%u bsr_length=%u\n",
                    bsr_id, bsr_length);
            /* clean up */
            (*(__bsr_func.bsr_free))((unsigned)bsr_id);
            return FAILED;
        }
        ITRC(IT_BSR, "BSR: (leader) bsr_map returns bsr_addr=%p bsr_length=%u\n",
                bsr_addr, bsr_length);
        // update BSR state
        shm->bsr_id = bsr_id;
        shm->bsr_setup_state = ST_ATTACHED;
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
            ITRC(IT_BSR, "Bsr: non-leader BSR setup failed, aborting ...\n");
            // nothing need to be cleaned
            return FAILED;
        }

        bsr_id = shm->bsr_id;

        unsigned bsr_length;
        bsr_addr = (unsigned char *)(*(__bsr_func.bsr_map))(NULL, (unsigned)bsr_id, 0, 0, &bsr_length);
        if (NULL == bsr_addr || bsr_length < mem_cnt) {
            shm->bsr_setup_state = ST_FAIL;
            ITRC(IT_BSR, "BSR: (non-leader) bsr_map failed with bsr_id=%u bsr_length=%u\n",
                    bsr_id, bsr_length);
            /* nothing need to be cleaned here */
            /* bsr_id will be freed by leader */
            return FAILED;
        }
        ITRC(IT_BSR, "BSR: (non-leader) bsr_map returns bsr_addr=%p bsr_length=%u\n",
                bsr_addr, bsr_length);
    }
#else  /* AIX flow */
    if (leader) {
        // generate a unique key for BSR region
        int num_try = 0;
        do {
            srand( time(NULL) );
            bsr_key = rand();
            bsr_id = shmget(bsr_key, bsr_size, IPC_CREAT|IPC_EXCL|0600);
            num_try ++;
        } while (bsr_id == -1 && num_try < 100);

        if (bsr_id != -1) {
            ITRC(IT_BSR, "Bsr: BSR master (BSR key=0x%x, BSR id=%d) allocation passed after %d tries.\n",
                    bsr_key, bsr_id, num_try);
        } else {
            ITRC(IT_BSR, "Bsr: shmget to allocate BSR region failed with errno %d after %d tries.\n",
                    errno, num_try);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        }

        // request BSR memory for the shm region
        if (shmctl(bsr_id, SHM_BSR, NULL)) {
            ITRC(IT_BSR, "Bsr: shmctl(SHM_BSR) failed with errno=%u BSR id %u\n", 
                    errno, bsr_id);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        } else {
            ITRC(IT_BSR, "Bsr: shmctl(SHM_BSR) passed with BSR id %u.\n",
                    bsr_id);
        }

        // attach to BSR shm region
        bsr_addr = (unsigned char *)shmat(bsr_id, 0, 0);
        if (bsr_addr != (unsigned char*)-1) {
            ITRC(IT_BSR, "Bsr: leader BSR attatch with BSR id %u passed at 0x%p\n", 
                    bsr_id, bsr_addr);
        } else {
            ITRC(IT_BSR, "Bsr: leader BSR attatch with BSR id %u failed at 0x%p with errno %d\n", 
                    bsr_id, bsr_addr, errno);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        }

        // update BSR state
        shm->bsr_id = bsr_id;
        shm->bsr_setup_state = ST_ATTACHED;
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
            ITRC(IT_BSR, "Bsr: non-leader BSR setup failed, aborting ...\n");
            CleanUp();
            return FAILED;
        }

        bsr_id = shm->bsr_id;

        // attach to BSR shm region
        bsr_addr = (unsigned char*)shmat(bsr_id, 0, 0);
        if (bsr_addr != (unsigned char*)-1) {
            ITRC(IT_BSR, "Bsr: non-leader BSR attatch with BSR id %u passed at 0x%p\n", 
                    bsr_id, bsr_addr);
        } else {
            ITRC(IT_BSR, "Bsr: non-leader BSR attatch with BSR id %u failed at 0x%p with errno %d\n", 
                    bsr_id, bsr_addr, errno);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return FAILED;
        }
    }
#endif /* _LAPI_LINUX */

    // Init and priming BSR shm region properly, ready to do barrier
    Store1(mem_id, init_val);
    unsigned char tmp_val = Load1(mem_id);
    assert(init_val == tmp_val);

    // increase ref count
    is_bsr_attached = true;
    fetch_and_add ((atomic_p)&shm->bsr_setup_ref, 1);
    ITRC(IT_BSR, "Bsr: %s BSR setup passed (bsr_setup_ref=%d)\n",
            (is_leader)?"(master)":"(slave)", shm->bsr_setup_ref);

    // wait everyone reaches here; also check if there is any task failed
    unsigned int cnt = shm->bsr_setup_ref;
    state = shm->bsr_setup_state;
    if (progress_cb) {
        while (cnt < mem_cnt && ST_FAIL != state) {
            (*progress_cb)(progress_cb_info);
            state = shm->bsr_setup_state;
            cnt = shm->bsr_setup_ref;
        }
    } else {
        while (cnt < mem_cnt && ST_FAIL != state) {
            state = shm->bsr_setup_state;
            cnt = shm->bsr_setup_ref;
        }
    }

    if (state == ST_FAIL) {
        ITRC(IT_BSR, "Bsr: BSR setup failed; aborting ...\n");
        CleanUp();
        return FAILED;
    }

#ifndef _LAPI_LINUX
    if (leader) {
        shmctl(bsr_id, IPC_RMID, 0); 
        PosixShmDestroy();
    }
#endif /* _LAPI_LINUX */
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
