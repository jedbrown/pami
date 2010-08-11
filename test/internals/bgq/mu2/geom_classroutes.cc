///
/// \file test/internals/bgq/mu2/geom_classroutes.cc
/// \brief ???
///

#include <stdio.h>
#include <pami.h>

#include "Global.h"

static void g_done_fn(pami_context_t ctx, void *cookie, pami_result_t err) {
	int *done = (int *)cookie;
	if (err) {
		*done = -err;
	} else {
		*done = 1;
	}
}

int main(int argc, char ** argv) {
        pami_context_t context;
        size_t task_id;
        size_t num_tasks;

        pami_client_t client;
        pami_result_t status = PAMI_ERROR;
        status = PAMI_Client_create("multisync test", &client, NULL, 0);
        if (status != PAMI_SUCCESS) {
                fprintf (stderr, "Error. Unable to initialize pami client. result = %d\n", status);
                return 1;
        }

        status = PAMI_Context_createv(client, NULL, 0, &context, 1);
        if (status != PAMI_SUCCESS) {
                fprintf (stderr, "Error. Unable to create pami context. result = %d\n", status);
                return 1;
        }

        pami_configuration_t configuration;

        configuration.name = PAMI_CLIENT_TASK_ID;
        status = PAMI_Client_query(client, &configuration, 1);
        if (status != PAMI_SUCCESS) {
                fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
                return 1;
        }
        task_id = configuration.value.intval;
        //fprintf(stderr, "My task id = %zu\n", task_id);

        configuration.name = PAMI_CLIENT_NUM_TASKS;
        status = PAMI_Client_query(client, &configuration, 1);
        if (status != PAMI_SUCCESS) {
                fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
                return 1;
        }
        num_tasks = configuration.value.intval;
        if (task_id == 0) fprintf(stderr, "Number of tasks = %zu\n", num_tasks);


// END standard setup
// ------------------------------------------------------------------------

	pami_geometry_t world_geometry;
	status = PAMI_Geometry_world(client, &world_geometry);

	configuration.name = PAMI_GEOMETRY_OPTIMIZE;
        status = PAMI_Geometry_query(world_geometry, &configuration, 1);
        if (status != PAMI_SUCCESS) {
                fprintf (stderr, "Error. Unable query to geom configuration (%d). result = %d\n", configuration.name, status);
                return 1;
        }
	printf("World geometry is%s optimized (%zd)\n",
		configuration.value.intval ? "" : " not",
		configuration.value.intval);

	pami_geometry_range_t range;
	range.lo = 0;
	range.hi = __global.topology_global.size();
	unsigned id = 300;

	if (range.lo < range.hi) {
		pami_geometry_t geom = NULL;
		configuration.value.intval = 1;
		int g_done = 0;
#if 0
		status = PAMI_Geometry_create_taskrange(client, &configuration, 1,
				__global.mapping.task() > range.hi ? NULL : &geom,
				world_geometry, id, &range, 1, context, g_done_fn, &g_done);
#else
		// hack to avoid barrier callbacks with NULL contexts...
		status = PAMI_Geometry_create_taskrange(client, NULL, 0,
				__global.mapping.task() > range.hi ? NULL : &geom,
				world_geometry, id, &range, 1, context, g_done_fn, &g_done);
        	if (status == PAMI_SUCCESS) {
			while (!g_done) {
				PAMI_Context_advance(context, 100);
			}
			if (g_done == 1) {
				g_done = 0;
fprintf(stderr, "calling PAMI_Geometry_update...\n");
				status = PAMI_Geometry_update(geom, &configuration, 1,
							context, g_done_fn, &g_done);
			}
       		}
#endif
        	if (status != PAMI_SUCCESS) {
                	fprintf (stderr, "Error. Failed to create geom %d. result = %d\n", id, status);
       		} else {
			while (!g_done) {
				PAMI_Context_advance(context, 100);
			}
			if (g_done < 0) {
				fprintf(stderr, "Failed to create geometry %u [%zd..%zd] %d\n",
							id, range.lo, range.hi, -g_done);
			} else {
        			status = PAMI_Geometry_query(geom, &configuration, 1);
        			if (status != PAMI_SUCCESS) {
                			fprintf (stderr, "Error. Unable query to geom %d configuration (%d). result = %d\n", id, configuration.name, status);
        			} else {
					printf("Geometry %d is%s optimized (%zd)\n", id,
						configuration.value.intval ? "" : " not",
						configuration.value.intval);
				}
			}
		}
		range.hi /= 2;
		++id;
	}

// ------------------------------------------------------------------------
        status = PAMI_Context_destroyv(&context, 1);
        if (status != PAMI_SUCCESS) {
                fprintf(stderr, "Error. Unable to finalize pami contexts. result = %d\n", status);
                return 1;
        }
        status = PAMI_Client_destroy(&client);
        if (status != PAMI_SUCCESS) {
                fprintf(stderr, "Error. Unable to finalize pami client. result = %d\n", status);
                return 1;
        }

        return 0;
}
