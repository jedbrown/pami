
#include <xmi.h>
#include "../../interface/ll_pipeworkqueue.h"

inline void dgs_pack (void       * src_buffer,
                      xmi_type_t   src_type,
                      void       * dst_buffer,
                      size_t       bytes)
{
  LL_PipeWorkQueue_t src;
  LL_PipeWorkQueue_configure_typed (LL_PipeWorkQueue_t &src, src_type, (char *)src_buffer, bytes, bytes);

  uint8_t * dst_cursor = (uint8_t *) dst_buffer;

  while (bytes > 0)
  {
    size_t bytes_to_copy = LL_PipeWorkQueue_bytesAvailableToConsume (src);
    char * src_cursor = LL_PipeWorkQueue_bufferToConsume (src);

    memcpy ((void *)dst_cursor, (void *)src_cursor, bytes_to_copy);

    LL_PipeWorkQueue_consumeBytes (src, bytes_to_copy);
    dst_cursor += bytes_to_copy;
    bytes -= bytes_to_copy;
  }
}


inline void dgs_unpack (void       * dst_buffer,
                        xmi_type_t   dst_type,
                        void       * src_buffer,
                        size_t       bytes)
{
  LL_PipeWorkQueue_t dst;
  LL_PipeWorkQueue_configure_typed (LL_PipeWorkQueue_t &dst, dst_type, (char *)dst_buffer, bytes, 0);

  uint8_t * src_cursor = (uint8_t *) src_buffer;

  while (bytes > 0)
  {
    size_t bytes_to_copy = LL_PipeWorkQueue_bytesAvailableToProduce (dst);
    char * dst_cursor = LL_PipeWorkQueue_bufferToProduce (dst);

    memcpy ((void *)dst_cursor, (void *)src_cursor, bytes_to_copy);

    LL_PipeWorkQueue_produceBytes (dst, bytes_to_copy);
    src_cursor += bytes_to_copy;
    bytes -= bytes_to_copy;
  }
}




inline void pwqcpy (LL_PipeWorkQueue_t * in, LL_PipeWorkQueue_t * out, size_t total_bytes)
{
  while (total_bytes > 0)
  {
    size_t bytes_to_produce = LL_PipeWorkQueue_bytesAvailableToProduce (out);
    if (bytes_to_produce > 0)
    {
      size_t bytes_to_consume = LL_PipeWorkQueue_bytesAvailableToConsume (in);
      if (bytes_to_consume > 0)
      {
        size_t bytes_to_copy = MIN(bytes_to_produce,bytes_to_consume);
        char * produce_buffer = LL_PipeWorkQueue_bufferToProduce (out);
        char * consume_buffer = LL_PipeWorkQueue_bufferToConsume (in);

        memcpy ((void *)produce_buffer, (void *)consume_buffer, bytes_to_copy);

        LL_PipeWorkQueue_produceBytes (out, bytes_to_copy);
        LL_PipeWorkQueue_consumeBytes (in, bytes_to_copy);

        total_bytes -= bytes_to_copy;
      }
    }
  }
}

inline void dgs_transpose (void       * src_buffer,
                           xmi_type_t   src_type,
                           void       * dst_buffer,
                           xmi_type_t   dst_type,
                           size_t       bytes)
{
  LL_PipeWorkQueue_t dst;
  LL_PipeWorkQueue_configure_typed (LL_PipeWorkQueue_t &dst, dst_type, (char *)dst_buffer, bytes, 0);

  LL_PipeWorkQueue_t src;
  LL_PipeWorkQueue_configure_typed (LL_PipeWorkQueue_t &src, src_type, (char *)src_buffer, bytes, bytes);

  pwqcpy (&src, &dst, bytes);
}





