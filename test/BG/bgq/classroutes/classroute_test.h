
#include <spi/include/kernel/collective.h>
#include "spi/include/mu/Classroute.h"
#undef __INLINE__
#define __INLINE__      static inline
#include "spi/include/mu/Classroute_inlines.h"

	CR_RECT_T refcomm;
	CR_COORD_T me;

	// initialize entire-block rectange:
	*CR_RECT_LL(&refcomm) = (CR_COORD_T){{0,0,0,0,0}};
	*CR_RECT_UR(&refcomm) = (CR_COORD_T){{
		pers->Network_Config.Anodes - 1,
		pers->Network_Config.Bnodes - 1,
		pers->Network_Config.Cnodes - 1,
		pers->Network_Config.Dnodes - 1,
		pers->Network_Config.Enodes - 1
	}};

	// get this node's coordinates:
	me = (CR_COORD_T){{
		pers->Network_Config.Acoord,
		pers->Network_Config.Bcoord,
		pers->Network_Config.Ccoord,
		pers->Network_Config.Dcoord,
		pers->Network_Config.Ecoord
	}};

	CR_RECT_T communiv;
	CR_COORD_T refroot;
	int pri_dim;

	// Pick a reasonable root node for comm-world:
	// TBD: based on booted block or sub-block job?
	// \todo Get dimension-order mapping instead of NULL.
	MUSPI_PickWorldRoot(&refcomm, NULL, &refroot, &pri_dim);

	size_t np = (size_t)-1;
	char *s = getenv("BG_PROCESSESPERNODE");
	if (s) {
		np = stroul(s, NULL, 0);
	}
	int map[CR_NUM_DIMS];
	s = getenv("BG_MAPPING");
	if (s) {
		// if [[ ${s} == +([A-ET]) ]]
		// if (strlen(s) == CR_NUM_DIMS + 1)
		char *x, *m = CR_DIM_NAMES;
		i = 0;
		for (; *s; ++s) {
			// do we have to worry about where T is in map?
			if (*s == 'T') continue;
			x = strchr(m, *s);
			if (!x) break;
			map[i++] = (x - m);
		}
	}
	if (i != CR_NUM_DIMS) {
		// either bogus BG_MAPPING or none at all...
		// Use simple 1:1 mapping
		for (i = 0; i < CR_NUM_DIMS; ++i) map[i] = i;
	}

	BG_SubBlockDefinition_t subblk;
	rc = Kernel_SubBlockDefinition(&subblk);
	if (rc == FAIL) { // no sub-block == use entire block

		// initialize entire-block rectange:
		*CR_RECT_LL(&communiv) = *CR_RECT_LL(&refcomm);
		*CR_RECT_UR(&communiv) = *CR_RECT_UR(&refcomm);

	} else {

		*CR_RECT_LL(&communiv) = (CR_COORD_T){{
			subblk.corner.a,
			subblk.corner.b,
			subblk.corner.c,
			subblk.corner.d,
			subblk.corner.e
		}};
		*CR_RECT_UR(&communiv) = (CR_COORD_T){{
			subblk.corner.a + subblk.shape.a - 1,
			subblk.corner.b + subblk.shape.b - 1,
			subblk.corner.c + subblk.shape.c - 1,
			subblk.corner.d + subblk.shape.d - 1,
			subblk.corner.e + subblk.shape.e - 1
		}};

	}
	CR_RECT_T commworld;
	CR_COORD_T *excluded = NULL;
	int nexcl = 0;
	if (np != (size_t)-1) {
		int z = __MUSPI_rect_size(&communiv);
		excluded = malloc(z * sizeof(CR_COORD_T));
		// assert(excluded != NULL);

		// \todo really discard previous pri_dim here?
		MUSPI_MakeNpRectMap(&communiv, np, map,
				&commworld, excluded, &nexcl, &pri_dim);
	} else {
		commworld = communiv;
	}
	// At this point, 'commworld' should be the circumscribing rectangle for the
	// active nodes and 'excluded' (if not NULL and nexcl > 0) is the array of
	// excluded nodes from 'commworld'.
	// Note: it was agreed that all sub-comms will be full rectangles, and
	// commworld was setup by CNK using -np, so we really don't care about
	// 'excluded' and 'nexcl'.

	// NOTE: commworld already setup by CNK. Do we even care about
	// that rectangle any more?
	// Or do we only care about the booted block and it's root?

	ClassRoute_t cr;
	void *crdata = NULL; // used by routines to keep track of classroute assignments

	CR_RECT_T subcomm;
	for (i = 0; i < nClassRoutes; ++i) {
		if (i == 0) {
			// Build this node's classroute for comm-world:
			subcomm = commworld;
		} else {
			// TBD: what sub-comms to build...
		}
		if (nexcl > 0) {
			MUSPI_BuildNodeClassrouteSparse(&refcomm, &refroot, &me,
							&commworld, excluded, nexcl,
							NULL, pri_dim, &cr);
		} else {
			MUSPI_BuildNodeClassroute(&refcomm, &refroot, &me,
							&commworld,
							NULL, pri_dim, &cr);
		}

		// assume we are contributing data. sparse rectangles might need an if-check here.
		cr.input |= BGQ_CLASS_INPUT_LINK_LOCAL;

		// we use the "sub-comm" VC since there are actually only two.
		cr.input |= BGQ_CLASS_INPUT_VC_SUBCOMM;

		// Use SUBCOMM VC to avoid conflicts with SYSTEM.
		uint32_t mask = MUSPI_GetClassrouteIds(BGQ_CLASS_INPUT_VC_SUBCOMM,
			                                            &commworld, &crdata);
		int id;
		// If other nodes might be creating difference classroutes, need to do a
		// collective operation here to get classroute ID.  But for this test we
		// can just assume all nodes are doing the exact same thing.
		//
		// allreduce(&mask, OP_AND);
		//
		id = ffs(mask);
		if (id == 0) {
			    // fatal error - no classroute ids available
		}
		--id; // ffs() returns bit# + 1
		classRouteIds[i] = id;
		classRoutes[i] = cr;
		(void)MUSPI_SetClassrouteId(classRouteIds[i], BGQ_CLASS_INPUT_VC_SUBCOMM,
			                                            &commworld, &crdata);
	}

	//=======================================================================
 
	// Initialize all of the collective class routes
	for (i=0; i<nClassRoutes; ++i) {
		rc = Kernel_SetCollectiveClassRoute(classRouteIds[i], &classRoutes[i]);
		if (rc) {
			printf("Kernel_SetCollectiveClassRoute for class route %u returned rc = %u\n",i, rc);
			exit(1);
		}
	}
	//=======================================================================

	// Now test all the classroutes... TBD

	//=======================================================================

	//=======================================================================

	// Deallocate all of the collective class routes
	for (i = 0; i < nClassRoutes; ++i) {
		MUSPI_ReleaseClassrouteId(classRouteIds[i], BGQ_CLASS_INPUT_VC_SUBCOMM,
			                                            &commworld, &crdata);
		rc = Kernel_DeallocateCollectiveClassRoute(classRouteIds[i]);
		if (rc) {
			printf("Kernel_DeallocateCollectiveClassRoute for class route %u returned rc = %u\n",i, rc);
			test_exit(1);
		}
	}


