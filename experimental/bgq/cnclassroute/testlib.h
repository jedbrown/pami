/**
 * \file experimental/cnclassroute/testlib.h
 * \brief ???
 */
#ifndef __experimental_cnclassroute_testlib_h__
#define __experimental_cnclassroute_testlib_h__

#include <stdio.h>

#include "cnclassroute.h"

char *dim_names = XMI_DIM_NAMES;

typedef struct {
	rect_t rect;
	coord_t root;
	int pri_dim;
} commworld_t;

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
	if (s == buf) { *s++ = ' '; *s++ = ' '; }
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

int parse_sparse(char *s, char **ep, rect_t *r, coord_t **exc, int *nexc) {
	char *t = NULL, *u;
	int e, n;
	coord_t *c;
	e = parse_rect(s, &t, r);
	if (e != 0) {
		*ep = t;
		return e;
	}
	if (!t || *t == '\0') {
		*ep = t;
		*nexc = 0;
		*exc = NULL;
		return 0;
	}
	for (n = 0, u = t; *u; ++u) if (*u == '-') ++n;
	c = (coord_t *)malloc(n * sizeof(*c));
	n = 0;
	while (*t && *t == '-') {
		++t;
		e = parse_coord(t, &u, &c[n]);
		if (e != 0) { *ep = u; return e; }
		t = u;
		++n;
	}
	*exc = c;
	*nexc = n;
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
		rank += (coord->coords[d] - world->ll.coords[d]);
	}
	return rank;
}

void rank2coord(rect_t *world, int rank, coord_t *coord) {
	int d;
	for (d = world->ll.dims - 1; d >= 0; --d) {
		int x = (world->ur.coords[d] - world->ll.coords[d] + 1);
		coord->coords[d] = world->ll.coords[d] + (rank % x);
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

#define ERR_O_CRCONFLICT	0x8000	/**< two classroutes conflict? */
#define ERR_O_ONEROOT		0x4000	/**< not exactly one root? */
#define ERR_O_MULTIUPLINK	0x2000	/**< more than one up link? */
#define ERR_O_UNCONN		0x1000	/**< link not connected */
#define ERR_O_OUTBOUNDS		0x0002	/**< link is outside rectangle */
/*				0x0001	*/

#define ERR_I_CRCONFLICT	0x8000	/**< two classroutes conflict? */
#define ERR_I_UPDNCONFLICT	0x4000	/**< any up/dn links same? */
#define ERR_I_LOOP		0x2000	/**< loop in connections */
#define ERR_I_UNCONN		0x1000	/**< link not connected */
#define ERR_I_OUTBOUNDS		0x0002	/**< link is outside rectangle */
#define FLAG_I_VISITED		0x0001	/**< internal - !orphaned */

#define ERRS	(ERR_O_CRCONFLICT	| \
		 ERR_O_ONEROOT		| \
		 ERR_O_MULTIUPLINK	| \
		 ERR_O_OUTBOUNDS	| \
		 ERR_O_UNCONN		| \
		 ERR_I_CRCONFLICT	| \
		 ERR_I_UPDNCONFLICT	| \
		 ERR_I_OUTBOUNDS	| \
		 ERR_I_UNCONN		| \
		 ERR_I_LOOP		| \
		 FLAG_I_VISITED)
#define CHK_CRCONFLICT(a,b)	(((a & b) & ~ERRS) != 0)

void chk_classroute(ClassRoute_t *cra, ClassRoute_t *crb) {
	if (CHK_CRCONFLICT(cra->output, crb->output)) {
		cra->output |= ERR_O_CRCONFLICT;
		crb->output |= ERR_O_CRCONFLICT;
	}
	if (CHK_CRCONFLICT(cra->input, crb->input)) {
		cra->input |= ERR_I_CRCONFLICT;
		crb->input |= ERR_I_CRCONFLICT;
	}
}

void chk_sanity(ClassRoute_t *cr, int *nroots) {
	uint32_t ul, dl;
	/* check up link for zero or one link */
	ul = cr->output & ~ERRS;
	if (!ul) {
		++(*nroots);
		if (*nroots > 1) {
			cr->output |= ERR_O_ONEROOT;
		}
	} else if ((ul & (ul - 1)) != 0) {
		cr->output |= ERR_O_MULTIUPLINK;
	}
	dl = (cr->input & ~ERRS);
	if ((ul & dl) || !(ul | dl)) {
		cr->input |= ERR_I_UPDNCONFLICT;
	}
}

void print_classroute(coord_t *me, ClassRoute_t *cr, int rank) {
	static char buf[1024];
	char *s = buf;
	s += sprint_coord(s, me);
	//s += sprintf(s, "[%d]", rank);
	s += sprintf(s, " up: ");
	s += sprint_links(s, cr->output & ~ERRS);
	s += sprintf(s, " dn: ");
	s += sprint_links(s, cr->input & ~ERRS);
	printf("%s\n", buf);
}

void print_classroute_errs(coord_t *me, ClassRoute_t *cr, int rank) {
	static char buf[1024];
	char *s = buf;
	s += sprint_coord(s, me);
	//s += sprintf(s, "[%d]", rank);
	*s++ = ' ';
	*s++ = '|';
	*s++ = (cr->output & ERR_O_CRCONFLICT   ? '*' : ' ');
	*s++ = (cr->output & ERR_O_ONEROOT      ? 'R' : ' ');
	*s++ = (cr->output & ERR_O_MULTIUPLINK  ? 'M' : ' ');
	*s++ = (cr->output & ERR_O_OUTBOUNDS    ? 'V' : ' ');
	*s++ = (cr->output & ERR_O_UNCONN       ? 'X' : ' ');
	*s++ = '|';
	*s++ = (cr->input & ERR_I_CRCONFLICT   ? '*' : ' ');
	*s++ = (cr->input & ERR_I_UPDNCONFLICT ? 'C' : ' ');
	*s++ = (cr->input & ERR_I_OUTBOUNDS    ? 'V' : ' ');
	*s++ = (cr->input & ERR_I_UNCONN       ? 'X' : ' ');
	*s++ = (cr->input & ERR_I_LOOP         ? '!' : ' ');
	*s++ = ((cr->input & FLAG_I_VISITED) == 0 ? '?' : ' ');
	*s++ = '|';
	s += sprintf(s, " up: ");
	s += sprint_links(s, cr->output & ~ERRS);
	s += sprintf(s, " dn: ");
	s += sprint_links(s, cr->input & ~ERRS);
	printf("%s\n", buf);
}

void recurse_dims(commworld_t *cw, rect_t *comm, coord_t *me, ClassRoute_t *cr) {
	int x;
	for (x = comm->ll.coords[me->dims]; x <= comm->ur.coords[me->dims]; ++x) {
		me->coords[me->dims] = x;
		++me->dims;
		if (me->dims == cw->rect.ll.dims) {
			int r = coord2rank(comm, me);
			build_node_classroute(&cw->rect, &cw->root, me, comm, cw->pri_dim, &cr[r]);
		} else {
			recurse_dims(cw, comm, me, cr);
		}
		--me->dims;
	}
}

void make_classroutes(commworld_t *cw, rect_t *comm, ClassRoute_t *cr) {
	coord_t me = { 0 };
	recurse_dims(cw, comm, &me, cr);
}

#ifdef SUPPORT_SPARSE_RECTANGLE

void recurse_dims_sparse(commworld_t *cw, rect_t *comm, coord_t *excl, int nexcl,
				coord_t *me, ClassRoute_t *cr) {
	int x;
	for (x = comm->ll.coords[me->dims]; x <= comm->ur.coords[me->dims]; ++x) {
		me->coords[me->dims] = x;
		++me->dims;
		if (me->dims == cw->rect.ll.dims) {
			int r = coord2rank(comm, me);
			build_node_classroute_sparse(&cw->rect, &cw->root, me,
							comm, excl, nexcl,
							cw->pri_dim, &cr[r]);
		} else {
			recurse_dims_sparse(cw, comm, excl, nexcl, me, cr);
		}
		--me->dims;
	}
}
void make_classroutes_sparse(commworld_t *cw, rect_t *comm, coord_t *excl, int nexcl, ClassRoute_t *cr) {
	coord_t me = { 0 };
	recurse_dims_sparse(cw, comm, excl, nexcl, &me, cr);
}

#endif /* SUPPORT_SPARSE_RECTANGLE */

void chk_conn(commworld_t *cw, rect_t *comm, ClassRoute_t *cr) {
	int z = rect_size(comm);
	int r, t, n, s;
	uint32_t ul, dl, ol, m;
	static int signs[] = {
		[CR_SIGN_POS] = 1,
		[CR_SIGN_NEG] = -1,
	};

	for (r = 0; r < z; ++r) {
		coord_t c0, c1;
		rank2coord(comm, r, &c0);
		ul = cr[r].output;
		dl = cr[r].input;
		for (n = 0; n < XMI_MAX_DIMS; ++n) {
			for (s = 0; s < 2; ++s) {
				m = CR_LINK(n, s);
				ol = CR_LINK(n, (1 - s));
				c1 = c0;
				c1.coords[n] += signs[s];
				if (c1.coords[n] < comm->ll.coords[n] ||
				    c1.coords[n] > comm->ur.coords[n]) {
					t = -1;
				} else {
					t = coord2rank(comm, &c1);
				}
				if (ul & m) {
					if (t == -1) {
						cr[r].output |= ERR_O_OUTBOUNDS;
					} else if ((cr[t].input & ol) == 0) {
						cr[r].output |= ERR_O_UNCONN;
					}
				}
				if (dl & m) {
					if (t == -1) {
						cr[r].input |= ERR_I_OUTBOUNDS;
					} else if ((cr[t].output & ol) == 0) {
						cr[r].input |= ERR_I_UNCONN;
					}
				}
			}
		}
	}
}

void traverse_down(commworld_t *cw, rect_t *comm, ClassRoute_t *cr, int curr) {
	uint32_t l, m;
	int n, s, t;
	coord_t c0, c1;
	static int signs[] = {
		[CR_SIGN_POS] = 1,
		[CR_SIGN_NEG] = -1,
	};

	l = cr[curr].input;
	if (l & FLAG_I_VISITED) {
		cr[curr].input |= ERR_I_LOOP;
		return;
	}
	cr[curr].input |= FLAG_I_VISITED;
	if (!(l & ~ERRS)) return;	/* leaf node */
	rank2coord(comm, curr, &c0);
	for (n = 0; n < cw->rect.ll.dims; ++n) {
		for (s = 0; s < 2; ++s) {
			m = CR_LINK(n, s);
			if (l & m) {
				c1 = c0;
				c1.coords[n] += signs[s];
				if (c1.coords[n] < comm->ll.coords[n] ||
				    c1.coords[n] > comm->ur.coords[n]) {
					/* error - already flagged by chk_conn() */
					continue;
				}
				t = coord2rank(comm, &c1);
				traverse_down(cw, comm, cr, t);
			}
		}
	}
}

void chk_visit(commworld_t *cw, rect_t *comm, ClassRoute_t *cr) {
	int r, z = rect_size(comm);
	/*
	 * find root node, traverse all connections and ensure each node is
	 * visited exactly once. Assumes one root node - only traverses first root.
	 */
	for (r = 0; r < z; ++r) {
		if ((cr[r].output & ~ERRS) == 0) break;
	}
	if (r >= z) {
		/* error: no root node - all nodes show orphaned */
		return;
	}
	traverse_down(cw, comm, cr, r);
	/* any node without FLAG_I_VISITED is orphaned... */
}

void chk_all_sanity(commworld_t *cw, rect_t *comm, ClassRoute_t *cr) {
	int z = rect_size(comm);
	int r, nroots = 0;

	for (r = 0; r < z; ++r) {
		chk_sanity(&cr[r], &nroots);
	}
	if (!nroots) {
		cr[0].output |= ERR_O_ONEROOT;
	}
	chk_conn(cw, comm, cr);
	chk_visit(cw, comm, cr);
}

void print_classroutes(commworld_t *cw, rect_t *comm, ClassRoute_t *cr, int errs) {
	static char buf[1024];
	int z = rect_size(comm);
	char *s = buf;
	int r;

	s += sprintf(s, "Classroute for comm ");
	s += sprint_rect(s, comm);
	s += sprintf(s, "\n\tin world ");
	s += sprint_rect(s, &cw->rect);
	s += sprintf(s, "\n\twith root ");
	s += sprint_coord(s, &cw->root);
	printf("%s\n\tand primary dimension '%c':\n", buf, dim_names[cw->pri_dim]);
	for (r = 0; r < z; ++r) {
		coord_t c;
		rank2coord(comm, r, &c);
		if (errs) {
			print_classroute_errs(&c, &cr[r], r);
		} else {
			print_classroute(&c, &cr[r], r);
		}
	}
}

#endif /* __experimental_cnclassroute_testlib_h__ */
