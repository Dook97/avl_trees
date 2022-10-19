#include "explorer.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

AVL_DEFINE_ROOT(db_t, dbentry_t);

int charcount(const char *str, char c) {
	int out = 0;
	for (; *str != '\0'; ++str)
		if (*str == c)
			++out;
	return out;
}

int readdb(dbentry_t **out, size_t *count) {
	dbentry_t *entries = NULL;
	char *line = NULL;
	size_t len = 0;
	size_t nmemb = 0;
	*count = 0;
	while (getline(&line, &len, stdin) != -1) {
		++nmemb;
		if (line == NULL || (entries = reallocarray(entries, nmemb, sizeof(dbentry_t))) == NULL)
			exit(2);
		if (charcount(line, ',') != 5)
			return -1;

		char *delim1 = strchr(line, ',');
		char *delim2 = strchr(delim1 + 1, ',');
		entries[nmemb - 1].firstname = strndup(line, delim1 - line);
		entries[nmemb - 1].lastname = strndup(delim1 + 1, delim2 - delim1 - 1);
                sscanf(delim2 + 1, "%d,%d,%d,%lu\n", &entries[nmemb - 1].day,
                       &entries[nmemb - 1].month, &entries[nmemb - 1].year,
                       &entries[nmemb - 1].income);
		entries[nmemb - 1].entrynumber = nmemb - 1;
        }
	free(line);
	*out = entries;
	*count = nmemb;
	return 0;
}

void freedb(dbentry_t *db, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		free(db[i].firstname);
		free(db[i].lastname);
	}
	free(db);
}

void processargs(int argc, char **argv, db_t *db, dbentry_t *query, args_t *out) {
	out->reverse = false;
	out->specify_upper_bound = false;
	out->half_open = false;

	char opt;
	int optcount = 0;
	bool mandatory = false;
	while ((opt = getopt(argc, argv, "n:s:d:m:y:i:rouh")) != -1) {
		++optcount;
		switch (opt) {
		case 'n':
			*db = AVL_NEW(db_t, avl_node, namecmp);
			query->firstname = optarg;
			mandatory = true;
			break;
		case 's':
			*db = AVL_NEW(db_t, avl_node, surnamecmp);
			query->lastname = optarg;
			mandatory = true;
			break;
		case 'd':
			*db = AVL_NEW(db_t, avl_node, daycmp);
			query->day = atoi(optarg);
			mandatory = true;
			break;
		case 'm':
			*db = AVL_NEW(db_t, avl_node, monthcmp);
			query->month = atoi(optarg);
			mandatory = true;
			break;
		case 'y':
			*db = AVL_NEW(db_t, avl_node, yearcmp);
			query->year = atoi(optarg);
			mandatory = true;
			break;
		case 'i':
			*db = AVL_NEW(db_t, avl_node, incomecmp);
			query->income = strtoul(optarg, NULL, 0);
			mandatory = true;
			break;
		case 'r':
			out->reverse = true;
			break;
		case 'o':
			out->half_open = true;
			break;
		case 'u':
			out->specify_upper_bound = true;
			break;
		case 'h':
			fprintf(stderr, HELPMSG);
			exit(0);
		default:
			fprintf(stderr, USAGEMSG);
			exit(1);
		}
	}
	if (optcount == 0 || !mandatory) {
		fprintf(stderr, USAGEMSG);
		exit(1);
	}
}

int main(int argc, char **argv) {
	db_t db;
	dbentry_t query = (dbentry_t){ .entrynumber = 0 };
	args_t args;
	processargs(argc, argv, &db, &query, &args);

	dbentry_t *entries;
	size_t nmemb;
	if (readdb(&entries, &nmemb) == -1) {
		fprintf(stderr, "shit done goofed\n");
		exit(1);
	}

	for (size_t i = 0; i < nmemb; ++i)
		avl_insert(&db, &entries[i]);

	dbentry_t *lower_bound = (args.half_open && args.specify_upper_bound) ? NULL : &query;
	dbentry_t *upper_bound = (args.half_open && args.specify_upper_bound) ? &query : NULL;
	if (upper_bound != NULL)
		upper_bound->entrynumber = ULONG_MAX;

	avl_iterator_t iter = avl_get_iterator(&db, lower_bound, upper_bound, !args.reverse);
	char opt;
	for (dbentry_t *cur; (cur = avl_advance(&db, &iter)) != NULL;)
		printf("%s,%s,%d,%d,%d,%lu\n", cur->firstname, cur->lastname, cur->day, cur->month, cur->year, cur->income);

	freedb(entries, nmemb);
}
