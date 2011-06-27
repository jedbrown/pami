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
#include "Arch.h"
#include "dlpnsd_lib.h"
#include "util/common.h"

using namespace PNSDapi;

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
Bsr::Bsr() :
    bsr_key(-1),
    bsr_id(-1),
    bsr_size(0),
    bsr_addr(NULL),
    bsr_state(ST_NONE){};

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

#ifdef _LAPI_LINUX
    if (bsr_addr) {
        assert(shm != NULL);

        if (is_leader) {
            (*__bsr_func.bsr_free)(bsr_id);
        }
        bsr_addr = NULL;
    }
#else
    if (bsr_addr) {
        mem_barrier();
        mem_isync();
        // detach from bsr, if attached before
        shmdt(bsr_addr);
        bsr_addr = NULL;
    }
    if (is_leader && bsr_id != -1) {
        shmctl(bsr_id, IPC_RMID, 0); 
    }
#endif
    shm     = NULL;
    bsr_key = -1;
    bsr_id  = -1;
}

int Bsr::GetBsrUniqueKey(const unsigned int j_key)
{
    int             pnsd_handle, rc, key_rc;
    char            *name = "key.shmv";
    pnsd_key_t      key;
    open_flags_t    flags = NTBL_UPDATES;

    rc = papi_open(&pnsd_handle, flags);
    if( rc != 0 ) {
        ITRC(IT_BSR, "Bsr::GetBsrUniqueKey() pnsd_api_open failed, rc = %d\n", rc);
        return rc;
    }

    key_rc = papi_get_keys(pnsd_handle, j_key, name, 1, &key);
    if( key_rc != 0 ) {
        ITRC(IT_BSR, "Bsr::GetBsrUniqueKey() pnsd_api_get_keys failed, rc = %d\n",
                key_rc);
    } else {
        bsr_key = (key_t)key;
    }

    rc = papi_close(pnsd_handle);
    if ( rc != 0 ) {
        ITRC(IT_BSR, "Bsr::GetBsrUniqueKey() pnsd_api_close failed with rc = %d\n", rc);
    }

    ITRC(IT_BSR, "Bsr::GetBsrUniqueKey() pnsd_api_get_keys succeeded with key (0x%x)\n",
            key);
    return key_rc;
}

Bsr::BSR_SETUP_STATE Bsr::CheckBsrReady()
{
    // Check if all tasks have update the reference count, or
    // if any task failed

    if (this->member_cnt == shm->bsr_setup_ref && 
            shm->bsr_setup_state == ST_BSR_ATTACHED) 
    {
        ITRC(IT_BSR, "Bsr: (%s) BSR setup passed (bsr_setup_ref=%d)\n",
                (this->is_leader?"master":"slave"), shm->bsr_setup_ref);
        ITRC(IT_BSR, "Bsr: initialied successfully\n");
#ifndef _LAPI_LINUX
        if (this->is_leader) {
            shmctl(this->bsr_id, IPC_RMID, 0); 
        }
#endif /* _LAPI_LINUX */
        return ST_SUCCESS;
    } else if (shm->bsr_setup_state == ST_FAIL) {
        ITRC(IT_BSR, "Bsr: BSR setup failed; aborting ...\n");
        fetch_and_add ((atomic_p)&shm->bsr_setup_ref, -1);
        CleanUp();
        return ST_FAIL;
    } else {
        return ST_BSR_CHECK_REF_CNT;  
    }
}

Bsr::BSR_SETUP_STATE Bsr::CheckBsrRefCount(const int mem_id, const unsigned char init_val) 
{
    // Init and priming BSR shm region properly, ready to do barrier
    Store1(mem_id, init_val);
    unsigned char tmp_val = Load1(mem_id);
    assert(init_val == tmp_val);

    // increase ref count
    fetch_and_add ((atomic_p)&shm->bsr_setup_ref, 1);

    return CheckBsrReady();
}

Bsr::BSR_SETUP_STATE Bsr::CheckBsrAttach(const unsigned int job_key) 
{
#ifdef _LAPI_LINUX
    if (this->is_leader) {
        unsigned alloc_sz = this->bsr_size;
        unsigned bsr_stride;
        int      libbsr_rc = 0;
        libbsr_rc = (*(__bsr_func.bsr_alloc))(alloc_sz, &bsr_stride, (unsigned*)&bsr_id);
        if (libbsr_rc) {
            ITRC(IT_BSR, "BSR: bsr_alloc failed with rc = %d\n", libbsr_rc);
            shm->bsr_setup_state = ST_FAIL;
            return ST_FAIL;
        }
        ITRC(IT_BSR, "BSR: (leader) bsr_alloc got bsr_id=%u stride=%u\n",
                bsr_id, bsr_stride);
        unsigned bsr_length;
        bsr_addr = (unsigned char *)(*(__bsr_func.bsr_map))(NULL, (unsigned)bsr_id, 0, 0, &bsr_length);
        if (NULL == bsr_addr || bsr_length < this->member_cnt) {
            shm->bsr_setup_state = ST_FAIL;
            ITRC(IT_BSR, "BSR: (leader) bsr_map failed with bsr_id=%u bsr_length=%u\n",
                    bsr_id, bsr_length);
            /* clean up */
            (*(__bsr_func.bsr_free))((unsigned)bsr_id);
            return ST_FAIL;
        }
        ITRC(IT_BSR, "BSR: (leader) bsr_map returns bsr_addr=%p bsr_length=%u\n",
                bsr_addr, bsr_length);

        // update BSR state
        shm->bsr_id = bsr_id;
        shm->bsr_setup_state = ST_BSR_ATTACHED;

        return ST_BSR_ATTACHED;
    } else {
        // check if bsr_setup_state in bootstrap shm is ST_ATTACHED or ST_FAIL
        switch (shm->bsr_setup_state) {
            case ST_BSR_ATTACHED:
                bsr_id = shm->bsr_id;
                unsigned bsr_length;
                bsr_addr = (unsigned char *)(*(__bsr_func.bsr_map))(NULL, (unsigned)bsr_id, 0, 0, &bsr_length);
                if (NULL == bsr_addr || bsr_length < this->member_cnt) {
                    shm->bsr_setup_state = ST_FAIL;
                    ITRC(IT_BSR, 
                            "BSR: (non-leader) bsr_map failed with bsr_id=%u bsr_length=%u\n",
                            bsr_id, bsr_length);
                    /* nothing need to be cleaned here */
                    /* bsr_id will be freed by leader */
                    return ST_FAIL;
                }
                ITRC(IT_BSR, "BSR: (non-leader) bsr_map returns bsr_addr=%p bsr_length=%u\n",
                        bsr_addr, bsr_length);
                return ST_BSR_ATTACHED;
            case ST_FAIL:
                ITRC(IT_BSR, "Bsr: non-leader BSR setup failed, aborting ...\n");
                // nothing need to be cleaned
                return ST_FAIL;
            default:
                return ST_BOOTSTRAP_DONE;
        }
    }
#else  /* AIX flow */
    if (this->is_leader) {
        int key_rc = GetBsrUniqueKey(job_key);
        if (key_rc == 0) {
            bsr_id = shmget(bsr_key, bsr_size, IPC_CREAT|IPC_EXCL|0600);
            if (bsr_id != -1) {
                ITRC(IT_BSR, "Bsr: BSR master shmget passed with (BSR key=0x%x, BSR id=%d)\n",
                        bsr_key, bsr_id);
            } else {
                ITRC(IT_BSR, "Bsr: BSR master shmget failed with (BSR key=0x%x, errno=%d)\n",
                        bsr_key, errno);
                shm->bsr_setup_state = ST_FAIL;
                return ST_FAIL;
            }
        } else {
            ITRC(IT_BSR, "Bsr: BSR master failed to get key from PNSD (%d)\n",
                    key_rc);
            shm->bsr_setup_state = ST_FAIL;
            return ST_FAIL;
        }

        // request BSR memory for the shm region
        if (shmctl(bsr_id, SHM_BSR, NULL)) {
            ITRC(IT_BSR, "Bsr: shmctl(SHM_BSR) failed with errno=%u BSR id %u\n", 
                    errno, bsr_id);
            shm->bsr_setup_state = ST_FAIL;
            CleanUp();
            return ST_FAIL;
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
            return ST_FAIL;
        }

        // update BSR state
        shm->bsr_id = bsr_id;
        shm->bsr_setup_state = ST_BSR_ATTACHED;

        return ST_BSR_ATTACHED;
    } else {
        // check leader's bsr_setup_state in bootstrap shm is ST_ATTACHED or ST_FAIL
        switch (shm->bsr_setup_state) {
            case ST_BSR_ATTACHED:
                bsr_id = shm->bsr_id;
                // attach to BSR shm region
                bsr_addr = (unsigned char*)shmat(bsr_id, 0, 0);
                if (bsr_addr != (unsigned char*)-1) {
                    ITRC(IT_BSR, 
                            "Bsr: non-leader BSR attatch with BSR id %u passed at 0x%p\n", 
                            bsr_id, bsr_addr);
                    return ST_BSR_ATTACHED;
                } else {
                    ITRC(IT_BSR, 
                            "Bsr: non-leader BSR attatch with BSR id %u failed "
                            "at 0x%p with errno %d\n", 
                            bsr_id, bsr_addr, errno);
                    shm->bsr_setup_state = ST_FAIL;
                    return ST_FAIL;
                }
            case ST_FAIL:
                ITRC(IT_BSR, "Bsr: non-leader BSR setup failed, aborting ...\n");
                CleanUp();
                return ST_FAIL;
            default:
                return ST_BOOTSTRAP_DONE;
        }
    }
#endif /* _LAPI_LINUX */
}

Bsr::BSR_SETUP_STATE Bsr::CheckBootstrapSetup()
{
    RC rc = IsPosixShmSetupDone(sizeof(Shm));
    switch (rc) {
        case SUCCESS:
            // attach the shared memory
            shm = (Shm *)shm_seg;

            ITRC(IT_BSR, "Bsr: %s Bootstrap_ShmSetup with key <%s> PASSED.\n",
                    (this->is_leader)?"LEADER":"FOLLOWER",
                    shm_str->c_str());

            return ST_BOOTSTRAP_DONE;
        case PROCESSING:
            return ST_BOOTSTRAP_PROCESSING;
        default:
            ITRC(IT_BSR, "Bsr: %s Bootstrap_ShmSetup with key <%s> FAILED with"
                    " rc %d\n",
                    (this->is_leader)?"LEADER":"FOLLOWER",
                    shm_str->c_str(), rc);
            return ST_FAIL;
    }
}

Bsr::BSR_SETUP_STATE Bsr::CheckBsrResource(const unsigned int mem_cnt,
        const unsigned int job_key, const uint64_t unique_key, const bool leader)
{
    // Check availability of BSR resources.
#ifdef _LAPI_LINUX 
    if (! __bsr_func.LoadFunc())
        return ST_FAIL;

    unsigned avail, total;
    int      libbsr_rc = 0;

    libbsr_rc = (*(__bsr_func.bsr_query))(&total, &smask, &avail, &fmask);
    if (0 != libbsr_rc || !fmask || total == 0 || avail < mem_cnt) {
        ITRC(IT_BSR, "BSR: no free BSRs total=%u smask=0x%x, avail=%u, fmask=0x%x, libbsr_rc=%d\n",
                total, smask, avail, fmask, libbsr_rc);
        return ST_FAIL;
    } else {
        ITRC(IT_BSR, "BSR: bsr_query total=%u smask=0x%x, avail=%u, fmask=0x%x, libbsr_rc=%d\n",
                total, smask, avail, fmask, libbsr_rc);
    }

    for (unsigned req_size = 1; req_size <= avail; req_size = req_size << 1) {
        if ((req_size & fmask) == 0)
            continue;
        if (req_size < mem_cnt)
            continue;
        else {
            this->bsr_size = req_size;
            break;
        }
    }

    /* not enough bsr */
    ITRC(IT_BSR, "Bsr: request %d bytes with %d members %lu bytes available BSR memory\n",
            this->bsr_size, mem_cnt, avail);
    if (this->bsr_size == 0)
        return ST_FAIL;
#else 
    struct vminfo bsr_info = {0};
    if (vmgetinfo(&bsr_info, VMINFO, sizeof(bsr_info)) != 0) {
        ITRC(IT_BSR, "Bsr: vmgetinfo() failed\n");
        perror("vmgetinfo() unexpectedly failed");
        return ST_FAIL;
    }

    // determine BSR region size to allocate
    this->bsr_size = (mem_cnt%PAGESIZE) ? (mem_cnt/PAGESIZE)*PAGESIZE+PAGESIZE : mem_cnt;

    // Check availability of BSR memory and number of tasks
    ITRC(IT_BSR, "Bsr: request %d bytes with %d members %lu bytes available BSR memory\n",
            this->bsr_size, mem_cnt, bsr_info.bsr_mem_free);
    if (bsr_info.bsr_mem_free < bsr_size) { 
        return ST_FAIL;
    }
#endif /* _LAPI_LINUX */

    this->is_leader     = leader;
    this->member_cnt    = mem_cnt;

    char tmp[128];
    sprintf(tmp, "/BSR_shm_%d_%llu", job_key, unique_key);
    shm_str = new string(tmp);
    ITRC(IT_BSR, "Bsr::Init Unique key string for BSR bootstrap <%s>\n",
            shm_str->c_str());
    return CheckBootstrapSetup();
}

SharedArray::RC Bsr::CheckInitDone(const unsigned int   mem_cnt, 
                                   const unsigned int   job_key, 
                                   const uint64_t       unique_key,
                                   const bool           leader, 
                                   const int            mem_id, 
                                   const unsigned char  init_val)
{
    RC              rc;
    bool advance = true;
    while (advance) {
        advance = false;
        switch (bsr_state) {
            case ST_NONE:
                bsr_state = CheckBsrResource(mem_cnt, 
                                             job_key, 
                                             unique_key, 
                                             leader);
                advance = (bsr_state != ST_NONE);
                break;
            case ST_BOOTSTRAP_PROCESSING:
                bsr_state = CheckBootstrapSetup();
                advance = (bsr_state != ST_BOOTSTRAP_PROCESSING);
                break;
            case ST_BOOTSTRAP_DONE:
                bsr_state = CheckBsrAttach(job_key);
                advance = (bsr_state != ST_BOOTSTRAP_DONE);
                break;
            case ST_BSR_ATTACHED:
                bsr_state = CheckBsrRefCount(mem_id, init_val);
                advance = (bsr_state != ST_BSR_ATTACHED);
                break;
            case ST_BSR_CHECK_REF_CNT:
                bsr_state = CheckBsrReady();
                advance = (bsr_state != ST_BSR_CHECK_REF_CNT);
                break;
            case ST_FAIL:
                return FAILED;
            case ST_SUCCESS:
                return SUCCESS;
        }
    }

    return PROCESSING;
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
