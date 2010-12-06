/**
 * \file api/extension/c/hfi_extension/Extension.h
 * \brief PAMI "HFI" extension interface template specialization
 */
#ifndef __api_extension_c_hfi_extension_Extension_h__
#define __api_extension_c_hfi_extension_Extension_h__
///
/// This extension specific #define is created during configure with the
/// \c --with-pami-extension=hfi_extension option
///
/// \todo Change this to check for the #define specific to the extension. The
///       format is \c __pami_extension_{name}__
///
#ifdef __pami_extension_hfi_extension__ // configure --with-pami-extension=hfi_extension

#include "api/extension/Extension.h"
#include "HfiExtension.h"

namespace PAMI
{
  template <>
  void * Extension::openExtension<3000> (pami_client_t   client,
                                         const char    * name,
                                         pami_result_t & result)
  {
    PAMI::HfiExtension * x;
    pami_result_t rc;
    rc = __global.heap_mm->memalign((void **)&x, 0, sizeof(*x));
    PAMI_assertf(rc == PAMI_SUCCESS, "Failed to alloc PAMI::HfiExtension");
    new (x) PAMI::HfiExtension();

    result = PAMI_SUCCESS;
    return (void *) x;
  }

  ///
  /// \brief Close the extension for use by a client
  ///
  /// The extension cookie was originally provided when the extension was
  /// opened.
  ///
  template <>
  void Extension::closeExtension<3000> (void * cookie, pami_result_t & result)
  {
    PAMI::HfiExtension * x = (PAMI::HfiExtension *) cookie;
    __global.heap_mm->free (x);

    result = PAMI_SUCCESS;
    return;
  }

  ///
  /// \brief Query the extension for a named function
  ///
  /// Returns a function pointer to the static function associated with the
  /// input parameter name.
  ///
  template <>
  void * Extension::queryExtension<3000> (const char * name, void * cookie)
  {
    if (strcasecmp (name, "hfi_pkt_counters") == 0)
      return (void *) PAMI::HfiExtension::hfi_pkt_counters;

    if (strcasecmp (name, "hfi_remote_update") == 0)
      return (void *) PAMI::HfiExtension::hfi_remote_update;

    return NULL;
  };
};

#endif // __pami_extension_hfi_extension__
#endif // __api_extension_c_hfi_extension_Extension_h__
