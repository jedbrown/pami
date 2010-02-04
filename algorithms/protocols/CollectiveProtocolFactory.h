/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/CollectiveProtocolFactory.h
 * \brief ???
 */

#ifndef __algorithms_protocols_CollectiveProtocolFactory_h__
#define __algorithms_protocols_CollectiveProtocolFactory_h__

namespace CCMI
{
  namespace Adaptor
  {
    class CollectiveProtocolFactory
    {
    public:
      CollectiveProtocolFactory ()
      {
	_cb_geometry = NULL;
      }

      void setMapIdToGeometry(xmi_mapidtogeometry_fn     cb_geometry) {
	_cb_geometry = cb_geometry;
      }

      xmi_geometry_t getGeometry(unsigned id) {
	CCMI_assert (_cb_geometry != NULL);
	return _cb_geometry (id);
      }

      virtual ~CollectiveProtocolFactory ()
      {
      }

      /// NOTE: This is required to make "C" programs link successfully with virtual destructors
      void operator delete(void * p)
      {
        CCMI_abort();
      }

      /// \brief All protocols determine if a given geometry is adequate
      virtual bool Analyze(XMI_GEOMETRY_CLASS *grequest) = 0;

      virtual Executor::Composite * generate(void                      * request_buf,
					     size_t                      rsize,
					     xmi_context_t               context,
					     xmi_geometry_t              geometry,
					     void                      * cmd) 
	{
	  //The main all protocol generate function
	  CCMI_abort();
	}

    protected:
      xmi_mapidtogeometry_fn     _cb_geometry;
    };
  };
};

#endif
