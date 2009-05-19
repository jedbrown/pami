#include "./regTable.h"
#include <map>

namespace CCMI
{
    namespace Adaptor
    {
        namespace Generic
        {
	    std::map<int,void*> _g_regtable_data;
	    CCMI::Adaptor::Generic::regTable _g_regtable;


	    void CCMI::Adaptor::Generic::regTable::add(int in, void*reg)
	    {
		_g_regtable_data[in] = reg;

	    }

	    void * CCMI::Adaptor::Generic::regTable::get(int in)
	    {
		return _g_regtable_data[in];
	    }
	}
    }
}
