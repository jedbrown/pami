#include <stdio.h>
#include "sys/xmi.h"

#include "Topology.h"
#include "components/devices/generic/AtomicBarrierMsg.h"

#include "Global.h"

void fail_reg(const char *s) {
	fprintf(stderr, "failed to register \"%s\"\n", s);
	exit(1);
}

#ifdef __bgp__

#warning using BGP lockbox barrier to avoid bug in CounterBarrier/GccCounter
#define BARRIER_NAME	"XMI::Barrier::BGP::LockBoxNodeProcBarrier"
#include "components/atomic/bgp/LockBoxBarrier.h"
typedef XMI::Barrier::BGP::LockBoxNodeProcBarrier<XMI_SYSDEP_CLASS> Barrier_Type;

#else

#warning CounterBarrier/GccCounter is broken - it does not barrier
#define BARRIER_NAME	"XMI::Barrier::CounterBarrier<XMI::Counter::GccNodeCounter>"
#include "components/atomic/gcc/GccCounter.h"
#include "components/atomic/counter/CounterBarrier.h"
typedef XMI::Barrier::CounterBarrier<XMI_SYSDEP_CLASS,XMI::Counter::GccNodeCounter<XMI_SYSDEP_CLASS> > Barrier_Type;

#endif

typedef XMI::Device::AtomicBarrierMdl<Barrier_Type> Barrier_Model;

char ab[sizeof(Barrier_Model)];
char abm[Barrier_Model::sizeof_msg];

XMI_TOPOLOGY_CLASS topo;

int done = 0;

void done_cb(xmi_context_t context, void *cookie, xmi_result_t result) {
	// printf skews timing too much...
	//fprintf(stderr, "... completion callback for %s, done %d ++\n", (char *)cookie, done);
	++done;
}

int main(int argc, char ** argv) {
	unsigned x;
	xmi_client_t client;
	xmi_context_t context;
	xmi_result_t status = XMI_ERROR;

	status = XMI_Client_initialize("multisync test", &client);
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

#warning ensure only local ranks

// END standard setup
// ------------------------------------------------------------------------

	// Do two barriers: first is to synchronize the ranks so that
	// the timing of the second barrier is cleaner. Also, no printfs
	// in the barriers because that messes up timing.  So, if the barrier
	// needs debug then the printfs will have to be re-enabled.

	// Basic timing profile: Each rank will delay prior to the start
	// by the rank multiplied by 1000 (cycles). The idea being, if the
	// barrier actually does it's job then the total time spent by each
	// rank should be (relatively) close to the others. NOTE: these times
	// are not to be used as performance numbers! They intentionally introduce
	// extra time in order to show that the barrier is functional.

	// Register some multisyncs, C++ style
	unsigned long long t0, t1, t2;
	bool rc;

	const char *test = BARRIER_NAME;
	Barrier_Model *model = new (&ab) Barrier_Model(status);
	if (status != XMI_SUCCESS) fail_reg(test);

	new (&topo) XMI_TOPOLOGY_CLASS(task_id);

	xmi_multisync_t msync;

	// simple allreduce on the tree... SMP mode (todo: check and error)
	msync.request = &abm;
	msync.cb_done = (xmi_callback_t){done_cb, (void *)test};
	msync.roles = (unsigned)-1;
	msync.participants = (xmi_topology_t *)&topo;
	fprintf(stderr, "... before %s.postMultisync\n", test);
	done = 0;
	rc = model->postMultisync(&msync);
	if (!rc) {
		fprintf(stderr, "Failed to post first multisync \"%s\"\n", test);
		// need to skip rest...
	}
	// printf skews timing too much...
	//fprintf(stderr, "... before advance loop for %s.postMultisync\n", test);
	while (!done) {
		status = XMI_Context_advance(context, 100);
		if (status != XMI_SUCCESS) {
			fprintf (stderr, "Error. Unable to advance xmi context. result = %d\n", status);
			return 1;
		}
	}
	t0 = __global.time.timebase();
	while ((t1 = __global.time.timebase()) - t0 < task_id * 1000);
	done = 0;
	rc = model->postMultisync(&msync);
	if (!rc) {
		fprintf(stderr, "Failed to post second multisync \"%s\"\n", test);
		// need to skip rest...
	}

	// printf skews timing too much...
	//fprintf(stderr, "... before advance loop for %s.postMultisync\n", test);
	while (!done) {
		status = XMI_Context_advance(context, 100);
		// assert(status == XMI_SUCCESS);
	}
	t2 = __global.time.timebase();
	// first number is total time for test barrier, second number is time for *this*
	// rank's barrier. The second numbers should show a significant variation
	// between ranks, while the first number should be more uniform.
	fprintf(stderr, "PASS? %5lld (%5lld)\n", t2 - t0, t2 - t1);

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
