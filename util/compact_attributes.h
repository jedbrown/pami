/**
 * \file util/compact_attributes.h
 * \brief ???
 */

#ifndef __util_compact_attributes_h__
#define __util_compact_attributes_h__



/* denotes the end of arguments in variadic functions */
#define CA_END_ARGS   -1

#define CA_BIT(b)       ((b) / PAMI_CA_TOTAL_BITS)
#define CA_MASK(b)      ((pami_ca_mask) 1 << ((b) % PAMI_CA_TOTAL_BITS))

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
    unsigned int __i;                                                         \
    pami_ca_t *__list = (l);                                                    \
    for (__i = 0; __i < PAMI_CA_NUM_ELEMENTS; __i++)                               \
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
    pami_ca_t  *__src = (s);                                                    \
    pami_ca_t  *__dst = (d);                                                    \
    for (__i = 0; __i < PAMI_CA_NUM_ELEMENTS; ++__i)                               \
      CA_ELEMENT (__dst)[__i] |= CA_ELEMENT (__src)[__i];                     \
  } while (0)


/* utility macro to print '0' or '1' to indicate attributes are set/unset */
#define CA_PRINT(l)                                                           \
  do                                                                          \
  {                                                                           \
    unsigned int __i, __bit, __element;                                       \
    for (__element = PAMI_CA_NUM_ELEMENTS - 1; __element >=0; __element--)         \
    {                                                                         \
      for (__i = PAMI_CA_TOTAL_BITS - 1; i >= 0; i--)                              \
      {                                                                       \
        if (((__i+1) % 4 == 0) && (__i+1) != PAMI_CA_TOTAL_BITS) printf("-");      \
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
    pami_ca_t *__l1 = (l1);                                                     \
    pami_ca_t *__l2 = (l2);                                                     \
    for (__i = 0; __i < PAMI_CA_NUM_ELEMENTS; __i++)                               \
      if ((CA_ELEMENT (__l1)[__i] & CA_ELEMENT (__l2)[__i]) !=                \
          CA_ELEMENT (__l2)[__i])                                             \
      {                                                                       \
        result = 0;                                                           \
        break;                                                                \
      }                                                                       \
  } while (0)                                                                 \




#define pami_ca_copy      CA_COPY
#define pami_ca_set       CA_SET
#define pami_ca_unset     CA_UNSET
#define pami_ca_isset     CA_ISSET
#define pami_ca_unset_all CA_UNSET_ALL
#define pami_ca_set_all   CA_SET_ALL
#define pami_ca_issubset  CA_ISSUBSET
#define pami_ca_print     CA_PRINT

#endif
