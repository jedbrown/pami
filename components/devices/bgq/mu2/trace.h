/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file components/devices/bgq/mu2/trace.h
/// \brief Yet another attempt at a trace/debug facility
///
#ifndef __components_devices_bgq_mu2_trace_h__
#define __components_devices_bgq_mu2_trace_h__


static size_t __trace_fn_counter = -1;
static char   __trace_indent_string[64] = { "                                                               " }; // 63 ' ' and 1 NULL

#define TRACE_FN_ENTER() \
  if (DO_TRACE_ENTEREXIT) \
    fprintf (stderr, "%s>> %s() [%s:%d]\n", &__trace_indent_string[63-(++__trace_fn_counter<<1)], __FUNCTION__, __FILE__, __LINE__);

#define TRACE_FN_EXIT() \
  if (DO_TRACE_ENTEREXIT) \
    fprintf (stderr, "%s<< %s() [%s:%d]\n", &__trace_indent_string[63-(__trace_fn_counter--<<1)], __FUNCTION__, __FILE__, __LINE__);

#define TRACE_STRING(str) \
  if (DO_TRACE_DEBUG) \
    fprintf (stderr, "%s   %s():%d .. " str "\n", &__trace_indent_string[63-(__trace_fn_counter<<1)], __FUNCTION__, __LINE__);

#define TRACE_FORMAT(fmt, ...) \
  if (DO_TRACE_DEBUG) \
    fprintf (stderr, "%s   %s():%d .. " fmt "\n", &__trace_indent_string[63-(__trace_fn_counter<<1)], __FUNCTION__, __LINE__, __VA_ARGS__);

#define TRACE_HEXDATA(ptr,n) \
  if (DO_TRACE_DEBUG) { \
    size_t byte; char bigstring[1000];\
    size_t index = sprintf(&bigstring[0], "%s   %s():%d [%p]",&__trace_indent_string[63-(__trace_fn_counter<<1)], __FUNCTION__, __LINE__, ptr); \
    size_t indent = index; \
    for(byte=0; byte<n; byte++) { \
      if(byte%32==0) { fprintf(stderr,"%s\n",bigstring); index = sprintf(&bigstring[0], "%s", &__trace_indent_string[63-indent]); } \
      if(byte%8==0) index += sprintf(&bigstring[index], " "); \
      if(byte%4==0) index += sprintf(&bigstring[index], " "); \
      index += sprintf(&bigstring[index], "%02x", ((char *)ptr)[byte]); \
    } \
    fprintf(stderr, "%s\n", &bigstring[0]); \
  }





#endif // __components_devices_bgq_mu2_trace_h__
