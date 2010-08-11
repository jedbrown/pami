///
/// \file test/internals/bgq/mu2/geom_classroutes.cc
/// \brief ???
///

#include <stdio.h>
#include <pami.h>

//#include "Client.h"

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

#if 1
	configuration.name = PAMI_GEOMETRY_OPTIMIZE;
        status = PAMI_Geometry_query(world_geometry, &configuration, 1);
        if (status != PAMI_SUCCESS) {
                fprintf (stderr, "Error. Unable query to geom configuration (%d). result = %d\n", configuration.name, status);
                return 1;
        }
	printf("World geometry is%s optimized (%zd)\n",
		configuration.value.intval ? "" : " not",
		configuration.value.intval);
#else
	PAMI::BGQGeometry *geom = (PAMI::BGQGeometry *)world_geometry;
	void *val = geom->getKey(PAMI::Geometry::PAMI_GKEY_BGQCOLL_CLASSROUTE);
	printf("World geometry %s classroute (%ld)\n",
		val ? "has" : "does not have",
		(long int)val - 1);
#endif

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
