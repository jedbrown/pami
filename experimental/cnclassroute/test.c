#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cnclassroute.h"

extern void build_node_classroute(rect_t *world, coord_t *worldroot, coord_t *me,
						rect_t *comm, classroute_t *cr);

char *dim_names = XMI_DIM_NAMES;

int sprint_links(char *buf, uint32_t link) {
	char *s = buf;
	int x;
	for (x = 0; x < XMI_MAX_DIMS; ++x) {
		if (link & CR_LINK(x,CR_SIGN_POS)) {
			if (s != buf) *s++ = ' ';
			*s++ = dim_names[x];
			*s++ = '+';
		}
		if (link & CR_LINK(x,CR_SIGN_NEG)) {
			if (s != buf) *s++ = ' ';
			*s++ = dim_names[x];
			*s++ = '-';
		}
	}
	*s = '\0';
	return s - buf;
}

int parse_coord(char *s, char **e, coord_t *c) {
	coord_t c0;
	char *t;

	/* syntax: "( %d, %d, ... )" */
	while (*s && *s != '(') ++s;
	if (*s == '(') ++s;
	c0.dims = 0;
	while (1) {
		while (*s && isspace(*s)) ++s;
		if (!*s) {
			if (e) *e = s;
			return -1;
		}
		if (*s == ')') {
			if (e) *e = ++s;
			*c = c0;
			return 0;
		}
		c0.coords[c0.dims] = strtol(s, &t, 0);
		if (s == t) {
			if (e) *e = s;
			return -1;
		}
		s = t;
		if (*s == ',') ++s;
		++c0.dims;
	}
}

int parse_rect(char *s, char **e, rect_t *r) {
	int x;
	rect_t r0;
	char *t;

	/* syntax: "(%d,%d,...) : (%d,%d,...)" */
	x = parse_coord(s, e, &r0.ll);
	if (x != 0) return x;
	t = *e;
	while (*t && isspace(*t)) ++t;
	if (*t == ':') ++t;
	while (*t && isspace(*t)) ++t;
	x = parse_coord(t, e, &r0.ur);
	if (x != 0) return x;
	if (r0.ll.dims != r0.ur.dims) { *e = s; return -1; }
	*r = r0;
	return 0;
}

int sprint_coord(char *buf, coord_t *co) {
	char *s = buf;
	char c = '(';
	int x;
	for (x = 0; x < co->dims; ++x) {
		s += sprintf(s, "%c%d", c, co->coords[x]);
		c = ',';
	}
	*s++ = ')';
	*s = '\0';
	return s - buf;
}

int sprint_rect(char *buf, rect_t *r) {
	char *s = buf;

	s += sprint_coord(s, &r->ll);
	*s++ = ':';
	s += sprint_coord(s, &r->ur);
	return s - buf;
}

int coord2rank(rect_t *world, coord_t *coord) {
	int rank = 0;
	int d;
	for (d = 0; d < world->ll.dims; ++d) {
		rank *= (world->ur.coords[d] - world->ll.coords[d] + 1);
		rank += coord->coords[d];
	}
	return rank;
}

void rank2coord(rect_t *world, int rank, coord_t *coord) {
	int d;
	for (d = world->ll.dims - 1; d >= 0; --d) {
		int x = (world->ur.coords[d] - world->ll.coords[d] + 1);
		coord->coords[d] = rank % x;
		rank /= x;
	}
	coord->dims = world->ll.dims;
}

int rect_size(rect_t *rect) {
	int size = 1;
	int d;
	for (d = 0; d < rect->ll.dims; ++d) {
		size *= (rect->ur.coords[d] - rect->ll.coords[d] + 1);
	}
	return size;
}

void print_classroute(coord_t *me, classroute_t *cr) {
	static char buf[1024];
	char *s = buf;
	s += sprint_coord(s, me);
	s += sprintf(s, " up: ");
	s += sprint_links(s, cr->up_tree);
	s += sprintf(s, " dn: ");
	s += sprint_links(s, cr->dn_tree);
	printf("%s\n", buf);
}

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

	extern int optind;
	extern char *optarg;

	while ((x = getopt(argc, argv, "c:r:w:")) != EOF) {
		switch(x) {
		case 'c':
			e = parse_rect(optarg, &ep, &comm);
			if (e != 0) {
				fprintf(stderr, "invalid rect for sub-comm: \"%s\" at \"%s\"\n", optarg, ep);
				exit(1);
			}
			++comm_set;
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
	classroute_t *cr = (classroute_t *)malloc(z * sizeof(classroute_t));
	if (!cr) {
		fprintf(stderr, "out of memory allocating classroute array!\n");
		exit(1);
	}

	for (x = optind; x < argc; ++x) {
		root_set = 1;
		e = parse_coord(argv[x], &ep, &root);
		if (e != 0) {
			fprintf(stderr, "invalid coord for world root: \"%s\" at \"%s\"\n", argv[x], ep);
			continue;
		}
		if (root.dims != world.ll.dims) {
			fprintf(stderr, "root number of dimensions does not match 'world'\n");
			continue;
		}
		if (x > optind) printf("\n");
		memset(cr, -1, z * sizeof(classroute_t));
		make_classroutes(&world, &root, &comm, cr);
		print_classroutes(&world, &root, &comm, cr);
	}
	if (!root_set) {
		for (x = 0; x < world.ll.dims; ++x) {
			root.coords[x] = world.ll.coords[x] +
				(world.ur.coords[x] - world.ll.coords[x] + 1) / 2;

		}
		root.dims = world.ll.dims;
		memset(cr, -1, z * sizeof(classroute_t));
		make_classroutes(&world, &root, &comm, cr);
		print_classroutes(&world, &root, &comm, cr);
	}
	free(cr);
	exit(0);
}
