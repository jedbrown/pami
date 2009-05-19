#ifndef   __pgasrt_multisend_impl_h__
#define   __pgasrt_multisend_impl_h__

namespace CCMI
{
    namespace Adaptor
    {
	namespace Generic
	{
	    class regTable
	    {
	    public:
		void  add(int in, void *reg);
		void *get(int in);
	    };
	}
    }
}
#endif
