/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file arch/ArchInterface.h
 * \brief Defines common and default macros and checks for required macro definitions.
 */

#ifndef __arch_ArchInterface_h__
#define __arch_ArchInterface_h__

///
/// \brief Encapuslating class for static methods which allows partial template specialization
///
template <class T>
class Type
{
  public:
    inline  Type () {};
    inline ~Type () {};

    template <unsigned N>
    static inline void copy (T * dst, T * src)
    {
      uint8_t * const d = (uint8_t * const) dst;
      uint8_t * const s = (uint8_t * const) src;

      size_t i;
      for (i=0; i<N; i++) d[i] = s[i];
    };
};


///
/// \brief Template specialization for uint32_t copies.
///
template <>
template <unsigned N>
void Type<uint32_t>::copy (uint32_t * dst, uint32_t * src)
{
  size_t i;
  for (i=0; i<(N>>2); i++) dst[i] = src[i];

  if (N & 0x03)
  {
    uint8_t * const d = (uint8_t * const) dst[N>>2];
    uint8_t * const s = (uint8_t * const) src[N>>2];
    for (i=0; i<(N&0x03); i++) d[i] = s[i];
  }
};

#ifndef XMI_NO_SIZE_T_PROTO
///
/// \brief Template specialization for size_t copies.
///
template <>
template <unsigned N>
void Type<size_t>::copy (size_t * dst, size_t * src)
{
  size_t i;
  for (i=0; i<(N/sizeof(size_t)); i++) dst[i] = src[i];

  if (N%(sizeof(size_t)))
  {
    uint8_t * const d = (uint8_t * const) dst[N%(sizeof(size_t))];
    uint8_t * const s = (uint8_t * const) src[N%(sizeof(size_t))];

    for (i=0; i<(N%(sizeof(size_t))); i++) d[i] = s[i];
  }
};
#else // XMI_NO_SIZE_T_PROTO
 #warning (XMI_NO_SIZE_T_PROTO) size_t and unsigned definitions conflict
#endif // XMI_NO_SIZE_T_PROTO


#endif // __xmi_arch_common_h__
