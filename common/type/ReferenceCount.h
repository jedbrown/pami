/**
 * \file common/type/ReferenceCount.h
 * \brief ???
 */
#ifndef __common_type_ReferenceCount_h__
#define __common_type_ReferenceCount_h__

#include <assert.h>
#ifdef _AIX
#include "compiler/xl/Compiler.h"
#endif

namespace PAMI
{

    class ReferenceCount
    {
        public:
            ReferenceCount();
            virtual ~ReferenceCount();
            void    AcquireReference();
            void    ReleaseReference();
            ssize_t GetReferenceCount() const;
        private:
            ssize_t ref_cnt;
    };

    inline ReferenceCount::ReferenceCount()
        : ref_cnt(0)
    {
    }

    inline ReferenceCount::~ReferenceCount()
    {
        assert(ref_cnt == 0);
    }

    inline void ReferenceCount::AcquireReference()
    {
        __sync_fetch_and_add(&ref_cnt, 1);
    }

    inline void ReferenceCount::ReleaseReference()
    {
        assert(ref_cnt > 0);
        ssize_t old_cnt = __sync_fetch_and_add(&ref_cnt, -1);
        if (old_cnt == 1)
            delete this;

        // NOTE: The reference-counted object must always be from the heap.
        //       Otherwise delete will cause segfault.
    }

    inline ssize_t ReferenceCount::GetReferenceCount() const
    {
        return ref_cnt;
    }

}

#endif // _PAMI_REFERENCE_COUNT_H
