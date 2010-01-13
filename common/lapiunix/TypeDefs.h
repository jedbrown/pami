/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/lapiunix/TypeDefs.h
 * \brief ???
 */

#ifndef __common_lapiunix_typedefs_h__
#define __common_lapiunix_typedefs_h__


#include "components/devices/lapiunix/oldlapiunixmulticastmodel.h"
#include "components/devices/lapiunix/oldlapiunixm2mmodel.h"
#include "components/devices/lapiunix/lapiunixmessage.h"
#include "components/devices/lapiunix/lapiunixdevice.h"
#include "common/lapiunix/NativeInterface.h"


typedef XMI::Device::LAPIOldmulticastModel<XMI::Device::LAPIDevice<XMI::SysDep>,
                                          XMI::Device::LAPIMessage> LAPIMcastModel;
typedef TSPColl::NBCollManager<LAPIMcastModel> XMI_NBCollManager;

typedef XMI::Device::LAPIOldm2mModel<XMI::Device::LAPIDevice<XMI::SysDep>,
                                    XMI::Device::LAPIMessage,
                                    size_t> LAPIM2MModel;

typedef XMI::Device::LAPIDevice<XMI::SysDep>               LAPIDevice;


#define XMI_COLL_MCAST_CLASS  LAPIMcastModel
#define XMI_COLL_M2M_CLASS    LAPIM2MModel
#define XMI_COLL_SYSDEP_CLASS XMI::SysDep
#define XMI_NATIVEINTERFACE   LAPINativeInterface

#endif
