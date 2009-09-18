




MutexUser<LockboxMutex> _foo; // This will compile.

MutexUser<PthreadMutex> _bar; // This will not compile.

