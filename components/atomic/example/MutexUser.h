/**
 * \file components/atomic/example/MutexUser.h
 * \brief ???
 */

#ifndef __components_atomic_example_MutexUser_h__
#define __components_atomic_example_MutexUser_h__

namespace XMI
{
  template <T_Mutex>
  class MutexUser
  {
    public:

      inline MutexUser () :
        _mutex ()
      {
        _mutex.setNodeScope ();
        _mutex.init ();
      };

      inline void doSomething ()
      {
        _mutex.acquire ();

        // blah

        _mutex.release ();
      };

    private:

     T_Mutex _mutex;
  };
};

#endif
