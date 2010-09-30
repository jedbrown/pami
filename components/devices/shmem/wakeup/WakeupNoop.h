///
/// \file components/devices/shmem/wakeup/WakeupNoop.h
/// \brief Empty "noop" implementation of the wakeup interface
///
#ifndef __components_devices_shmem_wakeup_WakeupNoop_h__
#define __components_devices_shmem_wakeup_WakeupNoop_h__

#include "Global.h"

#include "components/devices/shmem/wakeup/WakeupInterface.h"

namespace PAMI
{
  namespace Wakeup
  {
    class Noop : public Wakeup::Interface<Wakeup::Noop>
    {
      public:

        friend class Wakeup::Interface<Wakeup::Noop>;

        class Region : public Wakeup::Interface<Wakeup::Noop>::Region
        {
          public:

            friend class Wakeup::Interface<Wakeup::Noop>::Region;

            inline size_t & operator[] (size_t position)
            {
              return _array[position];
            };

          protected:

            inline void init_impl (Region & original)
            {
              _array = original._array;
            };

            template <class T_MemoryManager>
            inline void init_impl (T_MemoryManager * mm,
                                   char            * key,
                                   size_t            count,
                                   Wakeup::Noop    * wakeup)
            {
              pami_result_t mmrc = PAMI_ERROR;

              mmrc = mm->memalign ((void **) & _array,
                                   16,                      // align
                                   count * sizeof(size_t),  // bytes
                                   key,                     // unique
                                   Memory::MemoryManager::memzero,
                                   NULL);                   // cookie

              PAMI_assertf(mmrc == PAMI_SUCCESS, "memalign failed for noop wakeup resources, rc=%d\n", mmrc);
            };


            size_t * _array;

        };

      protected:

        ///
        /// \see PAMI::Wakeup::Interface::generate()
        ///
        static Noop * generate_impl (size_t peers, const char * key)
        {
          Noop * wakeup = NULL;

          pami_result_t mmrc = PAMI_ERROR;
          mmrc = __global.heap_mm->memalign((void **) & wakeup,
                                            16,                      // align
                                            sizeof(*wakeup) * peers, // bytes
                                            key,                     // unique
                                            NULL,                    // static fn
                                            NULL);                   // cookie
          PAMI_assertf(mmrc == PAMI_SUCCESS, "heap memalign failed for noop wakeup object array, rc=%d\n", mmrc);

          return wakeup;
        };

        ///
        /// \see PAMI::Wakeup::Interface::poke()
        ///
        inline pami_result_t poke_impl ()
        {
          return PAMI_SUCCESS;
        };
        
      private:

        ///
        /// \brief Initialize an array of noop wakeup objects
        ///
        /// \see PAMI::Memory::MM_INIT_FN
        ///
        static void array_initialize (void       * memory,
                                      size_t       bytes,
                                      const char * key,
                                      unsigned     attributes,
                                      void       * cookie)
        {
          Wakeup::Noop * wakeup = (Wakeup::Noop *) memory;
          size_t i, peers = (size_t) cookie;

          for (i = 0; i < peers; i++)
            {
              new (&wakeup[i]) Wakeup::Noop ();
            }
        };


    };
  };
};
#endif // __components_devices_shmem_wakeup_WakeupNoop_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

