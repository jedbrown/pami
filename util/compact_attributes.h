#ifndef _COMPACT_ATTRIBUTES_H_
#define _COMPACT_ATTRIBUTES_H_

/* denotes the end of arguments in variadic functions */
#define CA_END_ARGS   -1

#define CA_BIT(b)       ((b) / CA_TOTAL_BITS)
#define CA_MASK(b)      ((CA_Mask) 1 << ((b) % CA_TOTAL_BITS))

#define CA_ELEMENT(list) ((list)->bits)

/* sets a bit in the attribute list */
#define CA_SET(l, a)   (CA_ELEMENT (l)[CA_BIT(a)] |= CA_MASK(a))

/* unsets a bit in the attribute list */
#define CA_UNSET(l, a) (CA_ELEMENT (l)[CA_BIT(a)] &= ~CA_MASK(a))

/* checks an attribute bit in the attribute list */
#define CA_ISSET(l, a) ((CA_ELEMENT (l)[CA_BIT(a)] & CA_MASK(a)) != 0)


/* unset all attributes bits */
#define CA_UNSET_ALL(l)                                                       \
  do                                                                          \
  {                                                                           \
    unsigned int __i, __j;                                                    \
    xmi_ca_t *__list = (l);                                                    \
    for (__i = 0; __i < CA_NUM_ELEMENTS; __i++)                               \
      CA_ELEMENT (__list)[__i] = 0;                                           \
  } while (0)

/* set all attributes bits up to a range 'r' (i.e. from 0 .. r) */
#define CA_SET_ALL(l, r)                                                      \
  do                                                                          \
  {                                                                           \
    unsigned int __i;                                                         \
    for (__i = 0; __i < r; ++__i)  CA_SET(l, __i);                            \
  } while (0)

/* copy attribute list s into list d */
#define CA_COPY(s, d)                                                         \
  do                                                                          \
  {                                                                           \
    unsigned int __i, __j;                                                    \
    xmi_ca_t  *__src = (s);                                                    \
    xmi_ca_t  *__dst = (d);                                                    \
    for (__i = 0; __i < CA_NUM_ELEMENTS; ++__i)                               \
      CA_ELEMENT (__dst)[__i] |= CA_ELEMENT (__src)[__i];                     \
  } while (0)


/* utility macro to print '0' or '1' to indicate attributes are set/unset */
#define CA_PRINT(l)                                                           \
  do                                                                          \
  {                                                                           \
    unsigned int __i, __bit, __element;                                       \
    for (__element = CA_NUM_ELEMENTS - 1; __element >=0; __element--)         \
    {                                                                         \
      for (__i = CA_TOTAL_BITS - 1; i >= 0; i--)                              \
      {                                                                       \
        if (((__i+1) % 4 == 0) && (__i+1) != CA_TOTAL_BITS) printf("-");      \
        __bit = (((CA_ELEMENT (l)[__element]) >> __i) & 1);                   \
        printf("%d", __bit);                                                  \
      }                                                                       \
      printf("-");                                                            \
    }                                                                         \
    printf("\n");                                                             \
  } while (0)                                                                 \

/* is 'l1' attribute list a subset of 'l2' list, if so, 'result' is true */
#define CA_ISSUBSET(l1, l2, result)                                           \
  do                                                                          \
  {                                                                           \
    result = 1;                                                               \
    unsigned int __i;                                                         \
    xmi_ca_t *__l1 = (l1);                                                     \
    xmi_ca_t *__l2 = (l2);                                                     \
    for (__i = 0; __i < CA_NUM_ELEMENTS; __i++)                               \
      if ((CA_ELEMENT (__l1)[__i] & CA_ELEMENT (__l2)[__i]) !=                \
          CA_ELEMENT (__l2)[__i])                                             \
      {                                                                       \
        result = 0;                                                           \
        break;                                                                \
      }                                                                       \
  } while (0)                                                                 \

/* this sets multiple attributes in a list in a single call */
/* extern void CA_MSET(xmi_ca_t *list, ...);*/


  /**
   * \brief Macro to set a collective algorithm pointed by 'algorithm' to
   *        default protocol for a given transfer type.
   *
   * \param[in]      coll_type  Type of xmi transfer operation
   * \param[in/out]  algorithm  variable to set the default protocol to.
   */
#define XMI_COLLECTIVE_ALGORITHM_DEFAULT(coll_type, algorithm)          \
  do                                                                    \
      {                                                                 \
        switch(coll_type)                                               \
            {                                                           \
                case XMI_XFER_BROADCAST:                                \
                  algorithm = XMI_BROADCAST_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_ALLREDUCE:                                \
                  algorithm = XMI_ALLREDUCE_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_REDUCE:                                   \
                  algorithm = XMI_REDUCE_ALGORITHM_DEFAULT;             \
                  break;                                                \
                case XMI_XFER_ALLGATHER:                                \
                  algorithm = XMI_ALLGATHER_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_ALLGATHERV:                               \
                  algorithm = XMI_ALLGATHERV_ALGORITHM_DEFAULT;         \
                  break;                                                \
                case XMI_XFER_ALLGATHERV_INT:                           \
                  algorithm = XMI_ALLGATHERV_INT_ALGORITHM_DEFAULT;     \
                  break;                                                \
                case XMI_XFER_SCATTERV:                                 \
                  algorithm = XMI_SCATTERV_ALGORITHM_DEFAULT;           \
                  break;                                                \
                case XMI_XFER_SCATTER:                                  \
                  algorithm = XMI_SCATTER_ALGORITHM_DEFAULT;            \
                  break;                                                \
                case XMI_XFER_SCATTER_INT:                              \
                  algorithm = XMI_SCATTER_INT_ALGORITHM_DEFAULT;        \
                  break;                                                \
                case XMI_XFER_BARRIER:                                  \
                  algorithm = XMI_BARRIER_ALGORITHM_DEFAULT;            \
                  break;                                                \
                case XMI_XFER_ALLTOALL:                                 \
                  algorithm = XMI_ALLTOALL_ALGORITHM_DEFAULT;           \
                  break;                                                \
                case XMI_XFER_ALLTOALLV:                                \
                  algorithm = XMI_ALLTOALLV_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_ALLTOALLV_INT:                            \
                  algorithm = XMI_ALLTOALLV_INT_ALGORITHM_DEFAULT;      \
                  break;                                                \
                case XMI_XFER_SCAN:                                     \
                  algorithm = XMI_SCAN_ALGORITHM_DEFAULT;               \
                  break;                                                \
                case XMI_XFER_AMBROADCAST:                              \
                  algorithm = XMI_AMBROADCAST_ALGORITHM_DEFAULT;        \
                  break;                                                \
                case XMI_XFER_AMSCATTER:                                \
                  algorithm = XMI_AMSCATTER_ALGORITHM_DEFAULT;          \
                  break;                                                \
                case XMI_XFER_AMGATHER:                                 \
                  algorithm = XMI_AMGATHER_ALGORITHM_DEFAULT;           \
                  break;                                                \
                case XMI_XFER_AMREDUCE:                                 \
                  algorithm = XMI_AMREDUCE_ALGORITHM_DEFAULT;           \
                  break;                                                \
                default:                                                \
                  printf("XMI: invalid xmi_xfer_type\n");               \
                  break;                                                \
            }                                                           \
      }                                                                 \
  while (0);                                                            \

#define xmi_ca_copy      CA_COPY
#define xmi_ca_set       CA_SET
#define xmi_ca_unset     CA_UNSET
#define xmi_ca_isset     CA_ISSET
#define xmi_ca_unset_all CA_UNSET_ALL
#define xmi_ca_set_all   CA_SET_ALL
#define xmi_ca_issubset  CA_ISSUBSET
#define xmi_ca_print     CA_PRINT

#endif
