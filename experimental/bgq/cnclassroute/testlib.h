/**
 * \file experimental/cnclassroute/testlib.h
 * \brief ???
 */
#ifndef __experimental_bgq_cnclassroute_testlib_h__
#define __experimental_bgq_cnclassroute_testlib_h__

#include <stdio.h>

#include "cnclassroute.h"

char *dim_names = CR_DIM_NAMES;

typedef struct {
	CR_RECT_T rect;
	CR_COORD_T root;
	int pri_dim;
} commworld_t;

typedef struct {
	uint32_t flags;
	ClassRoute_t cr;
} classroute_t;

int sprint_links(char *buf, uint32_t link) {
	char *s = buf;
	int x;
	for (x = 0; x < CR_NUM_DIMS; ++x) {
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

int parse_coord(char *s, char **e, CR_COORD_T *c) {
	CR_COORD_T c0;
	char *t;

	/* syntax: "( %d, %d, ... )" */
	while (*s && *s != '(') ++s;
	if (*s == '(') ++s;
	int dims = 0;
	while (1) {
		while (*s && isspace(*s)) ++s;
		if (!*s) {
			if (e) *e = s;
			return -1;
		}
		if (*s == ')') {
			int x;
			if (e) *e = ++s;
			while (dims < CR_NUM_DIMS) {
				CR_COORD_DIM(&c0,dims) = 0;
				++dims;
			}
			*c = c0;
			return 0;
		}
		if (dims >= CR_NUM_DIMS) {
			if (e) *e = s;
			return -1;
		}
		CR_COORD_DIM(&c0,dims) = strtol(s, &t, 0);
		if (s == t) {
			if (e) *e = s;
			return -1;
		}
		s = t;
		if (*s == ',') ++s;
		++dims;
	}
}

int parse_rect(char *s, char **e, CR_RECT_T *r) {
	int x;
	CR_RECT_T r0;
	char *t;

	/* syntax: "(%d,%d,...) : (%d,%d,...)" */
	x = parse_coord(s, e, CR_RECT_LL(&r0));
	if (x != 0) return x;
	t = *e;
	while (*t && isspace(*t)) ++t;
	if (*t == ':') ++t;
	while (*t && isspace(*t)) ++t;
	x = parse_coord(t, e, CR_RECT_UR(&r0));
	if (x != 0) return x;
	*r = r0;
	return 0;
}

int parse_sparse(char *s, char **ep, CR_RECT_T *r, CR_COORD_T **exc, int *nexc) {
	char *t = NULL, *u;
	int e, n;
	CR_COORD_T *c;
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
	c = (CR_COORD_T *)malloc(n * sizeof(*c));
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

int sprint_coord(char *buf, CR_COORD_T *co) {
	char *s = buf;
	char c = '(';
	int x;
	for (x = 0; x < CR_NUM_DIMS; ++x) {
		s += sprintf(s, "%c%d", c, CR_COORD_DIM(co,x));
		c = ',';
	}
	*s++ = ')';
	*s = '\0';
	return s - buf;
}

int sprint_rect(char *buf, CR_RECT_T *r) {
	char *s = buf;

	s += sprint_coord(s, CR_RECT_LL(r));
	*s++ = ':';
	s += sprint_coord(s, CR_RECT_UR(r));
	return s - buf;
}

int coord2rank(CR_RECT_T *world, CR_COORD_T *coord) {
	int rank = 0;
	int d;
	for (d = 0; d < CR_NUM_DIMS; ++d) {
		rank *= (CR_COORD_DIM(CR_RECT_UR(world),d) - CR_COORD_DIM(CR_RECT_LL(world),d) + 1);
		rank += (CR_COORD_DIM(coord,d) - CR_COORD_DIM(CR_RECT_LL(world),d));
	}
	return rank;
}

void rank2coord(CR_RECT_T *world, int rank, CR_COORD_T *coord) {
	int d;
	for (d = CR_NUM_DIMS - 1; d >= 0; --d) {
		int x = (CR_COORD_DIM(CR_RECT_UR(world),d) - CR_COORD_DIM(CR_RECT_LL(world),d) + 1);
		CR_COORD_DIM(coord,d) = CR_COORD_DIM(CR_RECT_LL(world),d) + (rank % x);
		rank /= x;
	}
}

int rect_size(CR_RECT_T *rect) {
	int size = 1;
	int d;
	for (d = 0; d < CR_NUM_DIMS; ++d) {
		size *= (CR_COORD_DIM(CR_RECT_UR(rect),d) - CR_COORD_DIM(CR_RECT_LL(rect),d) + 1);
	}
	return size;
}

#define ERR_O_CRCONFLICT	0x80000000	/**< two classroutes conflict? */
#define ERR_O_ONEROOT		0x40000000	/**< not exactly one root? */
#define ERR_O_MULTIUPLINK	0x20000000	/**< more than one up link? */
#define ERR_O_UNCONN		0x10000000	/**< link not connected */
#define ERR_O_OUTBOUNDS		0x08000000	/**< link is outside rectangle */
/*				0x04000000	*/

#define ERR_I_CRCONFLICT	0x00008000	/**< two classroutes conflict? */
#define ERR_I_UPDNCONFLICT	0x00004000	/**< any up/dn links same? */
#define ERR_I_LOOP		0x00002000	/**< loop in connections */
#define ERR_I_UNCONN		0x00001000	/**< link not connected */
#define ERR_I_OUTBOUNDS		0x00000800	/**< link is outside rectangle */
#define FLAG_I_VISITED		0x00000400	/**< internal - !orphaned */

#define CHK_CRCONFLICT(a,b)	((a & b) != 0)

void chk_classroute(classroute_t *cra, classroute_t *crb) {
	if (CHK_CRCONFLICT(CR_ROUTE_UP(&cra->cr), CR_ROUTE_UP(&crb->cr))) {
		cra->flags |= ERR_O_CRCONFLICT;
		crb->flags |= ERR_O_CRCONFLICT;
	}
	if (CHK_CRCONFLICT(CR_ROUTE_DOWN(&cra->cr), CR_ROUTE_DOWN(&crb->cr))) {
		cra->flags |= ERR_I_CRCONFLICT;
		crb->flags |= ERR_I_CRCONFLICT;
	}
}

void chk_sanity(classroute_t *cr, int *nroots) {
	uint32_t ul, dl;
	/* check up link for zero or one link */
	ul = CR_ROUTE_UP(&cr->cr);
	if (!ul) {
		++(*nroots);
		if (*nroots > 1) {
			cr->flags |= ERR_O_ONEROOT;
		}
	} else if ((ul & (ul - 1)) != 0) {
		cr->flags |= ERR_O_MULTIUPLINK;
	}
	dl = CR_ROUTE_DOWN(&cr->cr);
	if ((ul & dl) || !(ul | dl)) {
		cr->flags |= ERR_I_UPDNCONFLICT;
	}
}

void print_classroute(CR_COORD_T *me, classroute_t *cr, int rank) {
	static char buf[1024];
	char *s = buf;
	s += sprint_coord(s, me);
	//s += sprintf(s, "[%d]", rank);
	s += sprintf(s, " up: ");
	s += sprint_links(s, CR_ROUTE_UP(&cr->cr));
	s += sprintf(s, " dn: ");
	s += sprint_links(s, CR_ROUTE_DOWN(&cr->cr));
	printf("%s\n", buf);
}

void print_classroute_errs(CR_COORD_T *me, classroute_t *cr, int rank) {
	static char buf[1024];
	char *s = buf;
	s += sprint_coord(s, me);
	//s += sprintf(s, "[%d]", rank);
	*s++ = ' ';
	*s++ = '|';
	*s++ = (cr->flags & ERR_O_CRCONFLICT   ? '*' : ' ');
	*s++ = (cr->flags & ERR_O_ONEROOT      ? 'R' : ' ');
	*s++ = (cr->flags & ERR_O_MULTIUPLINK  ? 'M' : ' ');
	*s++ = (cr->flags & ERR_O_OUTBOUNDS    ? 'V' : ' ');
	*s++ = (cr->flags & ERR_O_UNCONN       ? 'X' : ' ');
	*s++ = '|';
	*s++ = (cr->flags & ERR_I_CRCONFLICT   ? '*' : ' ');
	*s++ = (cr->flags & ERR_I_UPDNCONFLICT ? 'C' : ' ');
	*s++ = (cr->flags & ERR_I_OUTBOUNDS    ? 'V' : ' ');
	*s++ = (cr->flags & ERR_I_UNCONN       ? 'X' : ' ');
	*s++ = (cr->flags & ERR_I_LOOP         ? '!' : ' ');
	*s++ = ((cr->flags & FLAG_I_VISITED) == 0 ? '?' : ' ');
	*s++ = '|';
	s += sprintf(s, " up: ");
	s += sprint_links(s, CR_ROUTE_UP(&cr->cr));
	s += sprintf(s, " dn: ");
	s += sprint_links(s, CR_ROUTE_DOWN(&cr->cr));
	printf("%s\n", buf);
}

typedef void (*recurse_func_t)(commworld_t *cw, CR_RECT_T *comm, CR_COORD_T *me, void *v);

void recurse_dims(commworld_t *cw, CR_RECT_T *comm, CR_COORD_T *me, int dim, recurse_func_t func, void *param) {
	int x;
	for (x = CR_COORD_DIM(CR_RECT_LL(comm),dim); x <= CR_COORD_DIM(CR_RECT_UR(comm),dim); ++x) {
		CR_COORD_DIM(me,dim) = x;
		if (dim + 1 == CR_NUM_DIMS) {
			func(cw, comm, me, param);
		} else {
			recurse_dims(cw, comm, me, dim + 1, func, param);
		}
	}
}

void recurse_build_cr(commworld_t *cw, CR_RECT_T *comm, CR_COORD_T *me, void *v) {
	classroute_t *cr = (classroute_t *)v;

	int r = coord2rank(comm, me);
	cr[r].flags = 0;
	build_node_classroute(&cw->rect, &cw->root, me, comm, cw->pri_dim, &cr[r].cr);
}

void make_classroutes(commworld_t *cw, CR_RECT_T *comm, classroute_t *cr) {
	CR_COORD_T me = { 0 };
	recurse_dims(cw, comm, &me, 0, recurse_build_cr, cr);
}

struct recurse_build_cr_sparse_s {
	classroute_t *cr;
	CR_COORD_T *excl;
	int nexcl;
};
void recurse_build_cr_sparse(commworld_t *cw, CR_RECT_T *comm, CR_COORD_T *me, void *v) {
	struct recurse_build_cr_sparse_s *st = (struct recurse_build_cr_sparse_s *)v;
	classroute_t *cr = st->cr;

	int r = coord2rank(comm, me);
	cr[r].flags = 0;
	build_node_classroute(&cw->rect, &cw->root, me, comm, cw->pri_dim, &cr[r].cr);
	build_node_classroute_sparse(&cw->rect, &cw->root, me, comm,
				st->excl, st->nexcl, cw->pri_dim, &cr[r].cr);
}

void make_classroutes_sparse(commworld_t *cw, CR_RECT_T *comm, CR_COORD_T *excl, int nexcl, classroute_t *cr) {
	struct recurse_build_cr_sparse_s st;
	st.cr = cr;
	st.excl = excl;
	st.nexcl = nexcl;
	CR_COORD_T me = { 0 };
	recurse_dims(cw, comm, &me, 0, recurse_build_cr_sparse, &st);
}

struct recurse_alloc_cr_s {
	int vc;
	uint32_t bitmap;
	int id;
	void **env;
};
void recurse_alloc_cr(commworld_t *cw, CR_RECT_T *comm, CR_COORD_T *me, void *v) {
	uint32_t m;
	struct recurse_alloc_cr_s *st = (struct recurse_alloc_cr_s *)v;

	int r = coord2rank(&cw->rect, me);
	m = get_classroute_ids(st->vc, comm, &st->env[r]); // must use per-node tables...
	/*
	 * this simulates an allreduce for bit-wise AND...
	 */
	st->bitmap &= m;
}
void recurse_set_cr(commworld_t *cw, CR_RECT_T *comm, CR_COORD_T *me, void *v) {
	uint32_t m;
	struct recurse_alloc_cr_s *st = (struct recurse_alloc_cr_s *)v;

	int r = coord2rank(&cw->rect, me);
	m = set_classroute_id(st->id, st->vc, comm, &st->env[r]); // must use per-node tables...
}

int alloc_classroutes(commworld_t *cw, CR_RECT_T *comm, int vc, void **env) {
	CR_COORD_T me = { 0 };
	struct recurse_alloc_cr_s st;
	st.env = env;
	st.vc = vc;
	st.bitmap = (uint32_t)-1;
	recurse_dims(cw, comm, &me, 0, recurse_alloc_cr, &st);
	int x = ffs(st.bitmap);
	if (!x) return -1;
	--x;
	st.id = x;
	recurse_dims(cw, comm, &me, 0, recurse_set_cr, &st);
	return x;
}

void chk_conn(commworld_t *cw, CR_RECT_T *comm, classroute_t *cr) {
	int z = rect_size(comm);
	int r, t, n, s;
	uint32_t ul, dl, ol, m;
	static int signs[] = {
		[CR_SIGN_POS] = 1,
		[CR_SIGN_NEG] = -1,
	};

	for (r = 0; r < z; ++r) {
		CR_COORD_T c0, c1;
		rank2coord(comm, r, &c0);
		ul = CR_ROUTE_UP(&cr[r].cr);
		dl = CR_ROUTE_DOWN(&cr[r].cr);
		for (n = 0; n < CR_NUM_DIMS; ++n) {
			for (s = 0; s < 2; ++s) {
				m = CR_LINK(n, s);
				ol = CR_LINK(n, (1 - s));
				c1 = c0;
				CR_COORD_DIM(&c1,n) += signs[s];
				if (CR_COORD_DIM(&c1,n) < CR_COORD_DIM(CR_RECT_LL(comm),n) ||
				    CR_COORD_DIM(&c1,n) > CR_COORD_DIM(CR_RECT_UR(comm),n)) {
					t = -1;
				} else {
					t = coord2rank(comm, &c1);
				}
				if (ul & m) {
					if (t == -1) {
						cr[r].flags |= ERR_O_OUTBOUNDS;
					} else if ((CR_ROUTE_DOWN(&cr[t].cr) & ol) == 0) {
						cr[r].flags |= ERR_O_UNCONN;
					}
				}
				if (dl & m) {
					if (t == -1) {
						cr[r].flags |= ERR_I_OUTBOUNDS;
					} else if ((CR_ROUTE_UP(&cr[t].cr) & ol) == 0) {
						cr[r].flags |= ERR_I_UNCONN;
					}
				}
			}
		}
	}
}

void traverse_down(commworld_t *cw, CR_RECT_T *comm, classroute_t *cr, int curr) {
	uint32_t l, m;
	int n, s, t;
	CR_COORD_T c0, c1;
	static int signs[] = {
		[CR_SIGN_POS] = 1,
		[CR_SIGN_NEG] = -1,
	};

	l = CR_ROUTE_DOWN(&cr[curr].cr);
	if (cr[curr].flags & FLAG_I_VISITED) {
		cr[curr].flags |= ERR_I_LOOP;
		return;
	}
	cr[curr].flags |= FLAG_I_VISITED;
	if (!l) return;	/* leaf node */
	rank2coord(comm, curr, &c0);
	for (n = 0; n < CR_NUM_DIMS; ++n) {
		for (s = 0; s < 2; ++s) {
			m = CR_LINK(n, s);
			if (l & m) {
				c1 = c0;
				CR_COORD_DIM(&c1,n) += signs[s];
				if (CR_COORD_DIM(&c1,n) < CR_COORD_DIM(CR_RECT_LL(comm),n) ||
				    CR_COORD_DIM(&c1,n) > CR_COORD_DIM(CR_RECT_UR(comm),n)) {
					/* error - already flagged by chk_conn() */
					continue;
				}
				t = coord2rank(comm, &c1);
				traverse_down(cw, comm, cr, t);
			}
		}
	}
}

void chk_visit(commworld_t *cw, CR_RECT_T *comm, classroute_t *cr) {
	int r, z = rect_size(comm);
	/*
	 * find root node, traverse all connections and ensure each node is
	 * visited exactly once. Assumes one root node - only traverses first root.
	 */
	for (r = 0; r < z; ++r) {
		if (CR_ROUTE_UP(&cr[r].cr) == 0) break;
	}
	if (r >= z) {
		/* error: no root node - all nodes show orphaned */
		return;
	}
	traverse_down(cw, comm, cr, r);
	/* any node without FLAG_I_VISITED is orphaned... */
}

void chk_all_sanity(commworld_t *cw, CR_RECT_T *comm, classroute_t *cr) {
	int z = rect_size(comm);
	int r, nroots = 0;

	for (r = 0; r < z; ++r) {
		chk_sanity(&cr[r], &nroots);
	}
	if (!nroots) {
		cr[0].flags |= ERR_O_ONEROOT;
	}
	chk_conn(cw, comm, cr);
	chk_visit(cw, comm, cr);
}

void print_classroutes(commworld_t *cw, CR_RECT_T *comm, classroute_t *cr, int errs) {
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
		CR_COORD_T c;
		rank2coord(comm, r, &c);
		if (errs) {
			print_classroute_errs(&c, &cr[r], r);
		} else {
			print_classroute(&c, &cr[r], r);
		}
	}
}

#endif /* __experimental_bgq_cnclassroute_testlib_h__ */
