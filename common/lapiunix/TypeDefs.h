/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/lapiunix/TypeDefs.h
 * \brief ???
 */

#ifndef __common_lapiunix_TypeDefs_h__
#define __common_lapiunix_TypeDefs_h__


#include "components/devices/lapiunix/oldlapiunixmulticastmodel.h"
#include "components/devices/lapiunix/oldlapiunixm2mmodel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "components/devices/lapiunix/lapiunixdevice.h"
#include "algorithms/geometry/LapiGeometry.h"


typedef PAMI::Device::LAPIOldmulticastModel<PAMI::Device::LAPIDevice,
                                           PAMI::Device::OldLAPIMcastMessage> LAPIOldMcastModel;
typedef PAMI::Device::LAPIOldm2mModel<PAMI::Device::LAPIDevice,
                                     PAMI::Device::OldLAPIM2MMessage,
                                     size_t>                                 LAPIOldM2MModel;

#define PAMI_COLL_MCAST_CLASS  LAPIOldMcastModel
#define PAMI_COLL_M2M_CLASS    LAPIOldM2MModel
#define PAMI_COLL_SYSDEP_CLASS PAMI::SysDep
#define PAMI_GEOMETRY_CLASS    PAMI::Geometry::Lapi

#endif
