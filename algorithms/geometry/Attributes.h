/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/geometry/Attributes.h
 * \brief ???
 */

#ifndef __algorithms_geometry_Attributes_h__
#define __algorithms_geometry_Attributes_h__

#define PAMI_ISPOF2(x)   (!(((x) - 1) & (x)) ? 1 : 0)
#define PAMI_ISEVEN(x)   (((x) % 2) ? 0 : 1)

/* Each '#define' below represents an informative attribute                   */
/*
 * initially, each variable can define up to 32 attributes. One can extend that
 */

/******************************************************************************/
/* Geometry attributes                                                        */
/******************************************************************************/
#define PAMI_GEOMETRY_POF2                                                      0
#define PAMI_GEOMETRY_ODD                                                       1
#define PAMI_GEOMETRY_TREE                                                      2
#define PAMI_GEOMETRY_RECT                                                      3
#define PAMI_GEOMETRY_TORUS                                                     4
#define PAMI_GEOMETRY_STAR                                                      5
#define PAMI_GEOMETRY_CROSSBAR                                                  6
#define PAMI_GEOMETRY_IRREG                                                     7
#define PAMI_GEOMETRY_LOCAL                                                     8
#define PAMI_GEOMETRY_GLOBAL                                                    9

/******************************************************************************/
/* Buffer/Operation/Datatype/msg_size ranges attributes                       */
/******************************************************************************/
#define PAMI_CONTIG_SBUFF                                                       0
#define PAMI_CONTIG_RBUFF                                                       1
#define PAMI_CONTIN_RBUFF                                                       2
#define PAMI_ALIGNED_BUFF                                                       3
#define PAMI_4XBYTE_BUFF                                                        4
#define PAMI_OP_TYPE_TREE                                                       5
#define PAMI_OP_TYPE_TORUS                                                      6
#define PAMI_OP_TYPE_ANY                                                        7
#define PAMI_MSIZE_RANGE_1                                                      8
#define PAMI_MSIZE_RANGE_2                                                      9
#define PAMI_MSIZE_RANGE_3                                                     10
#define PAMI_MSIZE_RANGE_4                                                     11
#define PAMI_MSIZE_RANGE_5                                                     12
#define PAMI_MSIZE_RANGE_6                                                     13
#define PAMI_MSIZE_RANGE_7                                                     14
#define PAMI_MSIZE_RANGE_8                                                     15

/******************************************************************************/
/* platform mode of operation, thread levels, misc                            */
/******************************************************************************/
#define PAMI_MODE_SMP                                                           0
#define PAMI_MODE_DUAL                                                          1
#define PAMI_MODE_VN                                                            2
#define PAMI_MODE_THREADED                                                      3
#define PAMI_MODE_UNTHREADED                                                    4

#endif // __algorithms_geometry_Attributes_h__
