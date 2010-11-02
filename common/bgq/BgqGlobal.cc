/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file common/bgq/BgqGlobal.cc
/// \brief Blue Gene/Q Global Object implementation
///

#include "Global.h"
#include "Topology.h" // need to make static members...

PAMI::Global __global;

PAMI::Mapping *PAMI::Topology::mapping = NULL;
pami_coord_t PAMI::Topology::my_coords;

#include "components/devices/misc/ProgressFunctionMsg.h"
PAMI::Device::ProgressFunctionDev _g_progfunc_dev;

#include "common/bgq/ResourceManager.h"
PAMI::ResourceManager __pamiRM;

#include "components/devices/bgq/mu2/global/Global.h"
PAMI::Device::MU::Global __MUGlobal ( __pamiRM, __global.mapping, __global.personality, __global.mm );
PAMI::Device::MU::Context   *__mu_contexts; //Short term fix to get internal tests working

const double PAMI::Time::seconds_per_cycle = 6.25e-10;

#ifdef ENABLE_MAMBO_WORKAROUNDS
extern "C" unsigned __isMambo() {return __global.personality._is_mambo? 1:0;};
#endif

#ifdef USE_COMMTHREADS
#include "components/devices/bgq/commthread/CommThreadFactory.h"
PAMI::Device::CommThread::Factory __CommThreadGlobal(&__global.mm, &__global.l2atomicFactory.__nodescoped_mm);
#endif // USE_COMMTHREADS

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
