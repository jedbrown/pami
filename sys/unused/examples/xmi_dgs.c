/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file sys/examples/xmi_dgs.c
 * \brief ???
 */

#include <xmi.h>

void CreateType()
{
    double       mesh[10][20];

    // create a type for a double
    xmi_type_t   double_type;
    XMI_Type_create(&double_type);
    XMI_Type_add_simple(double_type, sizeof(double), 0, 1, sizeof(double));
    XMI_Type_complete(double_type);

    // create a type for a row
    // NOTE: can be internally optimized to a single contiguous buffer
    xmi_type_t   row_type;
    XMI_Type_create(&row_type);
    XMI_Type_add_typed(row_type, double_type,
            0,                 // offset at 1st element
            20,                // repead 20 times
            sizeof(double));   // stride in 1 element
    XMI_Type_complete(row_type);

    // create a type for odd elements in a row
    // NOTE: can be internally optimized to strided buffers
    xmi_type_t   odd_type;
    XMI_Type_create(&odd_type);
    XMI_Type_add_typed(odd_type, double_type,
            sizeof(double),    // offset at 2nd element
            10,                // repead 10 times
            2*sizeof(double)); // stride in 2 element
    XMI_Type_complete(odd_type);

    // create a type for a column
    // NOTE: can be internally optimized to strided buffers
    xmi_type_t   column_type;
    XMI_Type_create(&column_type);
    XMI_Type_add_typed(column_type, double_type,
            0,                 // offset at 1st element
            10,                // repeat 10 times
            20*sizeof(double));// stride in 20 elements
    XMI_Type_complete(column_type);

    // create a type for the first 2 and the last 3 elements in a column
    xmi_type_t   end_type;
    XMI_Type_create(&end_type);
    XMI_Type_add_typed(&end_type, double_type,
            0,                 // offset at 1st element
            2,                 // repeat 2 times
            20*sizeof(double));// stride in 20 element
    XMI_Type_add_typed(&end_type, double_type,
            7*20*sizeof(double),// offset at 8th element
            3,                 // repeat 3 times
            20*sizeof(double));// stride in 1 element
    XMI_Type_complete(&end_type);
}

void my_unpack(void *dst, void *src, size_t size, void *dst_dgsm)
{
    XMI_Dgsm_unpack((xmi_dgsm_t *)dst_dgsm, src, size);
}

void TypedCopy1(void *dst_addr, xmi_type_t dst_type,
                void *src_addr, xmi_type_t src_type)
{
    xmi_dgsm_t dst_dgsm, src_dgsm;
    XMI_Dgsm_create(dst_addr, dst_addr, &dst_dgsm);
    XMI_Dgsm_create(src_addr, src_addr, &src_dgsm);

    XMI_Dgsm_set_callback(&src_dgsm, my_unpack, &dst_dgsm);
    XMI_Dgsm_pack(&src_dgsm, 0, XMI_Type_sizeof(&dst_type));

    XMI_Dgsm_destroy(dst_dgsm);
    XMI_Dgsm_destroy(src_dgsm);
}

void TypedCopy2(void *dst_addr, xmi_type_t dst_type,
                void *src_addr, xmi_type_t src_type)
{
    xmi_dgsm_t dst_dgsm, src_dgsm;
    XMI_Dgsm_create(dst_addr, dst_addr, &dst_dgsm);
    XMI_Dgsm_create(src_addr, src_addr, &src_dgsm);

    void * block_addr;
    size_t block_size;
    while (XMI_Dgsm_get_block(&src_dgsm, &block_addr, &block_size) &&
            block_size > 0)
        XMI_Dgsm_unpack(&dst_dgsm, block_addr, block_size);

    XMI_Dgsm_destroy(dst_dgsm);
    XMI_Dgsm_destroy(src_dgsm);
}

void reduce_double(void *dst, void *src, size_t size, void *cookie)
{
    double *d = (double *)dst;
    double *s = (double *)src;
    size_t  c = size/sizeof(double);
    while (c--)  *d++ = *s++;
}

void TypedReduce1(void *dst_addr, xmi_type_t dst_type,
                  void *src_addr, xmi_type_t src_type)
{
    xmi_dgsm_t dst_dgsm, src_dgsm;
    XMI_Dgsm_create(dst_addr, dst_addr, &dst_dgsm);
    XMI_Dgsm_create(src_addr, src_addr, &src_dgsm);

    XMI_Dgsm_set_callback(&dst_dgsm, reduce_double, 0);
    XMI_Dgsm_set_callback(&src_dgsm, my_unpack, &dst_dgsm);

    XMI_Dgsm_pack(&src_dgsm, 0, XMI_Type_sizeof(&dst_type));

    XMI_Dgsm_destroy(dst_dgsm);
    XMI_Dgsm_destroy(src_dgsm);
}

void TypedReduce2(void *dst_addr, xmi_type_t dst_type,
                  void *src_addr, xmi_type_t src_type)
{
    xmi_dgsm_t dst_dgsm, src_dgsm;
    XMI_Dgsm_create(dst_addr, dst_addr, &dst_dgsm);
    XMI_Dgsm_create(src_addr, src_addr, &src_dgsm);

    XMI_Dgsm_set_callback(&dst_dgsm, reduce_double, 0);

    void * block_addr;
    size_t block_size;
    while (XMI_Dgsm_get_block(&src_dgsm, &block_addr, &block_size) &&
            block_size > 0)
        XMI_Dgsm_unpack(&dst_dgsm, block_addr, block_size);

    XMI_Dgsm_destroy(dst_dgsm);
    XMI_Dgsm_destroy(src_dgsm);
}
