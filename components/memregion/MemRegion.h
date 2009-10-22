/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/memregion/MemRegion.h
 * \brief ???
 */

#ifndef __components_memregion_MemRegion_h__
#define __components_memregion_MemRegion_h__

#include "sys/xmi.h"

#ifndef XMI_MEMREGION_CLASS
#error XMI_MEMREGION_CLASS must be defined!
#endif

namespace XMI
{
  namespace MemRegion
  {
    ///
    /// \brief memory regions.
    ///
    /// \param T_MemRegion MemRegion template class
    ///
    template <class T_MemRegion>
    class MemRegion
    {
      public:
        ///
        /// \brief memory region constructor.
        ///
        inline MemRegion (xmi_context_t context) {};

        ///
        /// \brief Create a memory region.
        ///
        /// \attention All memory region derived classes \b must
        ///            implement the createMemRegion_impl() method.
        ///
        /// \param[out] bytes_out Actual number of bytes pinned
        /// \param[in]  bytes_in  Requested number of bytes to be pinned
        /// \param[in]  base      Requested base virtual address
        /// \param[in]  options   ???
        ///
        /// \retval XMI_SUCCESS The entire memory region, or a portion of
        ///                      the memory region was pinned. The actual
        ///                      number of bytes pinned from the start of the
        ///                      buffer is returned in the \c bytes_out
        ///                      field. The memory region must be free'd with
        ///                      with destroyMemRegion().
        ///
        /// \retval XMI_EAGAIN  The memory region was not pinned due to an
        ///                      unavailable resource. The memory region does
        ///                      not need to be freed with destroyMemRegion().
        ///
        /// \retval XMI_INVAL   An invalid parameter value was specified.
        ///
        /// \retval XMI_ERROR   The memory region was not pinned and does not need to
        ///                      be freed with destroyMemRegion().
        ///
        /// \see destroyMemRegion
        /// \see XMI_MemRegion_create()
        ///
        inline xmi_result_t createMemRegion (size_t   * bytes_out,
                                             size_t     bytes_in,
                                             void     * base,
                                             uint64_t   options);

        ///
        /// \attention All memory region derived classes \b must
        ///            implement the destroyMemRegion_impl() method.
        ///
        /// \copydoc XMI::MemRegion::destroy()
        ///
        inline xmi_result_t destroyMemRegion ();

        ///
        /// \attention All memory region derived classes \b must
        ///            implement the getInfo_impl() method.
        ///
        /// \copydoc XMI::MemRegion::query()
        ///
        inline xmi_result_t getInfo (size_t * bytes, void ** base);

        ///
        /// \brief Get base virtual address
        ///
        /// Get the base virtual address stored within the memory region.
        ///
        /// \attention All memory region derived classes \b must
        ///            implement the getBaseVirtualAddress_impl() method.
        ///
        /// \retval base Base virtual address
        ///
        inline void * getBaseVirtualAddress ();
    };

    template <class T_MemRegion>
    inline xmi_result_t MemRegion<T_MemRegion>::createMemRegion(size_t  * bytes_out,
                                                                size_t    bytes_in,
                                                                void    * base,
                                                                uint64_t  options)
    {
      return static_cast<T_MemRegion*>(this)->createMemRegion_impl(bytes_out, bytes_in, base, options);
    }

    template <class T_MemRegion>
    inline xmi_result_t MemRegion<T_MemRegion>::destroyMemRegion()
    {
      return static_cast<T_MemRegion*>(this)->destroyMemRegion_impl();
    }

    template <class T_MemRegion>
    inline xmi_result_t MemRegion<T_MemRegion>::getInfo(size_t * bytes, void ** base)
    {
      return static_cast<T_MemRegion*>(this)->getInfo_impl(bytes, base);
    }

    template <class T_MemRegion>
    inline void * MemRegion<T_MemRegion>::getBaseVirtualAddress()
    {
      return static_cast<T_MemRegion*>(this)->getBaseVirtualAddress_impl();
    }
  };
};
#endif /* __xmi__memregion_h__ */

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
