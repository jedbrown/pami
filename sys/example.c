/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file sys/example.c
 * \brief ???
 */

typedef struct
{
  size_t id; /* matches xmi protocol enum */
  size_t order; /* will be used to order protocols based on performance */
  xmi_metadata_t mdata;
} alg_repository;

MPI_Init(......)
{
  int num;
  alg_repository * bcast_repo;
  ....
  XMI_Geometry_algorithms_total (context,
                                 geometry,
                                 XMI_XFER_BROADCAST,
                                 &num);
  if (num)
  {
    bcast_repo = malloc(sizeof(alg_repository) * num);
    for (i = 0; i < num; i++)
      XMI_Geometry_algorithm_applicable (context,
                                         geometry,
                                         XMI_XFER_BROADCAST,
                                         i,
                                         &bcast_repo[i].mdata);
  }
  ....
  //.... do the same for other collectives
}

xmi_result_t XMI_Geometry_algorithm_applicable (xmi_context_t context,
                                                xmi_geometry_t geometry,
                                                xmi_xfer_type_t type,
                                                xmi_algorithm_t algorithm,
                                                xmi_metadata_t *mdata);
{
  // find the approperiate protocol pointed by passed in "algorithm"
  // see if it is applicable to the geometry and context
  ...
  ...

  xmi_ca_list_t *geometry_attr = mdata->geometry_attr;
  xmi_ca_list_t *buffer_attr = mdata->buffer_attr;
  xmi_ca_list_t *misc_attr = mdata->misc_attr;

  //fill in geometry/topology fields
  // assume this algorithm works only on torus topology, then we sit that bit
  xmi_ca_set(geometry_attr, XMI_GEOMETRY_TORUS);
  ...
  ...

  //fill in buffer requirement for this algorithm
  // assume this algorithm works only on aligned data types
  xmi_ca_set(buffer_attr, XMI_ALIGNED_BUFF);
  ...
  ...


  //fill in misc attributes
  // assume this algorithm works only in single thread modes
  xmi_ca_set(misc_attr, XMI_MODE_UNTHREADED);
  ...
  ...
}
