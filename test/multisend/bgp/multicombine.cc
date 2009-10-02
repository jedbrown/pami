#include <stdio.h>
#include "sys/xmi.h"

// PipeWorkQueue and Topology are designed wrong for platform-independent code...
#include "components/pipeworkqueue/bgp/BgpPipeWorkQueue.h"

#include "components/devices/bgp/collective_network/CNAllreduceMsg.h"
#include "components/devices/bgp/collective_network/CNAllreducePPMsg.h"
#include "components/devices/bgp/collective_network/CNAllreduceSum2PMsg.h"


#ifndef TEST_BUF_SIZE
#define TEST_BUF_SIZE	1024
#endif // TEST_BUF_SIZE

void fail_reg(const char *s) {
	fprintf(stderr, "failed to register \"%s\"\n", s);
	exit(1);
}

char cna[sizeof(XMI::Device::BGP::CNAllreduceModel)];
char cnam[sizeof(XMI::Device::BGP::CNAllreduceMessage)];
char cnpp[sizeof(XMI::Device::BGP::CNAllreducePPModel)];
char cnppm[sizeof(XMI::Device::BGP::CNAllreducePPMessage)];
char cns2p[sizeof(XMI::Device::BGP::CNAllreduce2PModel)];
char cns2pm[sizeof(XMI::Device::BGP::CNAllreduce2PMessage)];

char source[TEST_BUF_SIZE];
char result[TEST_BUF_SIZE];

XMI_PIPEWORKQUEUE_CLASS ipwq;
XMI_PIPEWORKQUEUE_CLASS opwq;

int done = 0;

void done_cb(xmi_context_t context, void *cookie, xmi_result_t result) {
	fprintf(stderr, "... completion callback for %s, done %d ++\n", (char *)cookie, done);
	++done;
}

int main(int argc, char ** argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("multicombine test", &client);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable to initialize xmi client. result = %d\n", status);
		return 1;
	}

	status = XMI_Context_create(client, NULL, 0, &context);
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
	fprintf(stderr, "My task id = %zd\n", task_id);

	configuration.name = XMI_NUM_TASKS;
	status = XMI_Configuration_query(context, &configuration);
	if (status != XMI_SUCCESS) {
		fprintf (stderr, "Error. Unable query configuration (%d). result = %d\n", configuration.name, status);
		return 1;
	}
	size_t num_tasks = configuration.value.intval;
	fprintf(stderr, "Number of tasks = %zd\n", num_tasks);

// END standard setup
// ------------------------------------------------------------------------

	// Register some multicombines, C++ style

	const char *test = "CNAllreduceModel";
	XMI::Device::BGP::CNAllreduceModel *cnamodel = new (&cna) XMI::Device::BGP::CNAllreduceModel(status);
	if (status != XMI_SUCCESS) fail_reg(test);

	ipwq.configure(NULL, source, sizeof(source), sizeof(source));
	ipwq.reset();
	opwq.configure(NULL, result, sizeof(result), 0);
	opwq.reset();
	xmi_multicombine_t mcomb;

	// simple allreduce on the tree... SMP mode (todo: check and error)
	mcomb.request = &cnam;
	mcomb.cb_done = (xmi_callback_t){done_cb, (void *)test};
	mcomb.roles = (unsigned)-1;
	mcomb.data = (xmi_pipeworkqueue_t *)&ipwq;
	mcomb.data_participants = NULL;
	mcomb.results = (xmi_pipeworkqueue_t *)&opwq;
	mcomb.results_participants = NULL;
	mcomb.optor = XMI_SUM;
	mcomb.dtype = XMI_UNSIGNED_INT;
	mcomb.count = TEST_BUF_SIZE / sizeof(unsigned);
	for (x = 0; x < mcomb.count; ++x) {
		((unsigned *)source)[x] = 1;
		((unsigned *)result)[x] = -1;
	}
	done = 0;
	fprintf(stderr, "... before %s.postMulticombine\n", test);
	bool rc = cnamodel->postMulticombine(&mcomb);
	if (!rc) {
		fprintf(stderr, "Failed to post multicombine \"%s\"\n", test);
		// need to skip rest...
	}

	fprintf(stderr, "... before advance loop for %s.postMulticombine\n", test);
	while (!done) {
		status = XMI_Context_advance(context, 100);
		if (status != XMI_SUCCESS) {
			fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", status);
			return 1;
		}
	}
	for (x = 0; x < mcomb.count; ++x) {
		if (((unsigned *)source)[x] != 1) {
			fprintf(stderr, "Corrupted source buffer at index %d. stop.\n", x);
			break;
		}
		if (((unsigned *)result)[x] != num_tasks) {
			fprintf(stderr, "Incorrect result at index %d. stop.\n", x);
			break;
		}
	}
	if (x < mcomb.count) {
		// failed
	} else {
		fprintf(stderr, "PASS\n");
	}

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
