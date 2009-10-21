/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/geometry/mpi/bgpcollinfo.h
 * \brief ???
 */

#ifndef   __xmi_bgpcollinfo__h__
#define   __xmi_bgpcollinfo__h__

#include <vector>
#include "components/geometry/CollRegistration.h"
#include "SysDep.h"
#include "Mapping.h"


namespace XMI
{
  namespace CollInfo
  {
    typedef enum
    {
      CI_BROADCAST0=0,
      CI_BROADCAST1,
      CI_BROADCAST2,
      CI_ALLGATHER0,
      CI_ALLGATHERV0,
      CI_SCATTER0,
      CI_SCATTERV0,
      CI_ALLREDUCE0,
      CI_ALLREDUCE1,
      CI_ALLREDUCE2,
      CI_BARRIER0,
      CI_BARRIER1,
      CI_AMBROADCAST0,
      CI_ALLTOALLV0,
    }collinfo_type_t;


    template <class T_Device>
    class CollInfo
    {
    public:
      CollInfo(T_Device *dev) {}
      collinfo_type_t _colltype;
      xmi_metadata_t _meta;
    };
#if 0
    template <class T_Device>
    class BGPRectBroadcastInfo:public CollInfo<T_Device>
    {
    public:
      BGPRectBroadcastInfo(T_Device *dev): CollInfo<T_Device>(dev)
      {}
    };
#endif

  };
};
typedef XMI::Device::BGPDevice<XMI::SysDep> BGPDevice;
typedef std::vector<XMI::CollInfo::CollInfo<BGPDevice> *> RegQueue;

#endif
