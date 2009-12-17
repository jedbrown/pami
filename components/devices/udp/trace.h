/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/udp/trace.h
 * \brief ???
 */

#ifndef __components_devices_udp_trace_h__
#define __components_devices_udp_trace_h__

#include <iomanip>
#include <iostream>

#ifndef TRACE_COUT
#define TRACE_COUT(x) std::cout << "<" << __global.mapping.task() << ">: " <<  x << std::endl; 
#endif

void dumpHexData( void * bPtr, size_t nb )
{
    size_t i;
    std::cout << "<" << __global.mapping.task() << ">: Hex dump of " << bPtr << " for " << nb << " bytes" << std::endl;
    std::cout << "<" << __global.mapping.task() << ">: " << std::hex << bPtr << ": "; 
    for ( i=0; i<nb; i+=sizeof(uint32_t) )
    {
       std::cout << std::hex << std::setfill('0') << std::setw(8) << *((uint32_t*)bPtr) << " "; 
       bPtr = (void*)((uintptr_t)bPtr + sizeof(uint32_t)); 
      if ( i%(4*sizeof(uint32_t)) == 3*sizeof(uint32_t) ) 
      {
         std::cout << std::endl; 
         std::cout << "<" << __global.mapping.task() << ">: " << std::hex << bPtr << ": "; 
      }
     }
    if ( i%16 != 0 )
    {
      std::cout << std::endl; 
    }
    std::cout << std::dec; 
}

#ifndef DUMP_HEX_DATA
#define DUMP_HEX_DATA(x,y) // dumpHexData(x,y)
#endif 

#endif // __components_devices_udp_trace_h__
