#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"

extern char *dim_names;

typedef struct {
	coord_t root;
	classroute_t *cr;
} clsrt_entry_t;

void recurse_dims(rect_t *world, coord_t *root, rect_t *comm, coord_t *me, classroute_t *cr) {
	int x;
	for (x = comm->ll.coords[me->dims]; x <= comm->ur.coords[me->dims]; ++x) {
		me->coords[me->dims] = x;
		++me->dims;
		if (me->dims == world->ll.dims) {
			int r = coord2rank(comm, me);
			build_node_classroute(world, root, me, comm, &cr[r]);
		} else {
			recurse_dims(world, root, comm, me, cr);
		}
		--me->dims;
	}
}

void make_classroutes(rect_t *world, coord_t *root, rect_t *comm, classroute_t *cr) {
	coord_t me = { 0 };
	recurse_dims(world, root, comm, &me, cr);
}

void print_classroutes(rect_t *world, coord_t *root, rect_t *comm, classroute_t *cr) {
	static char buf[1024];
	int z = rect_size(comm);
	char *s = buf;
	int r;

	s += sprintf(s, "Classroute for comm ");
	s += sprint_rect(s, comm);
	s += sprintf(s, " in world ");
	s += sprint_rect(s, world);
	s += sprintf(s, " with root ");
	s += sprint_coord(s, root);
	printf("%s:\n", buf);
	for (r = 0; r < z; ++r) {
		coord_t c;
		rank2coord(comm, r, &c);
		print_classroute(&c, &cr[r]);
	}
}

int main(int argc, char **argv) {
	int x, e;
	rect_t world;
	rect_t comm;
	coord_t me, root;
	char *ep;
	int world_set = 0, comm_set = 0, root_set = 0;
	int chk = 0;

	extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "c:kr:w:")) != EOF) {
		switch(x) {
		case 'c':
			e = parse_rect(optarg, &ep, &comm);
			if (e != 0) {
				fprintf(stderr, "invalid rect for sub-comm: \"%s\" at \"%s\"\n", optarg, ep);
				exit(1);
			}
			++comm_set;
			break;
		case 'k':
			++chk;
			break;
		case 'r':
			dim_names = optarg;
			break;
		case 'w':
			e = parse_rect(optarg, &ep, &world);
			if (e != 0) {
				fprintf(stderr, "invalid rect for world-comm: \"%s\" at \"%s\"\n", optarg, ep);
				exit(1);
			}
			++world_set;
			break;
		}
	}
	if (!world_set) {
		fprintf(stderr, "Usage: %s [-r <dim-names>] -w <world-rectangle> [ -c <sub-comm-rect> ] [<world-root>...]\n"
				"Computes and prints classroute for all nodes in <sub-comm>.\n"
				"Where rectangle syntax is \"(a0,b0,c0,...):(aN,bN,cN,...)\"\n"
				"and 'a0' (etc) are lower-left corner coords\n"
				"and 'aN' (etc) are upper-right corner coords\n"
				"<world-root> is coordinates of root to use in world rect (default: center)\n"
				"-r renames dimensions, default is \"ABCDEFGH\"\n"
				"All coordinates must use the same number of dimensions.\n"
				"sub-comm defaults to world\n", argv[0]);
		exit(1);
	}
	if (!comm_set) {
		comm = world;
	}
	if (world.ll.dims != comm.ll.dims) {
		fprintf(stderr, "sub-comm number of dimensions does not match 'world'\n");
		exit(1);
	}
	int z = rect_size(&comm);
	if (optind < argc) {

		clsrt_entry_t *cra = (clsrt_entry_t *)malloc((argc - optind) * sizeof(clsrt_entry_t));
		if (!cra) {
			fprintf(stderr, "out of memory allocating classroute array!\n");
			exit(1);
		}

		for (x = 0; x < argc - optind; ++x) {
			e = parse_coord(argv[optind + x], &ep, &root);
			if (e != 0) {
				fprintf(stderr, "invalid coord for world root: \"%s\" at \"%s\"\n", argv[optind + x], ep);
				continue;
			}
			if (root.dims != world.ll.dims) {
				fprintf(stderr, "root number of dimensions does not match 'world'\n");
				continue;
			}

			classroute_t *cr = (classroute_t *)malloc(z * sizeof(classroute_t));
			if (!cr) {
				fprintf(stderr, "out of memory allocating classroute array!\n");
				exit(1);
			}
			memset(cr, -1, z * sizeof(classroute_t));
			make_classroutes(&world, &root, &comm, cr);

			if (!chk) {
				if (x) printf("\n");
				print_classroutes(&world, &root, &comm, cr);
				free(cr);
			} else {
				cra[x].cr = cr;
				cra[x].root = root;
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
				print_classroutes(&world, &cra[x].root, &comm, cra[x].cr);
				free(cra[x].cr);
			}
		}
	} else {
		for (x = 0; x < world.ll.dims; ++x) {
			root.coords[x] = world.ll.coords[x] +
				(world.ur.coords[x] - world.ll.coords[x] + 1) / 2;

		}
		root.dims = world.ll.dims;

		classroute_t *cr = (classroute_t *)malloc(z * sizeof(classroute_t));
		if (!cr) {
			fprintf(stderr, "out of memory allocating classroute array!\n");
			exit(1);
		}
		memset(cr, -1, z * sizeof(classroute_t));
		make_classroutes(&world, &root, &comm, cr);
		print_classroutes(&world, &root, &comm, cr);
		free(cr);
	}
	exit(0);
}
