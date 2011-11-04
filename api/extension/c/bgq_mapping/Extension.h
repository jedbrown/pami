/**
 * \file api/extension/c/bgq_mapping/Extension.h
 * \brief PAMI extension interface template specialization for bgq mapping
 */
#ifndef __api_extension_c_bgq_mapping_Extension_h__
#define __api_extension_c_bgq_mapping_Extension_h__
///
/// This extension specific #define is created during configure with the
/// \c --with-pami-extension=bgq_mapping option
///
#ifdef __pami_extension_bgq_mapping__ // configure --with-pami-extension=bgq_mapping

#include "api/extension/Extension.h"
#include "Global.h"

namespace PAMI
{
  ///
  /// \brief Open the extension for use by a client
  ///
  /// An extension cookie is returned after the extension is opened which is
  /// provided as an input parameter when the extension is closed.
  ///
  /// Each extension implementation may define the extension cookie differently.
  /// Often the cookie is a pointer to allocated memory which may contain an
  /// instance of a class that provides the extension capabilities. Any defined
  /// extension classes are private to the extension and are not visible to
  /// objects external to the extension implementation.
  ///
  template <>
  void * Extension::openExtension<9100> (pami_client_t   client,
                                         const char    * name,
                                         pami_result_t & result)
  {
    result = PAMI_SUCCESS;
    return (void *) NULL;
  }

  ///
  /// \brief Close the extension for use by a client
  ///
  /// The extension cookie was originally provided when the extension was
  /// opened.
  ///
  template <>
  void Extension::closeExtension<9100> (void * cookie, pami_result_t & result)
  {
    result = PAMI_SUCCESS;
  }

  ///
  /// \brief Query the extension for a named symbol
  ///
  /// Returns a pointer to the symbol, such as a static function or global
  /// object, associated with the input parameter name.
  ///
  template <>
  void * Extension::queryExtension<9100> (const char * name, void * cookie)
  {
    if (strcasecmp(name, "mapcache") == 0)
      return (void *) __global.getMapCache()->torus.task2coords;

    return NULL;
  };
};

#endif // __pami_extension_bgq_mapping__
#endif // __api_extension_c_bgq_mapping_Extension_h__
