/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
#ifndef __common_bgq_BgqPersonality_h__
#define __common_bgq_BgqPersonality_h__
///
///  \file common/bgq/BgqPersonality.h
///  \brief Blue Gene/Q Personality Object
///
///  This object currently provides the following features:
///  - Access to coordinate information
///  - Access to partition size information
///  - Access to operating modes (vnm, etc)
///  - Access to pset information
///  - Ability to dump the entire personality
///
///  Definitions:
///  - Personality is an object that returns machine specific hardware info
///

#define PERS_SIZE 1024

#include <sys/types.h>
#include <util/common.h>

#include <unistd.h>
#include <stdlib.h>

#include <firmware/include/personality.h>
#include <kernel/location.h>
#include <kernel/process.h>

#ifdef ENABLE_MAMBO_WORKAROUNDS
#undef TRACE_MAMBO
#define TRACE_MAMBO(x) //fprintf x
#else
#undef TRACE_MAMBO
#define TRACE_MAMBO(x)
#endif

//#define FAKE_PERSONALITY

namespace PAMI
{
  class BgqPersonality : public Personality_t
  {
    public:

      BgqPersonality ();

      void location (char location[], size_t size);
      void dumpPersonality ();

      ///
      /// \brief Retrieves the rank of the task in this job.
      ///
      /// deprecated - Rank is not part of the personality
      //size_t rank() const { return Network_Config.Rank; }


      ///
      /// \brief Retrieves the 'A' coordinate of the node
      ///
      size_t aCoord() const { return Network_Config.Acoord; }

      ///
      /// \brief Retrieves the 'B' coordinate of the node
      ///
      size_t bCoord() const { return Network_Config.Bcoord; }

      ///
      /// \brief Retrieves the 'C' coordinate of the node
      ///
      size_t cCoord() const { return Network_Config.Ccoord; }

      ///
      /// \brief Retrieves the 'D' coordinate of the node
      ///
      size_t dCoord() const { return Network_Config.Dcoord; }

      ///
      /// \brief Retrieves the 'E' coordinate of the node
      ///
      size_t eCoord() const { return Network_Config.Ecoord; }

      ///
      /// \brief Retrieves the core id of the node
      ///
      ///
      size_t core() const { return _core; }

      ///
      /// \brief Retrieves the 'P' coordinate of the node
      ///
      /// a.k.a. the processor id starting at 0 and
      /// incrementing to pSize. Not the same as hardware core!
      ///
      size_t pCoord() const { return _pCoord; }

      ///
      /// \brief Retrieves the 'T' coordinate of the node
      ///
      /// a.k.a. the thread id on the core starting at 0 and
      /// incrementing to tSize.  Not the same as hwThread!
      ///
      size_t tCoord() const { return _tCoord; }

      ///
      /// \brief Retrieves the 'hardware thread id' on the core
      ///
      size_t thread() const { return _hwthread; }

      ///
      /// \brief Retrieves the 'thread id' on the node
      ///
      size_t tid() const { return thread() + core()*maxThreads(); }

      ///
      /// \brief Retrieves the size of the 'A' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t aSize()  const { return Network_Config.Anodes; }

      ///
      /// \brief Retrieves the size of the 'B' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t bSize()  const { return Network_Config.Bnodes; }

      ///
      /// \brief Retrieves the size of the 'C' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t cSize()  const { return Network_Config.Cnodes; }

      ///
      /// \brief Retrieves the size of the 'D' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t dSize()  const { return Network_Config.Dnodes; }

      ///
      /// \brief Retrieves the size of the 'E' dimension.
      /// \note  Does not consider the mapping.
      ///
      size_t eSize()  const { return Network_Config.Enodes; }

      ///
      /// \brief Retrieves the size of the 'P' dimension.
      ///
      /// The 'P' coordinate identifies the core
      /// starting at 0 and incrementing to pSize.
      ///
      size_t pSize()  const { return _cores; }

      ///
      /// \brief Retrieves the max number of cores, not the number active
      ///
      size_t maxCores() const { return 16; }/// \todo max cores == 16?

      ///
      /// \brief Retrieves the max number of threads, not the number active
      ///
      size_t maxThreads() const { return 4; }/// \todo max threads == 4?

      ///
      /// \brief Retrieves the size of the 'T' dimension.
      ///
      /// The 'T' coordinate identifies the hardware thread.
      ///
      /// \note  Does not consider the mapping.
      ///
      size_t tSize()  const { return _hwthreads; }

      ///
      /// \brief Get the size of NODE memory
      /// \return _node_ memory size in MiB
      ///
      size_t  memSize()  const { return DDR_Config.DDRSizeMB;}

      ///
      /// \brief Gets the clock speed in MEGA-Hz
      /// \return MHz
      ///
      size_t  clockMHz()  const { return Kernel_Config.FreqMHz; }

    protected:

      size_t _tCoord;
      size_t _pCoord;
      size_t _core;
      size_t _hwthread;

      bool   _torusA;
      bool   _torusB;
      bool   _torusC;
      bool   _torusD;
      bool   _torusE;
#ifdef ENABLE_MAMBO_WORKAROUNDS
    public:
      bool   _is_mambo; // Indicates whether mambo is being used
    protected:
#endif

      size_t _cores;
      size_t _hwthreads;
  };	// class BgqPersonality
};	// namespace PAMI

#endif // __pami_components_sysdep_bgq_bgqpersonnality_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
