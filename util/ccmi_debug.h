/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file util/ccmi_debug.h
 * \brief common debug macros
 */

#ifndef __util_ccmi_debug_h__
#define __util_ccmi_debug_h__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

//#define TRACE_MSG(x) fprintf x
#ifndef TRACE_MSG
  #define TRACE_MSG(x)
#endif

//#define CCMI_DEBUG 0

#ifdef CCMI_DEBUG

  #warning CCMI DEBUG enabled

  #undef TRACE_MSG
  #define TRACE_MSG(x) fprintf x

  #undef TRACE_BUF
//  #define TRACE_BUF(x) fprintf x
  #ifndef TRACE_BUF
    #define TRACE_BUF(x)
  #endif

  #define TRACE_DATA(x) CCMI_ADAPTOR_DEBUG_trace_data x
//#define TRACE_DATA(x) if((XMI_SINGLE_COMPLEX == _dt) && (XMI_SUM == _op)) CCMI_ADAPTOR_DEBUG_trace_data x
inline void CCMI_ADAPTOR_DEBUG_trace_data(const char* string, const char* buffer,unsigned size)
{
  unsigned nChunks = size / 32;
  TRACE_MSG((stderr, "<%p> %s, length=%#X\n",(int) buffer, string, size));
  if(!buffer) return;
  for(unsigned i = 0; i < nChunks; i++)
  {
    TRACE_BUF((stderr,
               "<%p>: %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X \n",
               (int)buffer+(i*32),
               *(int*)(buffer+(i*32)+0),
               *(int*)(buffer+(i*32)+4),
               *(int*)(buffer+(i*32)+8),
               *(int*)(buffer+(i*32)+12),
               *(int*)(buffer+(i*32)+16),
               *(int*)(buffer+(i*32)+20),
               *(int*)(buffer+(i*32)+24),
               *(int*)(buffer+(i*32)+28)
              ));
  }
  if(size % 32)
  {
    unsigned lastChunk = nChunks * 32;
    TRACE_BUF((stderr,
               "<%p>: %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X %8.8X \n",
               (int)buffer+lastChunk,
               lastChunk+0<size?*(int*)(buffer+lastChunk+0):0xDEADDEAD,
               lastChunk+4<size?*(int*)(buffer+lastChunk+4):0xDEADDEAD,
               lastChunk+8<size?*(int*)(buffer+lastChunk+8):0xDEADDEAD,
               lastChunk+12<size?*(int*)(buffer+lastChunk+12):0xDEADDEAD,
               lastChunk+16<size?*(int*)(buffer+lastChunk+16):0xDEADDEAD,
               lastChunk+20<size?*(int*)(buffer+lastChunk+20):0xDEADDEAD,
               lastChunk+24<size?*(int*)(buffer+lastChunk+24):0xDEADDEAD,
               lastChunk+28<size?*(int*)(buffer+lastChunk+28):0xDEADDEAD
              ));
    lastChunk = 0; // gets rid of an annoying warning when not tracing the buffer
  }
}

#else // CCMI_DEBUG not defined
  #define TRACE_DATA(x)
  #ifndef TRACE_BUF
    #define TRACE_BUF(x)
  #endif
#endif

//#define TRACE_FLOW(x) fprintf x
#ifndef TRACE_FLOW
  #define TRACE_FLOW(x)
#endif

//#define TRACE_INIT(x) fprintf x
#ifndef TRACE_INIT
  #define TRACE_INIT(x)
#endif

//#define TRACE_ADVANCE(x) fprintf x
#ifndef TRACE_ADVANCE
  #define TRACE_ADVANCE(x)
#endif

//#define TRACE_REDUCEOP(x) fprintf x
#ifndef TRACE_REDUCEOP
  #define TRACE_REDUCEOP(x)
#endif

//#define CCMI_DEBUG_SCHEDULE
//#define TRACE_STATE(x) fprintf x
#ifndef TRACE_STATE
  #define TRACE_STATE(x)
#endif

//#define TRACE_SCHEDULE(x) fprintf x
#ifndef TRACE_SCHEDULE
  #define TRACE_SCHEDULE(x)
#endif

// TRACE_ALERT helps (minimally) trace object ctor/resets that help determine whether
// they're being reused appropriately.
//#define TRACE_ALERT(x) fprintf x
#ifndef TRACE_ALERT
  #define TRACE_ALERT(x)
#endif

//#define TRACE_ADAPTOR(x) fprintf x
#ifndef TRACE_ADAPTOR
  #define TRACE_ADAPTOR(x)
#endif



static inline void CCMI_FATALERROR (int errcode, const char * strg, ...)
{
    char buffer[120];
    va_list ap;
    va_start(ap, strg);
    vsnprintf (buffer, 119, strg, ap);
    va_end(ap);
    if (errcode==0) errcode=-1;
    fprintf(stderr, "Collective Runtime Error %d: %s\n",errcode,buffer);

    // char * z = 0;
    // *z = 0xFF;
    exit(1);
}



#endif //__adaptor_ccmi_debug_h__
