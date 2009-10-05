namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief Standard pthread mutex class
    ///
    template <class T>
    class PthreadMutex : public Interface::Mutex<PthreadMutex>,
                         public Interface::ProcessScope<PthreadMutex>
    {
      public:

        inline PthreadMutex  () :
          Interface::Mutex<PthreadMutex> (),
          Interface::ProcessScope<PthreadMutex> (),
          _scoped (false)
        {};

        inline ~PthreadMutex () {};

        inline void init_impl ()
        {
          // foo...
        };

        inline void acquire_impl ()
        {
        };

        inline void release_impl ()
        {
        }

        inline void setProcessScope_impl ()
        {
          _scoped = true;
        };

        /// \see XMI::Atomic::Interface::ProcessScope
        bool _scoped;
    };
  };
};
