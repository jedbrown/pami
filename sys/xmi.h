/**
 * \file sys/xmi.h
 * \brief messaging interface
 */
#ifndef __xmi_h_
#define __xmi_h_

/*  Platform specific defines and configuration options */
#include "xmi_config.h"

/*  XMI datatypes                                       */
#include "xmi_types.h"

/*  XMI dispatch routines                               */
#include "xmi_dispatch.h"

/*  XMI point to point routines                         */
#include "xmi_p2p.h"

/*  XMI global and per destination fence routines       */
#include "xmi_fence.h"

/*  XMI high level collective routines                  */
#include "xmi_collectives.h"

/*  XMI data pipelining routines                        */
#include "xmi_pipeworkqueue.h"

/*  XMI topology routines                               */
#include "xmi_topology.h"

/*  XMI low level collective routines                   */
#include "xmi_multisend.h"

/*  XMI datatype management routines                    */
#include "xmi_datatypes.h"

/*  XMI platform specific extensions                    */
#include "xmi_ext.h"

#endif
