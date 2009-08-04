#include <xmi_dgs.h>

void CreateType()
{
    double       mesh[10][20];

    // create a type for a double
    xmi_type_t   double_type;
    XMI_Type_create(&double_type);
    XMI_Type_add_simple(double_type, sizeof(double), 0, 1, sizeof(double));
    XMI_Type_commit(double_type);
    
    // create a type for a row
    // NOTE: can be internally optimized to a single contiguous buffer
    xmi_type_t   row_type;
    XMI_Type_create(&row_type);
    XMI_Type_add_typed(row_type, double_type, 
            0,                 // offset at 1st element
            20,                // repead 20 times
            sizeof(double));   // stride in 1 element
    XMI_Type_commit(row_type);

    // create a type for odd elements in a row
    // NOTE: can be internally optimized to strided buffers
    xmi_type_t   odd_type;
    XMI_Type_create(&odd_type);
    XMI_Type_add_typed(odd_type, double_type, 
            sizeof(double),    // offset at 2nd element
            10,                // repead 10 times
            2*sizeof(double)); // stride in 2 element
    XMI_Type_commit(odd_type);

    // create a type for a column 
    // NOTE: can be internally optimized to strided buffers
    xmi_type_t   column_type;
    XMI_Type_create(&column_type);
    XMI_Type_add_typed(column_type, double_type, 
            0,                 // offset at 1st element
            10,                // repeat 10 times
            20*sizeof(double));// stride in 20 elements
    XMI_Type_commit(column_type);

    // create a type for the first 2 and the last 3 elements in a column
    xmi_type_t   end_type;
    XMI_Type_create(&end_type);
    XMI_Type_add_typed(&end_type, double_type,
            0,                 // offset at 1st element
            2,                 // repeat 2 times
            sizeof(double));   // stride in 1 element
    XMI_Type_add_typed(&end_type, double_type,
            7*sizeof(double),  // offset at 8th element
            3,                 // repeat 2 times
            sizeof(double));   // stride in 1 element
    XMI_Type_commit(&end_type);
}
