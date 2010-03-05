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
#include "common/lapiunix/NativeInterface.h"
#include "algorithms/geometry/Geometry.h"


typedef XMI::Device::LAPIOldmulticastModel<XMI::Device::LAPIDevice,
                                           XMI::Device::OldLAPIMcastMessage> LAPIOldMcastModel;
typedef XMI::Device::LAPIOldm2mModel<XMI::Device::LAPIDevice,
                                     XMI::Device::OldLAPIM2MMessage,
                                     size_t>                                 LAPIOldM2MModel;

#define XMI_COLL_MCAST_CLASS  LAPIOldMcastModel
#define XMI_COLL_M2M_CLASS    LAPIOldM2MModel
#define XMI_COLL_SYSDEP_CLASS XMI::SysDep
#define XMI_NATIVEINTERFACE   LAPINativeInterface
#define XMI_GEOMETRY_CLASS    XMI::Geometry::Common

#endif
