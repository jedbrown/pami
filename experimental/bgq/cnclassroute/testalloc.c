/**
 * \file experimental/bgq/cnclassroute/testalloc.c
 * \brief Test allocation of classroute IDs
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"

int main(int argc, char **argv) {
	int x, y, z, e;
	commworld_t world;
	CR_RECT_T *comm;
	char *ep;
	int world_set = 0;
	int sanity = 0;

	extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "r:sw:")) != EOF) {
		switch(x) {
		case 's':
			++sanity;
			break;
		case 'w':
			e = parse_rect(optarg, &ep, &world.rect);
			if (e != 0) {
				fprintf(stderr, "invalid rect for world-comm: \"%s\" at \"%s\"\n", optarg, ep);
				exit(1);
			}
			world.pri_dim = 0;
			++world_set;
			break;
		}
	}
	if (!world_set) {
		fprintf(stderr,
"Usage: %s <options> -w <world-rectangle> [<sub-comm-rect>...]\n"
"Computes and prints classroute for all nodes in each <sub-comm>.\n"
"Where <optionas> are:\n"
"\t-s check classroute for samity\n"
"<sub-comm-rect> rectangle to use for classroute(s) (default: world-rectangle)\n"
"coordinates (<world-root>) syntax is \"(a,b,c,...)\"\n"
"rectangle syntax is \"<ll-coords>:<ur-coords>\"\n"
"\t<ll-coords> are lower-left corner coordinates\n"
"\t<ur-coords> are upper-right corner coordinates\n"
"All coordinates must use the same number of dimensions.\n"
"Example: %s -w \"(0,0):(3,3)\" \"(0,0):(2,2)\" \"(1,1):(2,2)\" \"(1,1):(3,3)\"\n"
			, argv[0], argv[0]);
		exit(1);
	}
	static CR_RECT_T *cr_alloc[BGQ_COLL_CLASS_MAX_CLASSROUTES][BGQ_COLL_CLASS_MAX_CLASSROUTES];
	memset(cr_alloc, 0, sizeof(cr_alloc));
	uint32_t m;
	z = rect_size(&world.rect);
	void **env = malloc(z * sizeof(void *));

	x = alloc_classroutes(&world, &world.rect, BGQ_COLL_CLASS_INPUT_VC_SYSTEM, env);
	if (x == -1) {
		fprintf(stderr, "No space for system comm-world classroute!\n");
		exit(1);
	}
	for (z = 0; z < BGQ_COLL_CLASS_MAX_CLASSROUTES && cr_alloc[x][z] != NULL; ++z);
	cr_alloc[x][z] = &world.rect;

	x = alloc_classroutes(&world, &world.rect, BGQ_COLL_CLASS_INPUT_VC_USER, env);
	if (x == -1) {
		fprintf(stderr, "No space for user comm-world classroute!\n");
		exit(1);
	}
	for (z = 0; z < BGQ_COLL_CLASS_MAX_CLASSROUTES && cr_alloc[x][z] != NULL; ++z);
	cr_alloc[x][z] = &world.rect;

	if (optind < argc) {
		for (y = 0; y < argc - optind; ++y) {
			comm = malloc(sizeof(*comm));
			e = parse_rect(argv[optind + y], &ep, comm);
			if (e != 0) {
				fprintf(stderr, "invalid rect for sub-comm: \"%s\" at \"%s\"\n", argv[optind + y], ep);
				exit(1);
			}

			x = alloc_classroutes(&world, comm, BGQ_COLL_CLASS_INPUT_VC_SUBCOMM, env);
			if (x == -1) {
				fprintf(stderr, "No space for sub-comm classroute!\n");
				continue;
			}
			for (z = 0; z < BGQ_COLL_CLASS_MAX_CLASSROUTES && cr_alloc[x][z] != NULL; ++z);
			cr_alloc[x][z] = comm;
		}
	} else {
		comm = malloc(sizeof(*comm));
		*comm = world.rect;
		x = alloc_classroutes(&world, comm, BGQ_COLL_CLASS_INPUT_VC_SUBCOMM, env);
		if (x == -1) {
			fprintf(stderr, "No space for sub-comm classroute!\n");
		} else {
			--x;
			for (z = 0; z < BGQ_COLL_CLASS_MAX_CLASSROUTES && cr_alloc[x][z] != NULL; ++z);
			cr_alloc[x][z] = comm;
		}
	}
	for (x = 0; x < BGQ_COLL_CLASS_MAX_CLASSROUTES; ++x) {
		static char buf[1024];
		char *s = buf;
		comm = cr_alloc[x][0];
		if (comm) {
			for (z = 0; z < BGQ_COLL_CLASS_MAX_CLASSROUTES && cr_alloc[x][z] != NULL; ++z) {
				*s++ = ' ';
				s += sprint_rect(s, cr_alloc[x][z]);
			}
			printf("[%d]%s\n", x, buf);
		} else {
			printf("[%d] EMPTY\n", x);
		}
	}
	exit(0);
}
