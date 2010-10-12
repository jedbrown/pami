/**
 * \file components/devices/ShmemCollInterface.h
 * \brief ???
 */
#ifndef __components_devices_ShmemCollInterface_h__
#define __components_devices_ShmemCollInterface_h__

#include <sys/uio.h>

#include "sys/pami.h"
#include "util/common.h"

namespace PAMI
{
  namespace Device
  {
    namespace Interface
	{

	 typedef int (*MatchFunction_t) (void* coll_desc, 			//my collective descriptor
									void* match_desc , 			//matched descriptor of master
									void  *match_func_param );  //device cookie

	}
  }
}

#endif
