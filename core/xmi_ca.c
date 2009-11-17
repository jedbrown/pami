/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file core/xmi_ca.c
 * \brief ???
 */

#include "sys/xmi.h"
#include "util/compact_attributes.h"
#include <stdarg.h>

void xmi_metadata_multiset(xmi_ca_t *ca_list, ...)
{
  va_list arg_ptr;
  int value = 0;

  va_start(arg_ptr, ca_list);

  value = va_arg(arg_ptr, int);

  do
  {
    xmi_ca_set(ca_list, value);
    value = va_arg(arg_ptr, int);
  } while (value > CA_END_ARGS);

  va_end(arg_ptr);
}
