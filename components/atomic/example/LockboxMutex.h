namespace XMI
{
  namespace Atomic
  {
    ///
    /// \brief Blue Gene/P lockbox mutex class
    ///
    class LockboxMutex : public Interface::Mutex<LockboxMutex>,
                         public Interface::NodeScope<LockboxMutex>,
                         public Interface::ProcessScope<LockboxMutex>
    {
      private:

        typedef enum
        {
          LOCKBOX_SCOPE_NODE = 0;
          LOCKBOX_SCOPE_PROCESS
        } lockbox_scope_t;

        lockbox_scope_t _scope;

      public:

        /// \see XMI::Atomic::Interface::NodeScope
        /// \see XMI::Atomic::Interface::ProcessScope
        bool _scoped;

        inline LockboxMutex  () :
          Interface::Mutex<LockboxMutex> (),
          Interface::NodeScope<LockboxMutex> (),
          Interface::ProcessScope<LockboxMutex> (),
          _scoped (false)
        {};

        inline ~LockboxMutex () {};

        inline void init_impl (XMI::SysDep *sd)
        {
	  sd->lockboxFactory.lbx_alloc((void **)&_mutex, 1, );
        };

        inline void acquire_impl ()
        {
        };

        inline void release_impl ()
        {
        }

        inline void setNodeScope_impl ()
        {
          _scope  = LOCKBOX_SCOPE_NODE;
          _scoped = true;
        };

        inline void setProcessScope_impl ()
        {
          _scope  = LOCKBOX_SCOPE_PROCESS;
          _scoped = true;
        };
    };
  };
};
