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
#include "Memory.h"
#include "dlpnsd_lib.h"
#include "util/common.h"
#include "lapi_assert.h"
#include "lapi_base_type.h"
#include "lapi_env.h"

using namespace PNSDapi;

#ifdef _LAPI_LINUX
const char BSR_LIB_NAME[] = "libbsr.so";
typedef int   (*bsr_query_t) (unsigned*, uint32_t*, unsigned*, uint32_t*);
typedef int   (*bsr_alloc_t) (unsigned, unsigned*, unsigned*);
typedef int   (*bsr_free_t)  (unsigned);
typedef void* (*bsr_map_t)   (void*, unsigned, int, int, unsigned*);
typedef int   (*bsr_unmap_t) (void*, unsigned);

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
            bsr_unmap_t bsr_unmap = NULL;
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
            bsr_unmap = (bsr_unmap_t) dlsym(hndl, "bsr_unmap");
            if (NULL == bsr_unmap) {
                show_dlerror("BsrFunc: dlsym bsr_unmap failed");
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
        bsr_unmap_t bsr_unmap;
} __bsr_func;
bool BsrFunc::loaded = false;
#endif /* _LAPI_LINUX */

/*!
 * \brief Default constructor.
 */
Bsr::Bsr(unsigned int mem_cnt, bool is_leader, void *shm_block, size_t shm_block_sz) :
    SharedArray(mem_cnt, is_leader, shm_block, shm_block_sz, "BSR"),
    bsr_id(-1),
    bsr_addr(NULL),
    bsr_state(ST_NONE),
    shm(NULL),
    is_last(false)
{
    ASSERT(shm_block_sz >= sizeof(Shm));
    shm = (Shm *)shm_seg;
    assert(NULL!=shm);
    size_t align_mask = (sizeof(size_t) - 1);
    /* check alignment for variables that used by atomic */
    assert(((size_t)(&shm->setup_ref) & align_mask) == 0);
#ifdef _LAPI_LINUX
    __bsr_func.LoadFunc();
    bsr_length = 0;
#endif 
    ITRC(IT_BSR,"BSR: in Bsr() shm=0x%p shm_block_sz=%lu shm->bsr_acquired=%d "
            "shm->setup_failed=%d shm->setup_ref=%u shm->bsr_id=%d\n",
            shm, shm_block_sz, shm->bsr_acquired, shm->setup_failed, shm->setup_ref, 
            shm->bsr_id); 
};

/*!
 * \brief Default destructor.
 */
Bsr::~Bsr() {
    ITRC(IT_BSR, "BSR: In ~Bsr()\n");
    CleanUp();
}

void Bsr::ReleaseBsrResource()
{
    ITRC(IT_BSR, "BSR: ReleaseBsrResource() setup_ref = %d bsr_id=%d\n",
            shm->setup_ref, bsr_id);
    /* bsr_id maybe set to -1 already (on AIX when all tasks attached) */
    if (bsr_id != -1) {
        if (is_last) {
#ifdef _LAPI_LINUX
            ITRC(IT_BSR, "BSR: bsr_free() bsr_id=%d\n", bsr_id);
            (*__bsr_func.bsr_free)(bsr_id);
#else
            ITRC(IT_BSR, "BSR: shmctl RMID bsr_id=%d\n", bsr_id);
            shmctl(bsr_id, IPC_RMID, 0); 
#endif
        }
        bsr_id = -1;
    }
    if (is_last && ckpt_info.in_checkpoint) {
        PAMI::Memory::sync();
        /*
         * Reset bsr_id and bsr_acquired in control block.
         * Note that we don't reset the shm->setup_failed field.
         */ 
        shm->bsr_id       = 0; // every thing in control block is init to zero
        shm->bsr_acquired = false;
        PAMI::Memory::sync();
        ITRC(IT_BSR, "BSR: reset shm->bsr_id and shm->bsr_acquired for checkpoint\n");
    }
    bsr_state = ST_NONE;
}

void Bsr::DetachBsr()
{
    // detach from bsr, if attached before
    if (bsr_addr) {
#ifdef _LAPI_LINUX
        int libbsr_rc = (*(__bsr_func.bsr_unmap))(bsr_addr, bsr_length);
        if (0 != libbsr_rc) {
            ITRC(IT_BSR, "Bsr: bsr_unmap failed with rc=%d, addr=%p len=%u\n",
                    libbsr_rc, bsr_addr, bsr_length);
        }
#else
        PAMI::Memory::sync();
        PAMI::Memory::sync<PAMI::Memory::instruction>();
        shmdt(bsr_addr);
#endif
        bsr_addr = NULL;
        int ref = fetch_and_add ((atomic_p)&shm->setup_ref, -1);
        if (ref == 1) { // I am the last member leave
            is_last = true;
        }
        ITRC(IT_BSR,
                "BSR: DetachBsr() &setup_ref=%p setup_ref=%d->%d is_last=%d in_ckpt=%d\n",
                &shm->setup_ref, ref, ref-1, is_last, ckpt_info.in_checkpoint);
        assert(ref > 0);
    } else {
        ITRC(IT_BSR, "Bsr: DetachBsr() bsr_addr=NULL no-op\n");
    }
}

void Bsr::CleanUp()
{
    ITRC(IT_BSR, "BSR: In CleanUp()\n");

    DetachBsr();

    ReleaseBsrResource();
}

#ifndef _LAPI_LINUX
key_t Bsr::GetBsrUniqueKey(unsigned int j_key)
{
    key_t           bsr_key = -1;
    int             pnsd_handle, rc;
    char            *name = "key.shmv";
    pnsd_key_t      key;
    open_flags_t    flags = NTBL_UPDATES;

    /* if sizeof(pnsd_key_t) larger than the size of key_t, we may truncate the key
     * from PNSD and make it become not unique */
    COMPILE_TIME_ASSERT(sizeof(key_t) >= sizeof(pnsd_key_t));

    rc = papi_open(&pnsd_handle, flags);
    if( rc != 0 ) {
        ITRC(IT_BSR, "BSR::GetBsrUniqueKey() pnsd_api_open failed, rc = %d\n", rc);
        return bsr_key;
    }

    rc = papi_get_keys(pnsd_handle, j_key, name, 1, &key);
    if( rc != 0) {
        ITRC(IT_BSR, "BSR::GetBsrUniqueKey() pnsd_api_get_keys failed, rc = %d\n", rc);
    } else {
        ITRC(IT_BSR, "BSR::GetBsrUniqueKey() pnsd_api_get_keys succeeded with key (0x%x)\n", key);
        bsr_key = (key_t)key;
    }

    rc = papi_close(pnsd_handle);
    if ( rc != 0 ) {
        ITRC(IT_BSR, "BSR::GetBsrUniqueKey() pnsd_api_close failed with rc = %d\n", rc);
    }

    return bsr_key;
}
#endif

bool Bsr::IsBsrReady()
{
    // Check if all tasks have update the reference count, or
    // if any task failed
    if (shm->bsr_acquired && member_cnt == shm->setup_ref) {
        ITRC(IT_BSR, "BSR: %s READY to use (setup_ref=%d)\n",
                (is_leader?"LEADER":"FOLLOWER"), shm->setup_ref);
#ifndef _LAPI_LINUX
        /*
         * All member attached, we don't need bsr_id anymore.
         * Mark the Systemv Shared Memory to be freed.
         */
        /* TODO: Remove this part by adding shm id tracking on AIX.
         *       Adding shm id bookkeeping can avoid shm leak in abnormal case
         *       and enable checkpoint failover. */
        if (is_leader) {
            ITRC(IT_BSR, "BSR: All attached. shmctl RMID bsr_id=%d\n", bsr_id);
            shmctl(bsr_id, IPC_RMID, 0); 
        }
        bsr_id = -1;
#endif /* AIX */
        return true;
    } else 
        return false;
}

bool Bsr::AttachBsr(int mem_id, unsigned char init_val) 
{
    PAMI::Memory::sync();
    bsr_id = shm->bsr_id;
    ASSERT(bsr_id != -1);
    ASSERT(shm->bsr_acquired);
#ifdef _LAPI_LINUX
    bsr_addr = (unsigned char *)(*(__bsr_func.bsr_map))(NULL, (unsigned)bsr_id, 0, 0, &bsr_length);
    if (NULL == bsr_addr || MAP_FAILED == bsr_addr || bsr_length < member_cnt) {
        ITRC(IT_BSR, "BSR: %s bsr_map failed with bsr_id=%u bsr_length=%u errno=%u\n",
                (is_leader)?"LEADER":"FOLLOWER", bsr_id, bsr_length, errno);
        return false;
    }
    ITRC(IT_BSR, "BSR: %s bsr_map w/ bsr_id=%d returns bsr_addr=%p bsr_length=%u\n",
            (is_leader)?"LEADER":"FOLLOWER", bsr_id, bsr_addr, bsr_length);
#else  /* AIX flow */
    // attach to BSR shm region
    bsr_addr = (unsigned char*)shmat(bsr_id, 0, 0);
    if (bsr_addr != (unsigned char*)-1) {
        ITRC(IT_BSR, "BSR: %s BSR attatch with BSR id %d passed at 0x%p\n", 
                (is_leader)?"LEADER":"FOLLOWER", bsr_id, bsr_addr);
    } else {
        ITRC(IT_BSR, "BSR: %s BSR attatch with BSR id %u failed "
                "at 0x%p with errno %d\n", 
                (is_leader)?"LEADER":"FOLLOWER", bsr_id, bsr_addr, errno);
        return false;
    }
#endif /* _LAPI_LINUX */

    // Init and priming BSR shm region properly, ready to do barrier
    Store1(mem_id, init_val);
    unsigned char tmp_val = Load1(mem_id);
    assert(init_val == tmp_val);

    // increase ref count
    int ref = fetch_and_add ((atomic_p)&shm->setup_ref, 1);
    ITRC(IT_BSR, "BSR: attached ref=%d->%d\n", ref, ref+1);
    return true;
}

bool Bsr::GetBsrResource(unsigned int job_key)
{
    ASSERT(is_leader); // this is a leader only step
    size_t bsr_size;
#ifdef _LAPI_LINUX 
    // Check availability of BSR resources.
    if (! BsrFunc::loaded)
        return false;

    unsigned avail, total;
    int      libbsr_rc = 0;

    libbsr_rc = (*(__bsr_func.bsr_query))(&total, &smask, &avail, &fmask);
    if (0 != libbsr_rc || !fmask || total == 0 || avail < member_cnt) {
        ITRC(IT_BSR,
                "BSR: no free BSRs total=%u smask=0x%x, avail=%u, fmask=0x%x, libbsr_rc=%d\n",
                total, smask, avail, fmask, libbsr_rc);
        return false;
    } else {
        ITRC(IT_BSR, "BSR: bsr_query total=%u smask=0x%x, avail=%u, fmask=0x%x, libbsr_rc=%d\n",
                total, smask, avail, fmask, libbsr_rc);
    }

    for (unsigned req_size = 1; req_size <= avail; req_size = req_size << 1) {
        if ((req_size & fmask) == 0)
            continue;
        if (req_size < member_cnt)
            continue;
        else {
            bsr_size = req_size;
            break;
        }
    }

    ITRC(IT_BSR, "BSR: request %d bytes with %u members %lu bytes available BSR memory\n",
            bsr_size, member_cnt, avail);
    /* not enough bsr */
    if (bsr_size == 0)
        return false;

    /* allocate BSR resource */
    unsigned bsr_stride;
    libbsr_rc = (*(__bsr_func.bsr_alloc))
        ((unsigned)bsr_size, &bsr_stride, (unsigned*)&bsr_id);
    if (libbsr_rc) {
        ITRC(IT_BSR, "BSR: bsr_alloc failed with rc = %d\n", libbsr_rc);
        return false;
    }
    ITRC(IT_BSR, "BSR: LEADER bsr_alloc got bsr_id=%u stride=%u\n",
            bsr_id, bsr_stride);
#else 
    // Check availability of BSR resources.
    struct vminfo bsr_info = {0};
    if (vmgetinfo(&bsr_info, VMINFO, sizeof(bsr_info)) != 0) {
        ITRC(IT_BSR, "BSR: vmgetinfo() failed\n");
        perror("vmgetinfo() unexpectedly failed");
        return false;
    }

    // determine BSR region size to allocate
    bsr_size = (member_cnt%PAGESIZE)?
        (member_cnt/PAGESIZE)*PAGESIZE+PAGESIZE : member_cnt;

    // Check availability of BSR memory and number of tasks
    ITRC(IT_BSR, "BSR: request %d bytes with %d members %lu bytes available BSR memory\n",
            bsr_size, member_cnt, bsr_info.bsr_mem_free);
    if (bsr_info.bsr_mem_free < bsr_size) { 
        return false;
    }

    /* allocate BSR resource */
    key_t bsr_key = GetBsrUniqueKey(job_key);
    if (bsr_key != -1) {
        bsr_id = shmget(bsr_key, bsr_size, IPC_CREAT|IPC_EXCL|0600);
        if (bsr_id != -1) {
            ITRC(IT_BSR, "BSR: BSR master shmget passed with (BSR key=0x%x, BSR id=%d)\n",
                    bsr_key, bsr_id);
        } else {
            ITRC(IT_BSR, "BSR: BSR master shmget failed with (BSR key=0x%x, errno=%u)\n",
                    bsr_key, errno);
            return false;
        }
    } else {
        ITRC(IT_BSR, "BSR: BSR master failed to get key from PNSD\n");
        return false;
    }
    // request BSR memory 
    if (shmctl(bsr_id, SHM_BSR, NULL)) {
        ITRC(IT_BSR, "BSR: shmctl(SHM_BSR) failed with errno=%u BSR id %d\n", 
                errno, bsr_id);
        ITRC(IT_BSR, "BSR: shmctl RMID bsr_id=%d\n", bsr_id);
        shmctl(bsr_id, IPC_RMID, 0); 
        bsr_id = -1;
        return false;
    } else {
        ITRC(IT_BSR, "BSR: shmctl(SHM_BSR) passed with BSR id %u.\n", bsr_id);
    }
#endif /* _LAPI_LINUX */

    shm->bsr_id = bsr_id;
    return true;
}

SharedArray::RC Bsr::CheckInitDone(const unsigned int   job_key, 
                                   const int            mem_id, 
                                   const unsigned char  init_val)
{
    bool advance = true;
    while (advance) {
        advance = false;
        /* check if any remote task failed */
        if (shm->setup_failed)
            bsr_state = ST_FAIL;
        switch (bsr_state) {
            case ST_NONE:
                if (is_leader) {
                    if (GetBsrResource(job_key)) {
                        ITRC(IT_BSR,"BSR: Got BSR resource\n");
                        bsr_state = ST_BSR_ACQUIRED;
                        ITRC(IT_BSR,"BSR: bsr_state moved from ST_NONE to ST_BSR_ACQUIRED\n");
                        PAMI::Memory::sync();
                        shm->bsr_acquired = true;
                    } else {
                        ITRC(IT_BSR,"BSR: BSR resource is not available\n");
                        bsr_state = ST_FAIL;
                        PAMI::Memory::sync();
                        shm->setup_failed = true;
                        ITRC(IT_BSR,"BSR: bsr_state moved from ST_NONE to ST_FAIL\n");
                    }
                } else {
                    if (shm->bsr_acquired) {
                        bsr_state = ST_BSR_ACQUIRED;
                        ITRC(IT_BSR,"BSR: bsr_state moved from ST_NONE to ST_BSR_ACQUIRED\n");
                    }
                }
                advance = (bsr_state != ST_NONE);
                break;
            case ST_BSR_ACQUIRED:
                if ( AttachBsr(mem_id, init_val) ) {
                    bsr_state = ST_BSR_WAIT_ATTACH;
                    ITRC(IT_BSR,"BSR: bsr_state moved from ST_BSR_ACQUIRED to ST_BSR_WAIT_ATTACH\n");
                } else {
                    shm->setup_failed = true;
                    bsr_state = ST_FAIL;
                    ITRC(IT_BSR,"BSR: bsr_state moved from ST_NONE to ST_FAIL\n");
                }
                advance = (bsr_state != ST_BSR_ACQUIRED);
                break;
            case ST_BSR_WAIT_ATTACH:
                if ( IsBsrReady() ) {
                    bsr_state = ST_SUCCESS;
                    ITRC(IT_BSR,"BSR: bsr_state moved from ST_BSR_WAIT_ATTACH to ST_SUCCESS\n");
                }
                advance = (bsr_state != ST_BSR_WAIT_ATTACH);
                break;
            case ST_FAIL:
                CleanUp();
                return FAILED;
            case ST_SUCCESS:
                return SUCCESS;
        }
    }

    return PROCESSING;
}

unsigned char      Bsr::Load1(const int byte_offset) const
{
    return bsr_addr[byte_offset];
}

unsigned short     Bsr::Load2(const int byte_offset) const
{
    return *((unsigned short*)(bsr_addr + byte_offset));
}

unsigned int       Bsr::Load4(const int byte_offset) const
{
    return *((unsigned int*)(bsr_addr + byte_offset));
}

unsigned long long Bsr::Load8(const int byte_offset) const
{
    return *((unsigned long long*)(bsr_addr + byte_offset));
}

void Bsr::Store1(const int byte_offset, const unsigned char val)
{
    PAMI::Memory::sync<PAMI::Memory::instruction>();
    bsr_addr[byte_offset] = val;
    PAMI::Memory::sync();
}

void Bsr::Store2(const int byte_offset, const unsigned short val)
{
    PAMI::Memory::sync<PAMI::Memory::instruction>();
    *((unsigned short*)(bsr_addr + byte_offset)) = val;
    PAMI::Memory::sync();
}

void Bsr::Store4(const int byte_offset, const unsigned int val)
{
    assert(0 && "BSR::Store4() not supported");
}

void Bsr::Store8(const int byte_offset, const unsigned long long val)
{
    assert(0 && "BSR::Store8() not supported");
}

bool Bsr::Checkpoint(int byte_offset)
{
    assert(!ckpt_info.in_checkpoint);
    ckpt_info.in_checkpoint = true;
    ckpt_info.prev_state    = bsr_state;
    /*
     * Need to save the bsr byte when BSR is attached.
     */ 
    if (bsr_addr)
        ckpt_info.byte_data = Load1(byte_offset);

    /* release BSR resource */
    DetachBsr();

    /* remove bsr_id */
    ReleaseBsrResource();

    int ret = fetch_and_add((atomic_p)&(shm->ckpt_ref), 1);
    return true;
}

bool Bsr::Resume(int byte_offset)
{
    assert(ckpt_info.in_checkpoint);
    assert(shm->ckpt_ref <= member_cnt);
    /*
     * Resume from a unsuccessful checkpoint, is handled by following code
     * as well. In that case, the bsr_id is not freed.
     * We need to reattach to the BSR with the same shm->bsr_id, to
     * failover from the failed checkpoint.
     */
    PAMI::Memory::sync(); // This sync() is needed for trigger based ckpt emulation
    if (ckpt_info.prev_state == ST_FAIL) 
        bsr_state = ckpt_info.prev_state;
    else
        bsr_state = ST_NONE;

    /* reinitialize BSR (blocking) */
    SharedArray::RC init_rc = PROCESSING;
    while (PROCESSING == init_rc && bsr_state != ckpt_info.prev_state) {
        init_rc = CheckInitDone(
                _Lapi_env.MP_partition, /* Job id may change after restart */
                byte_offset,
                ckpt_info.byte_data);
    }

    int ret = fetch_and_add((atomic_p)&(shm->ckpt_ref), -1);
    assert(ret > 0);
    ckpt_info.in_checkpoint = false;

    return (FAILED != init_rc);
}

bool Bsr::Restart(int byte_offset)
{
    /* same as Resume() now */
    return Resume(byte_offset);
}

