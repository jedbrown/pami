/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/MemoryInterface.h
 * \brief Defines arch memory interface
 */

#ifndef __arch_MemoryInterface_h__
#define __arch_MemoryInterface_h__

#include "Arch.h"

namespace PAMI
{
  namespace Memory
  {
    typedef enum
    {
      full_sync = 0,
      remote_msync,
      l1p_flush
    } attribute_t;

    template <unsigned T_Attribute>
    static const bool supports ()
    {
      return false;
    };

    template <unsigned T_Attribute>
    static void sync ()
    {
      //fprintf (stdout, "sync<>()\n");
      mem_barrier();
    };

    static void sync ()
    {
      //fprintf (stdout, "sync()\n");
      sync<0> ();
    };

  };
};

#endif // __arch_MemoryInterface_h__


//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
