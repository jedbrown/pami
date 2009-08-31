#ifndef __xmi_datatypes_h__
#define __xmi_datatypes_h__

#include <stdlib.h>
#include <stdint.h>
#include "xmi_types.h"
#include "xmi_misc.h"

#ifdef __cplusplus
extern "C"
{
#endif
  /*****************************************************************************/
  /**
   * \defgroup datatype xmi non-contiguous datatype interface
   *
   * Some brief documentation on active message stuff ...
   * \{
   */
  /*****************************************************************************/

  /**
   * \brief Create a new type for noncontiguous transfers
   *
   * \todo provide example code
   *
   * \param[out] type Type identifier to be created
   */
  xmi_result_t XMI_Type_create (xmi_type_t * type);

  /**
   * \brief Append a simple contiguous buffer to an existing type identifier
   *
   * \todo doxygen for offset parameter
   * \todo provide example code
   *
   * \param[in,out] type   Type identifier to be modified
   * \param[in]     bytes  Number of contiguous bytes to append
   * \param[in]     offset Offset from the end of the type to place the buffer
   * \param[in]     count  Number of buffers
   * \param[in]     stride Data stride
   */
  xmi_result_t XMI_Type_add_simple (xmi_type_t type,
                                    size_t     bytes,
                                    size_t     offset,
                                    size_t     count,
                                    size_t     stride);

  /**
   * \brief Append a typed buffer to an existing type identifier
   *
   * \todo doxygen for offset parameter
   * \todo provide example code
   *
   * \warning It is considered \b illegal to append an imcomplete type to
   *          another type.
   *
   * \param[in,out] type    Type identifier to be modified
   * \param[in]     subtype Subtype to append
   * \param[in]     offset  Offset from the end of the type to place the buffer
   * \param[in]     count   Number of buffers
   * \param[in]     stride  Data stride
   */
  xmi_result_t XMI_Type_add_typed (xmi_type_t type,
                                   xmi_type_t subtype,
                                   size_t     offset,
                                   size_t     count,
                                   size_t     stride);

  /**
   * \brief Complete the type identifier
   *
   * \warning It is considered \b illegal to modify a type identifier after it
   *          has been completed.
   *
   * \param[in] type Type identifier to be completed
   */
  xmi_result_t XMI_Type_complete (xmi_type_t type);

  /**
   * \brief Get the byte size of a completed type
   *
   * \param[in] type Type identifier to get size from
   */
  xmi_result_t XMI_Type_sizeof (xmi_type_t type);

  /**
   * \brief Destroy the type
   *
   * Q. What if some in-flight messages are still using it?  What if some
   * other types have references to it?
   *
   * A. Maintain an internal reference count and release internal type
   * resources when the count hits zero.
   *
   * \param[in] type Type identifier to be destroyed
   */
  xmi_result_t XMI_Type_destroy (xmi_type_t type);

  /*
   * \brief Pack data from a non-contiguous buffer to a contiguous buffer
   *
   * \param[in] src_type   source data type
   * \param[in] src_offset starting offset of source data type
   * \param[in] src_addr   starting address of source buffer
   * \param[in] dst_addr   starting address of destination buffer
   * \param[in] dst_size   destination buffer size
   */
  xmi_result_t XMI_Type_pack_data (xmi_type_t src_type,
                                   size_t     src_offset,
                                   void     * src_addr,
                                   void     * dst_addr,
                                   size_t     dst_size);

  /*
   * \brief Unpack data from a contiguous buffer to a non-contiguous buffer
   *
   * \param[in] dst_type   destination data type
   * \param[in] dst_offset starting offset of destination data type
   * \param[in] dst_addr   starting address of destination buffer
   * \param[in] src_addr   starting address of source buffer
   * \param[in] src_size   source buffer size
   */
  xmi_result_t XMI_Type_unpack_data (xmi_type_t dst_type,
                                     size_t     dst_offset,
                                     void     * dst_addr,
                                     void     * src_addr,
                                     size_t     src_size);
  /** \} */ /* end of "datatype" group */

#ifdef __cplusplus
};
#endif

#endif /* __xmi_p2p_h__ */
