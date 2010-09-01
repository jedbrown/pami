/**
 * \file api/extension/c/template_extension/Extension.h
 * \brief PAMI "template" extension interface template specialization
 *
 * \todo Update doxygen and preprocessor directives
 */
#ifndef __api_extension_c_template_extension_Extension_h__
#define __api_extension_c_template_extension_Extension_h__
///
/// This extension specific #define is created during configure with the
/// \c --with-pami-extension=template_extension option
///
/// \todo Change this to check for the #define specific to the extension. The
///       format is \c __pami_extension_{name}__
///
#ifdef __pami_extension_template_extension__ // configure --with-pami-extension=template_extension

#include "api/extension/Extension.h"

namespace PAMI
{
  ///
  /// \brief Private class used by the template extension
  ///
  /// \note It is not neccesary for an extension to define or use a private
  ///       extension class
  ///
  class ExtensionTemplate
  {
    public:

      /// \brief An arbitrary function that does nothing
      static void foo () {};

      /// \brief Another arbitrary function that does nothing
      static void bar () {};
  };

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
  /// \todo Replace the template parameter with the unique extension identifier
  ///       assigned to the specific extension
  ///
  template <>
  void * Extension::openExtension<####> (pami_client_t   client,
                                         const char    * name,
                                         pami_result_t & result)
  {
    result = PAMI_UNIMPL;
    return NULL;
  }

  ///
  /// \brief Close the extension for use by a client
  ///
  /// The extension cookie was originally provided when the extension was
  /// opened.
  ///
  /// \todo Replace the template parameter with the unique extension identifier
  ///       assigned to the specific extension
  ///
  template <>
  void Extension::closeExtension<####> (void * cookie, pami_result_t & result)
  {
    result = PAMI_UNIMPL;
    return;
  }

  ///
  /// \brief Query the extension for a named function
  ///
  /// Returns a function pointer to the static function associated with the
  /// input parameter name.
  ///
  /// \todo Replace the template parameter with the unique extension identifier
  ///       assigned to the specific extension
  ///
  template <>
  void * Extension::queryExtension<####> (const char * name, void * cookie)
  {
    if (strcasecmp (name, "foo") == 0)
      return (void *) PAMI::ExtensionTest::foo;

    if (strcasecmp (name, "bar") == 0)
      return (void *) PAMI::ExtensionTest::bar;

    return NULL;
  };
};

#endif // __pami_extension_template_extension__
#endif // __api_extension_c_template_extension_Extension_h__
