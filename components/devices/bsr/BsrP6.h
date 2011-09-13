#ifndef __components_devices_bsr_BsrP6_h__
#define __components_devices_bsr_BsrP6_h__

/**
 * \file components/devices/bsr/BsrP6.h
 *
 * \brief BsrP6 class definition.
 *
 * \warning Only supports 64-bit applications.
 */

#include "SharedArray.h"

/**
 * \brief BsrP6 class
 *
 * A derived class for Barrier Synchronization Register (BSR) on Power6
 * machines.
 *
 * \note The progress callback function is only used in Init(), ShmSetup(),
 *       and ShmDestroy() functions.
 *
 */
class BsrP6 : public SharedArray
{
    public:
        BsrP6();
        ~BsrP6();
        RC CheckInitDone(const unsigned int   mem_cnt, 
                         const unsigned int   job_key, 
                         const uint64_t       unique_key,
                         const bool           leader, 
                         const int            mem_id, 
                         const unsigned char  init_val);

        unsigned char      Load1(const int byte_offset) const;
        unsigned short     Load2(const int byte_offset) const;
        unsigned int       Load4(const int byte_offset) const;
        unsigned long long Load8(const int byte_offset) const;
        void Store1(const int byte_offset, const unsigned char val);
        void Store2(const int byte_offset, const unsigned short val);
        void Store4(const int byte_offset, const unsigned int val);
        void Store8(const int byte_offset, const unsigned long long val);

    private:
        typedef int   (*_bsr_open_t)(void);
        typedef void  (*_bsr_close_t)(int);
        typedef int   (*_bsr_allocate_t)(int, int);
        typedef void  (*_bsr_unallocate_t)(int, int);
        typedef void* (*_bsr_attach_t)(int, int);
        typedef void  (*_bsr_detach_t)(int, void *);
        typedef int   (*_bsr_query_granule_t)(int);
        struct _bsr_func_t{
            _bsr_open_t           bsr_open;
            _bsr_close_t          bsr_close;
            _bsr_allocate_t       bsr_allocate;
            _bsr_unallocate_t     bsr_unallocate;
            _bsr_attach_t         bsr_attach;
            _bsr_detach_t         bsr_detach;
            _bsr_query_granule_t  bsr_query_granule;
        };
        static _bsr_func_t        bsr_func;
        static bool               bsr_func_loaded;
        int                       bsr_fd;      // BSR driver file descriptor
        int                       bsr_granule;
        int                       bsr_id_cnt;  // number of BSR IDs needed
        int                       bsr_att_cnt; // number of bsr_attach has been
                                               // done
        volatile unsigned char*  *bsr_addr;    // bsr address
        static RC                 LoadDriverFunc();

        enum BSR_SETUP_STATE {
            ST_NONE    = 0x00000000,
            ST_ATTACHED= 0x00000001,
            ST_FAIL    = 0x00009999
        };
        struct Shm {
            // current state of BSR setup process
            volatile BSR_SETUP_STATE bsr_setup_state;
            // ref count; number of successful bsr setup
            volatile unsigned int bsr_setup_ref;
            // array for BSR IDs
            volatile int          bsr_ids[0];
        };
        Shm*                      shm; // shm block used to do internal
                                       // communication.
        // helper function
        void CleanUp();
};
#endif
