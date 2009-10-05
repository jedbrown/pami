namespace XMI
{
  namespace Atomic
  {
    namespace Interface
    {
      ///
      /// \brief Mutex interface class
      ///
      template <class T>
      class Mutex
      {
        public:

          ///
          /// \brief  Allocate and initialize the mutex
          ///
          inline void init();

          ///
          /// \brief  Acquire a lock atomically
          ///
          inline void acquire();

          ///
          /// \brief  Release a lock atomically
          ///
          inline void release();
      };
    };
  };
};

template <class T>
inline void XMI::Atomic::Interface::Mutex<T>::init()
{
  static_cast<T*>(this)->init_impl();
}

template <class T>
inline void XMI::Atomic::Interface::Mutex<T>::acquire()
{
  static_cast<T*>(this)->acquire_impl();
}

template <class T>
inline void XMI::Atomic::Interface::Mutex<T>::release()
{
  static_cast<T*>(this)->release_impl();
}
