/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/mpi/GenericSubDeviceList.cc
 * \brief ???
 */

#include "GenericSubDeviceList.h"

// This gets included in a C++ file so it needs to actually instantiate things...

XMI::Device::MPISyncDev  _g_mpisync_dev;

XMI::Device::MPIBcastDev _g_mpibcast_dev;
// The generic bcast device needs its own communicator so it doesn't conflict with XMI::Device::MPIDevice
MPI_Comm                 _g_mpibcast_communicator; 

