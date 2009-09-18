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