/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
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


#define FAKE_PERSONALITY

namespace XMI
{
    class BgqPersonality : public Personality_t
    {
      public:

        BgqPersonality ();

        void location (char location[], size_t size);
        void dumpPersonality ();

        ///
        /// \brief Retreives the rank of the task in this job.
        ///
        size_t rank() const { return Network_Config.Rank; }


        ///
        /// \brief Retreives the 'A' coordinate of the node
        ///
        size_t aCoord() const { return Network_Config.Acoord; }

        ///
        /// \brief Retreives the 'B' coordinate of the node
        ///
        size_t bCoord() const { return Network_Config.Bcoord; }

        ///
        /// \brief Retreives the 'C' coordinate of the node
        ///
        size_t cCoord() const { return Network_Config.Ccoord; }

        ///
        /// \brief Retreives the 'D' coordinate of the node
        ///
        size_t dCoord() const { return Network_Config.Dcoord; }

        ///
        /// \brief Retreives the 'E' coordinate of the node
        ///
        size_t eCoord() const { return Network_Config.Ecoord; }

        ///
        /// \brief Retreives the 'P' coordinate of the node
        ///
        /// a.k.a. the processor (core) id.
        ///
        size_t pCoord() const { return _core; }

        ///
        /// \brief Retreives the 'T' coordinate of the node
        ///
        /// a.k.a. the hardware thread id on the core
        ///
        size_t tCoord() const { return _hwthread; }






        ///
        /// \brief Retreives the size of the 'A' dimension.
        /// \note  Does not consider the mapping.
        ///
        size_t aSize()  const { return Network_Config.Anodes; }

        ///
        /// \brief Retreives the size of the 'B' dimension.
        /// \note  Does not consider the mapping.
        ///
        size_t bSize()  const { return Network_Config.Bnodes; }

        ///
        /// \brief Retreives the size of the 'C' dimension.
        /// \note  Does not consider the mapping.
        ///
        size_t cSize()  const { return Network_Config.Cnodes; }

        ///
        /// \brief Retreives the size of the 'D' dimension.
        /// \note  Does not consider the mapping.
        ///
        size_t dSize()  const { return Network_Config.Dnodes; }

        ///
        /// \brief Retreives the size of the 'E' dimension.
        /// \note  Does not consider the mapping.
        ///
        size_t eSize()  const { return Network_Config.Enodes; }

        ///
        /// \brief Retreives the size of the 'P' dimension.
        ///
        /// The 'P' coordinate identifies the processor core.
        ///
        /// \note  Does not consider the mapping.
        ///
        size_t pSize()  const { return _cores; }

        ///
        /// \brief Retreives the size of the 'T' dimension.
        ///
        /// The 'T' coordinate identifies the hardware thread.
        ///
        /// \note  Does not consider the mapping.
        ///
        size_t tSize()  const { return _hwthreads; }

#if 0
        ///
        /// \brief Returns if the partition is a torus
        /// \return boolean true if the partition is a torus in the X dimension
        ///         false if the partition is a mesh in the X dimension
        ///
        bool isTorusX ()    const { return _isTorusX; }

        ///
        /// \brief Returns if the partition is a torus
        /// \return boolean true if the partition is a torus in the Y dimension
        ///         false if the partition is a mesh in the Y dimension
        ///
        bool isTorusY ()    const { return _isTorusY; }

        ///
        /// \brief Returns if the partition is a torus
        /// \return boolean true if the partition is a torus in the Z dimension
        ///         false if the partition is a mesh in the Y dimension
        ///
        bool isTorusZ ()    const { return _isTorusZ; }

        ///
        /// \brief Returns if the partition is a torus
        /// \return boolean true if the partition is a torus
        ///         false if the partition is a mesh
        ///
        bool isTorus ()    const { return _isTorus; }






        ///
        /// \brief Returns maximum number of threads possible in this
        ///        virtual node, including the main thread.
        /// \return Maximum number of threads
        ///
        size_t getMaxThreads() const { return _maxThreads; }
#endif
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
#if 0
        ///
        /// \brief Returns if the partition is in high throughput
        ///        computing mode
        /// \return boolean true if the partition in in HTC mode
        ///
        bool isHTCmode ()    const { return _isHTCmode; }
#endif
#if 0
        ///
        /// \brief Returns if the partition has GI enabled
        /// \return boolean true if the partition has GI enabled
        ///
        bool hasGI ()    const { return _hasGI; }
#endif
#if 0
        ///
        /// \brief Retrives the rank in the pset
        /// \return rank
        ///
        unsigned  rankPset()  const { return _rankpset; }

        ///
        /// \brief Retrives rank of this pset (not node rank in pset)
        /// \return pset rank
        ///
        unsigned  numPset()   const { return _numpset;  }

        ///
        /// \brief Retrives the number of nodes in the pset
        /// \return size of pset
        ///
        unsigned  sizePset()  const { return _sizepset; }
#endif
#if 0
        ///
        /// \brief Retrives p2p tree adress of the IO node
        /// \return address
        ///
        unsigned  ioNodeAddr()const { return _ionodeaddr; }

        ///
        /// \brief Retrives p2p tree address of the node
        /// \return address
        ///
        unsigned  treeAddr()  const { return _treeaddr; }
#endif


      protected:

        size_t _core;
        size_t _hwthread;

        bool   _torusA;
        bool   _torusB;
        bool   _torusC;
        bool   _torusD;
        bool   _torusE;

        size_t _cores;
        size_t _hwthreads;
#if 0
        size_t   _x;
        size_t   _y;
        size_t   _z;
        size_t   _t;
        size_t   _Xnodes;
        size_t   _Ynodes;
        size_t   _Znodes;
        size_t   _Tnodes;
        bool     _isTorusX;
        bool     _isTorusY;
        bool     _isTorusZ;
        bool     _isTorus;

        int      _maxThreads;
        int      _memSize;
        int      _clockMHz;
        int      _rankpset;
        int      _numpset;
        int      _sizepset;
        //int      _ionodeaddr;
        //int      _treeaddr;
        bool     _isHTCmode;
        //bool     _hasGI;
#endif
    };	// class BgqPersonality
};	// namespace XMI

#endif // __xmi_components_sysdep_bgq_bgqpersonnality_h__
