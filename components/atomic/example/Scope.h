namespace XMI
{
  namespace Atomic
  {
    namespace Interface
    {
      ///
      /// \brief Node scope interface class
      ///
      template <class T>
      class NodeScope
      {
        public:
          inline NodeScope  () {};
          inline ~NodeScope () {};

          /// \note All classes that implement the NodeScope interface must
          ///       contain the \c _scoped boolean data memeber that is
          ///       accessed in this method interface.
          inline void setNodeScope ();
      };

      ///
      /// \brief Process scope interface class
      ///
      template <class T>
      class ProcessScope
      {
        public:
          inline ProcessScope  () {};
          inline ~ProcessScope () {};

          /// \note All classes that implement the NodeScope interface must
          ///       contain the \c _scoped boolean data memeber that is
          ///       accessed in this method interface.
          inline void setProcessScope ();
      };
    };
  };
};

template <class T>
inline bool XMI::Atomic::Interface::NodeScope<T>::setNodeScope()
{
  static_cast<T*>(this)->setNodeScope_impl();
  return T::_scoped;
}

template <class T>
inline bool XMI::Atomic::Interface::ProcessScope<T>::setProcessScope()
{
  static_cast<T*>(this)->setProcessScope_impl();
  return T::_scoped;
}
