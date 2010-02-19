/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
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

#include "Global.h"

#ifndef TRACE_COUT
#define TRACE_COUT(x) std::cout << "<" << __global.mapping.task() << ">: " <<  x << std::endl;
#endif

void printHexLine( char * data, size_t num, size_t pad )
{
  size_t i;
  std::cout << "<" << __global.mapping.task() << ">: " << (void *)data << ": ";
  for ( i=0; i<num; i++ )
  {
    printf("%02x ", (uint8_t)data[i]);
  }
  for ( i=0; i<pad; i++ )
  {
    printf("   ");
  }
  printf(" ");
  for ( i=0; i<num; i++ )
  {
    if ( isgraph((unsigned)data[i]))
    {
      printf ("%c", data[i] );
    } else {
      printf(".");
    }
  }
  printf("\n");
}

void dumpHexData( void * bPtr, size_t nb )
{
    char * data = (char *)bPtr;
    size_t i;
    std::cout << "<" << __global.mapping.task() << ">: Hex dump of " << bPtr << " for " << nb << " bytes" << std::endl;
    for ( i=0; i<nb; i+=16 )
    {
      if ( i+16 <= nb )
      {
        printHexLine( data+i, 16, 0 );
      } else {
        printHexLine( data+i, nb-i, 16-(nb-i) );
      }
    }
}

#ifndef DUMP_HEX_DATA
#define DUMP_HEX_DATA(x,y) // dumpHexData(x,y)
#endif

#endif // __components_devices_udp_trace_h__
