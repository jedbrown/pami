
#ifndef  __ccmi_adaptor_protocol_factory__
#define  __ccmi_adaptor_protocol_factory__

#include "collectives/interface/Geometry.h"

namespace CCMI
{
  namespace Adaptor
  {
    class ProtocolFactory
    {
    public:
      ProtocolFactory ()
      {
      }
      
      virtual ~ProtocolFactory ()
      {
      }
      
      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }
      
      /// \brief All protocols determine if a given geometry is adequate
      virtual bool Analyze(CCMI::Adaptor::Geometry *grequest) = 0;

    };
  };
};

#endif
