/* file containing boilerplate comparator function definitions as not to clutter
 * the more interesting source files
 */

#include "dbfilter.h"
#include <strings.h>

int namecmp(const void *i1, const void *i2) {
	dbentry_t *e1 = (dbentry_t *)i1;
	dbentry_t *e2 = (dbentry_t *)i2;
	int comparison = strcasecmp(e1->firstname, e2->firstname);
	if (comparison == 0) {
		return (e1->entrynumber != e2->entrynumber)
			? ((e1->entrynumber < e2->entrynumber) ? -1 : 1)
			: 0;
	}
	return comparison;
}

int surnamecmp(const void *i1, const void *i2) {
	dbentry_t *e1 = (dbentry_t *)i1;
	dbentry_t *e2 = (dbentry_t *)i2;
	int comparison = strcasecmp(e1->lastname, e2->lastname);
	if (comparison == 0) {
		return (e1->entrynumber != e2->entrynumber)
			? ((e1->entrynumber < e2->entrynumber) ? -1 : 1)
			: 0;
	}
	return comparison;
}

int daycmp(const void *i1, const void *i2) {
	dbentry_t *e1 = (dbentry_t *)i1;
	dbentry_t *e2 = (dbentry_t *)i2;
	int comparison = (e1->day != e2->day) ? ((e1->day < e2->day) ? -1 : 1) : 0;
	if (comparison == 0) {
		return (e1->entrynumber != e2->entrynumber)
			? ((e1->entrynumber < e2->entrynumber) ? -1 : 1)
			: 0;
	}
	return comparison;
}

int monthcmp(const void *i1, const void *i2) {
	dbentry_t *e1 = (dbentry_t *)i1;
	dbentry_t *e2 = (dbentry_t *)i2;
	int comparison = (e1->month != e2->month) ? ((e1->month < e2->month) ? -1 : 1) : 0;
	if (comparison == 0) {
		return (e1->entrynumber != e2->entrynumber)
			? ((e1->entrynumber < e2->entrynumber) ? -1 : 1)
			: 0;
	}
	return comparison;
}

int yearcmp(const void *i1, const void *i2) {
	dbentry_t *e1 = (dbentry_t *)i1;
	dbentry_t *e2 = (dbentry_t *)i2;
	int comparison = (e1->year != e2->year) ? ((e1->year < e2->year) ? -1 : 1) : 0;
	if (comparison == 0) {
		return (e1->entrynumber != e2->entrynumber)
			? ((e1->entrynumber < e2->entrynumber) ? -1 : 1)
			: 0;
	}
	return comparison;
}

int incomecmp(const void *i1, const void *i2) {
	dbentry_t *e1 = (dbentry_t *)i1;
	dbentry_t *e2 = (dbentry_t *)i2;
	int comparison = (e1->income != e2->income) ? ((e1->income < e2->income) ? -1 : 1) : 0;
	if (comparison == 0) {
		return (e1->entrynumber != e2->entrynumber)
			? ((e1->entrynumber < e2->entrynumber) ? -1 : 1)
			: 0;
	}
	return comparison;
}
