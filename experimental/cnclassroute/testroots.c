/**
 * \file experimental/cnclassroute/testroots.c
 * \brief ???
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"

extern char *dim_names;

typedef struct {
	commworld_t world;
	classroute_t *cr;
} clsrt_entry_t;

#define DEBUG

int main(int argc, char **argv) {
	int x, e;
	rect_t comm;
	commworld_t world;
	char *ep;
	int world_set = 0, comm_set = 0, root_set = 0;
	int chk = 0;
	int pick = 0;
	int sanity = 0;
	int force_errs = 0;
	uint32_t rand_mask = 0;

	extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "c:eEkm:pr:sw:")) != EOF) {
		switch(x) {
		case 'c':
			e = parse_rect(optarg, &ep, &comm);
			if (e != 0) {
				fprintf(stderr, "invalid rect for sub-comm: \"%s\" at \"%s\"\n", optarg, ep);
				exit(1);
			}
			++comm_set;
			break;
#ifdef DEBUG
		case 'e':
			srand(time(NULL));
			/* FALLTHROUGH */
		case 'E':
			++force_errs;
			break;
		case 'm':
			rand_mask = strtoul(optarg, NULL, 0);
			break;
#endif /* DEBUG */
		case 'k':
			++chk;
			break;
		case 'p':
			++pick;
			break;
		case 'r':
			dim_names = optarg;
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
"Usage: %s <options> -w <world-rectangle> [<world-root>...]\n"
"Computes and prints classroute for all nodes in <sub-comm>.\n"
"Where <optionas> are:\n"
"\t-c <sub-comm-rect> rectangle to use for classroute(s) (default: world-rectangle)\n"
"\t-k checks each root's classroute for use of same links\n"
"\t-p pick two non-conflicting roots (system/user commworld)\n"
"\t-r <dim-names> renames dimensions, default is \"ABCDEFGH\"\n"
"\t-s check classroute sanity\n"
"coordinates (<world-root>) syntax is \"(a,b,c,...)\"\n"
"rectangle syntax is \"<ll-coords>:<ur-coords>\"\n"
"\t<ll-coords> are lower-left corner coordinates\n"
"\t<ur-coords> are upper-right corner coordinates\n"
"<world-root> is coordinates of root to use in world rect (default: center)\n"
"All coordinates must use the same number of dimensions.\n"
"<sub-comm-rect> defaults to <world-rectangle>\n"
"Example: %s -k -w \"(0,0):(3,3)\" \"(0,2)\" \"(3,2)\"\n"
			, argv[0], argv[0]);
		exit(1);
	}
	if (!comm_set) {
		comm = world.rect;
	}
	if (world.rect.ll.dims != comm.ll.dims) {
		fprintf(stderr, "sub-comm number of dimensions does not match 'world'\n");
		exit(1);
	}
	int z = rect_size(&comm);
	if (pick) {
		clsrt_entry_t *cra = (clsrt_entry_t *)malloc(2 * sizeof(clsrt_entry_t));
		if (!cra) {
			fprintf(stderr, "out of memory allocating classroute array!\n");
			exit(1);
		}
		int min = 99999999;
		int min_dim = -1;
		for (x = 0; x < world.rect.ll.dims; ++x) {
			int size = world.rect.ur.coords[x] - world.rect.ll.coords[x] + 1;
			world.root.coords[x] = world.rect.ll.coords[x] + size / 2;
			if (size > 1 && size <= min) {
				min = size;
				min_dim = x;
			}
		}
		world.root.dims = world.rect.ll.dims;
		world.pri_dim = min_dim;

		world.root.coords[min_dim] = world.rect.ll.coords[min_dim];
		classroute_t *cr = (classroute_t *)malloc(z * sizeof(classroute_t));
		if (!cr) {
			fprintf(stderr, "out of memory allocating classroute array!\n");
			exit(1);
		}
		memset(cr, -1, z * sizeof(classroute_t));
		make_classroutes(&world, &comm, cr);
		if (sanity) chk_all_sanity(&world, &comm, cr);
		if (!chk) {
			print_classroutes(&world, &comm, cr, sanity);
		} else {
			cra[0].cr = cr;
			cra[0].world = world;
			cr = (classroute_t *)malloc(z * sizeof(classroute_t));
			if (!cr) {
				fprintf(stderr, "out of memory allocating classroute array!\n");
				exit(1);
			}
		}

		world.root.coords[min_dim] = world.rect.ur.coords[min_dim];
		memset(cr, -1, z * sizeof(classroute_t));
		make_classroutes(&world, &comm, cr);
		if (sanity) chk_all_sanity(&world, &comm, cr);
		if (!chk) {
			printf("\n");
			print_classroutes(&world, &comm, cr, sanity);
			free(cr);
		} else {
			cra[1].cr = cr;
			cra[1].world = world;
		}
		if (chk) {
			/* flag conflicting classroutes with MSB set to "1" */
			int r, y;
			for (r = 0; r < z; ++r) {
				chk_classroute(&cra[0].cr[r], &cra[1].cr[r]);
			}
			/* now print classroutes, with errors flagged */
			print_classroutes(&cra[0].world, &comm, cra[0].cr, 1);
			free(cra[0].cr);
			printf("\n");
			print_classroutes(&cra[1].world, &comm, cra[1].cr, 1);
			free(cra[1].cr);
		}

	} else if (optind < argc) {

		clsrt_entry_t *cra = (clsrt_entry_t *)malloc((argc - optind) * sizeof(clsrt_entry_t));
		if (!cra) {
			fprintf(stderr, "out of memory allocating classroute array!\n");
			exit(1);
		}

		for (x = 0; x < argc - optind; ++x) {
			e = parse_coord(argv[optind + x], &ep, &world.root);
			if (e != 0) {
				fprintf(stderr, "invalid coord for world root: \"%s\" at \"%s\"\n", argv[optind + x], ep);
				continue;
			}
			if (world.root.dims != world.rect.ll.dims) {
				fprintf(stderr, "root number of dimensions does not match 'world'\n");
				continue;
			}

			classroute_t *cr = (classroute_t *)malloc(z * sizeof(classroute_t));
			if (!cr) {
				fprintf(stderr, "out of memory allocating classroute array!\n");
				exit(1);
			}
			memset(cr, -1, z * sizeof(classroute_t));
			make_classroutes(&world, &comm, cr);
			if (sanity) chk_all_sanity(&world, &comm, cr);

			if (!chk) {
				if (x) printf("\n");
				print_classroutes(&world, &comm, cr, sanity);
				free(cr);
			} else {
				cra[x].cr = cr;
				cra[x].world = world;
			}
		}
		if (chk) {
			/* flag conflicting classroutes with MSB set to "1" */
			int r, y;
			for (r = 0; r < z; ++r) {
				for (x = 0; x < argc - optind; ++x) {
					for (y = x + 1; y < argc - optind; ++y) {
						chk_classroute(&cra[x].cr[r],
								&cra[y].cr[r]);
					}
				}
			}
			/* now print classroutes, with errors flagged */
			for (x = 0; x < argc - optind; ++x) {
				if (x) printf("\n");
				print_classroutes(&cra[x].world, &comm, cra[x].cr, 1);
				free(cra[x].cr);
			}
		}
	} else {
		for (x = 0; x < world.rect.ll.dims; ++x) {
			world.root.coords[x] = world.rect.ll.coords[x] +
				(world.rect.ur.coords[x] - world.rect.ll.coords[x] + 1) / 2;

		}
		world.root.dims = world.rect.ll.dims;

		classroute_t *cr = (classroute_t *)malloc(z * sizeof(classroute_t));
		if (!cr) {
			fprintf(stderr, "out of memory allocating classroute array!\n");
			exit(1);
		}
		memset(cr, -1, z * sizeof(classroute_t));
		make_classroutes(&world, &comm, cr);
#ifdef DEBUG
		if (force_errs) {
			int r, l;
			for (r = 0; r < z; ++r) {
				if ((rand() & rand_mask) == 0) {
					l = (rand() & rand() & rand()); /* try to reduce bits */
					l &= ((1 << (world.rect.ll.dims * 2)) - 1);
					cr[r].up_tree = l;
				}
				if ((rand() & rand_mask) == 0) {
					l = (rand() & rand()); /* try to reduce bits */
					l &= ((1 << (world.rect.ll.dims * 2)) - 1);
					cr[r].dn_tree = l;
				}
			}
		}
#endif /* DEBUG */
		if (sanity) chk_all_sanity(&world, &comm, cr);
		print_classroutes(&world, &comm, cr, sanity);
		free(cr);
	}
	exit(0);
}
