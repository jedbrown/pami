/**
 * \file experimental/cnclassroute/testcomms.c
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"

extern char *dim_names;

int main(int argc, char **argv) {
	int x, z, e;
	commworld_t world;
	rect_t comm;
	char *ep;
	int world_set = 0, comm_set = 0, root_set = 0;
	int sanity = 0;

	extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "d:r:sw:")) != EOF) {
		switch(x) {
		case 'd':
			dim_names = optarg;
			break;
		case 'r':
			e = parse_coord(optarg, &ep, &world.root);
			if (e != 0) {
				fprintf(stderr, "invalid coord for world root: \"%s\" at \"%s\"\n", optarg, ep);
				continue;
			}
			++root_set;
			break;
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
"\t-d <dim-names> renames dimensions, default is \"ABCDEFGH\"\n"
"\t-r <world-root> is coordinates of root to use in world rect (default: center)\n"
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
	if (!root_set) {
		for (x = 0; x < world.rect.ll.dims; ++x) {
			world.root.coords[x] = world.rect.ll.coords[x] +
				(world.rect.ur.coords[x] - world.rect.ll.coords[x] + 1) / 2;

		}
		world.root.dims = world.rect.ll.dims;
	}
	if (world.root.dims != world.rect.ll.dims) {
		fprintf(stderr, "root number of dimensions does not match 'world'\n");
		exit(1);
	}
	if (optind < argc) {
		for (x = 0; x < argc - optind; ++x) {
			e = parse_rect(argv[optind + x], &ep, &comm);
			if (e != 0) {
				fprintf(stderr, "invalid rect for sub-comm: \"%s\" at \"%s\"\n", argv[optind + x], ep);
				exit(1);
			}
			if (world.rect.ll.dims != comm.ll.dims) {
				fprintf(stderr, "sub-comm number of dimensions does not match 'world'\n");
				exit(1);
			}

			z = rect_size(&comm);
			ClassRoute_t *cr = (ClassRoute_t *)malloc(z * sizeof(ClassRoute_t));
			if (!cr) {
				fprintf(stderr, "out of memory allocating classroute array!\n");
				exit(1);
			}
			memset(cr, -1, z * sizeof(ClassRoute_t));
			make_classroutes(&world, &comm, cr);
			if (sanity) chk_all_sanity(&world, &comm, cr);

			if (x) printf("\n");
			print_classroutes(&world, &comm, cr, sanity);
			free(cr);
		}
	} else {
		comm = world.rect;
		z = rect_size(&comm);
		ClassRoute_t *cr = (ClassRoute_t *)malloc(z * sizeof(ClassRoute_t));
		if (!cr) {
			fprintf(stderr, "out of memory allocating classroute array!\n");
			exit(1);
		}
		memset(cr, -1, z * sizeof(ClassRoute_t));
		make_classroutes(&world, &comm, cr);
		if (sanity) chk_all_sanity(&world, &comm, cr);
		print_classroutes(&world, &comm, cr, sanity);
		free(cr);
	}
	exit(0);
}
