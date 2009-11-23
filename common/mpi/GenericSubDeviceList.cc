/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#include "GenericSubDeviceList.h"

// This gets included in a C++ file so it needs to actually instantiate things...

XMI::Device::MPIBcastDev _g_mpibcast_dev;
// The generic device needs its own communicator so it doesn't conflict with XMI::Device::MPIDevice
MPI_Comm                 _g_mpi_communicator; 
