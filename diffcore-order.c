/*
 * Copyright (C) 2005 Junio C Hamano
 */
#include "cache.h"
#include "diff.h"
#include "diffcore.h"

static struct order_group **groups;
static int groups_count;

//static void prepare_order(const char *orderfile)
//{
//	int cnt, pass;
//	struct strbuf sb = STRBUF_INIT;
//	void *map;
//	char *cp, *endp;
//	ssize_t sz;
//
//	if (order)
//		return;
//
//	sz = strbuf_read_file(&sb, orderfile, 0);
//	if (sz < 0)
//		die_errno(_("failed to read orderfile '%s'"), orderfile);
//	map = strbuf_detach(&sb, NULL);
//	endp = (char *) map + sz;
//
//	for (pass = 0; pass < 2; pass++) {
//		cnt = 0;
//		cp = map;
//		while (cp < endp) {
//			char *ep;
//			for (ep = cp; ep < endp && *ep != '\n'; ep++)
//				;
//			/* cp to ep has one line */
//			if (*cp == '\n' || *cp == '#')
//				; /* comment */
//			else if (pass == 0)
//				cnt++;
//			else {
//				if (*ep == '\n') {
//					*ep = 0;
//					order[cnt] = cp;
//				} else {
//					order[cnt] = xmemdupz(cp, ep - cp);
//				}
//				cnt++;
//			}
//			if (ep < endp)
//				ep++;
//			cp = ep;
//		}
//		if (pass == 0) {
//			order_cnt = cnt;
//			ALLOC_ARRAY(order, cnt);
//		}
//	}
//
//	// NEW STUFF...
//	struct order_pattern **patterns;
//	ALLOC_ARRAY(patterns, 2);
//	groups_count = 1;
//	ALLOC_ARRAY(groups, groups_count);
//	patterns[0] = xmalloc(sizeof(struct order_pattern));
//	patterns[1] = xmalloc(sizeof(struct order_pattern));
//	patterns[0]->pattern = "*.h";
//	patterns[1]->pattern = "*.c*";
//
//	groups[0] = xmalloc(sizeof(struct order_group));
//	ALLOC_ARRAY(groups[0]->patterns, 2);
//	groups[0]->patterns[0] = xmalloc(sizeof(struct order_pattern));
//	groups[0]->patterns[0] = patterns[0];
//
//	groups[0]->patterns[1] = xmalloc(sizeof(struct order_pattern));
//	groups[0]->patterns[1] = patterns[1];
//	groups[0]->pattern_count = 2;
//
//	printf("pattern 1: %s %d\n", patterns[0]->pattern, patterns[0]->match_count);
//	printf("pattern 2: %s %d\n", patterns[1]->pattern, patterns[1]->match_count);
//
//}

static void prepare_order(const char *orderfile)
{
	struct order_pattern **patterns;

	groups_count = 2;
	ALLOC_ARRAY(groups, groups_count);

	ALLOC_ARRAY(patterns, 3);
	patterns[0] = xmalloc(sizeof(struct order_pattern));
	patterns[1] = xmalloc(sizeof(struct order_pattern));
	patterns[2] = xmalloc(sizeof(struct order_pattern));
	patterns[0]->pattern = "*.sh";
	patterns[0]->match_count = 0;
	patterns[1]->pattern = "*.h";
	patterns[1]->match_count = 0;
	patterns[2]->pattern = "*.c*";
	patterns[2]->match_count = 0;

	groups[0] = xmalloc(sizeof(struct order_group));
	ALLOC_ARRAY(groups[0]->patterns, 2);
	groups[0]->patterns[0] = xmalloc(sizeof(struct order_pattern));
	groups[0]->patterns[0] = patterns[0];

	groups[0]->patterns[1] = xmalloc(sizeof(struct order_pattern));
	groups[0]->patterns[1] = patterns[1];

	groups[0]->pattern_count = 2;

	groups[1] = xmalloc(sizeof(struct order_group));
	ALLOC_ARRAY(groups[1]->patterns, 1);
	groups[1]->patterns[0] = xmalloc(sizeof(struct order_pattern));
	groups[1]->patterns[0] = patterns[2];

	groups[1]->pattern_count = 1;
}

static void match_order(struct obj_order *o, obj_path_fn_t obj_path)
{
	int i, j;
	static struct strbuf p = STRBUF_INIT;

	for (i = 0; i < groups_count; i++)
	{
		for (j = 0; j < groups[i]->pattern_count; j++)
		{
			strbuf_reset(&p);
			strbuf_addstr(&p, obj_path(o->obj));
			while (p.buf[0]) {
				char *cp;
				if (!wildmatch(groups[i]->patterns[j]->pattern, p.buf, 0))
				{
					int multiplier = groups[i]->patterns[j]->match_count++;
					o->group = i;
					o->ranking = j + multiplier * groups[i]->pattern_count;
					return;
				}
				cp = strrchr(p.buf, '/');
				if (!cp)
					break;
				*cp = 0;
			}
		}
	}
}

static int compare_objs_order(const void *a_, const void *b_)
{
	struct obj_order const *a, *b;
	a = (struct obj_order const *)a_;
	b = (struct obj_order const *)b_;
	if (a->group != b->group)
		return a->group - b->group;
	return a->ranking - b->ranking;
}

void order_objects(const char *orderfile, obj_path_fn_t obj_path,
		   struct obj_order *objs, int nr)
{
	int i;

	if (!nr)
		return;

	prepare_order(orderfile);
	for (i = 0; i < nr; i++) {
		objs[i].group = groups_count;
		objs[i].ranking = i;
		match_order(&objs[i], obj_path);
	}
	QSORT(objs, nr, compare_objs_order);
}

static const char *pair_pathtwo(void *obj)
{
	struct diff_filepair *pair = (struct diff_filepair *)obj;

	return pair->two->path;
}

void diffcore_order(const char *orderfile)
{
	struct diff_queue_struct *q = &diff_queued_diff;
	struct obj_order *o;
	int i;

	if (!q->nr)
		return;

	ALLOC_ARRAY(o, q->nr);
	for (i = 0; i < q->nr; i++)
		o[i].obj = q->queue[i];
	order_objects(orderfile, pair_pathtwo, o, q->nr);
	for (i = 0; i < q->nr; i++)
		q->queue[i] = o[i].obj;
	free(o);
	return;
}
