/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/progressfunc/multiprogressfunc.cc
 * \brief Test the generic progress function feature with multiple functions
 */

#include <stdio.h>
#include "sys/xmi.h"
#include "components/devices/generic/ProgressFunctionMsg.h"
#include "Global.h"

#ifndef NUM_MULTI
#define NUM_MULTI	10
#endif // NUM_MULTI

XMI::Device::ProgressFunctionMdl _model;

char msgbuf[NUM_MULTI][XMI::Device::ProgressFunctionMdl::sizeof_msg];

struct my_work {
	unsigned long long count;
	unsigned long long t0;
	unsigned long v;
	unsigned long u;
};

static int my_func(void *cd) {
	struct my_work *w = (struct my_work *)cd;
	if (w->t0 == 0) {
		w->t0 = __global.time.timebase();
//		fprintf(stderr, "Starting %lu work at tick %llu, waiting until %llu\n",
//			w->u, w->t0, w->t0 + w->count);
		w->v = 0;
	}
	unsigned long long t1 = __global.time.timebase();
	if (t1 - w->t0 >= w->count) {
		fprintf(stderr, "Finished %lu at tick %llu (%ld calls), started at %llu\n", w->u, t1, w->v, w->t0);
		return 0;
	}
	++w->v;
	return 1;
}

void done_cb(xmi_context_t ctx, void *cd, xmi_result_t err) {
	unsigned *done = (unsigned *)cd;
	if (err != XMI_SUCCESS) {
		fprintf(stderr, "Completion callback: done ERROR: %d\n", err);
	} else {
		fprintf(stderr, "Completion callback: done SUCCESS\n");
	}
	++(*done);
}

struct my_work work[NUM_MULTI];

int main(int argc, char **argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("multisync test", &client);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
		return 1;
	}

	{ int _n = 1; status = XMI_Context_createv(client, NULL, 0, &context, &_n); }
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to create xmi context. result = %d\n", status);
		return 1;
	}

	xmi_configuration_t configuration;

	configuration.name = XMI_TASK_ID;
	status = XMI_Configuration_query(context, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t task_id = configuration.value.intval;
	//fprintf(stderr, "My task id = %zd\n", task_id);

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(context, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t num_tasks = configuration.value.intval;
	if (task_id == 0) fprintf(stderr, "Number of tasks = %zd\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------
	unsigned done = 0;
	XMI_ProgressFunc_t pf;
	unsigned long long count;

	if (argc > 1) {
		count = strtoull(argv[1], NULL, 0);
	} else {
		count = 300000ULL;	// the printf takes time, ensure we don't
	}				// finish too soon.

	for (x = 0; x < NUM_MULTI; ++x) {
		work[x].t0 = 0;
		work[x].count = count;
		work[x].u = x + 1;
		pf.context = context;
		pf.request = &msgbuf[x];
		pf.func = my_func;
		pf.clientdata = &work[x];
		pf.cb_done = (xmi_callback_t){ done_cb, &done };
		bool rc = _model.generateMessage(&pf);
		if (!rc) {
			fprintf(stderr, "Failed to generateMessage on progress function\n");
			exit(1);
		}
	}
	while (done < NUM_MULTI) {
		XMI_Context_advance(context, 100);
	}
	fprintf(stderr, "Test completed\n");

// ------------------------------------------------------------------------

	status = XMI_Context_destroy(context);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to destroy xmi context. result = %d\n", status);
		return 1;
	}

	status = XMI_Client_finalize(client);
	if (status != XMI_SUCCESS) {
		fprintf(stderr, "Error. Unable to finalize xmi client. result = %d\n", status);
		return 1;
	}

	return 0;
}
