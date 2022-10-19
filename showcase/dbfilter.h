#include "avl.h"

/* comparator functions for the different fields defined elsewhere to avoid cluttering */
int namecmp(const void *i1, const void *i2);
int surnamecmp(const void *i1, const void *i2);
int daycmp(const void *i1, const void *i2);
int monthcmp(const void *i1, const void *i2);
int yearcmp(const void *i1, const void *i2);
int incomecmp(const void *i1, const void *i2);

/* database entry type */
typedef struct dbentry {
	char *firstname;
	char *lastname;
	int day;
	int month;
	int year;
	unsigned long income;
	unsigned long entrynumber;
	avl_node_t avl_node;
} dbentry_t;

/* type representing command line options  */
typedef struct args {
	bool reverse;
	bool specify_upper_bound;
	bool half_open;
} args_t;

/* usage and help messages */
#define USAGEMSG                                                               \
  "USAGE:\n"                                                                   \
  "        dbfilter [OPTION] [PARAMETER]\n"

#define HELPMSG                                                                \
  USAGEMSG                                                                     \
  "\n"                                                                         \
  "DESCRIPTION:\n"                                                             \
  "        dbfilter reads from stdin a .csv file with entries in the\n"        \
  "        following format:\n"                                                \
  "\n"                                                                         \
  "        NAME,SURNAME,DAY_OF_BIRTH,MONTH_OF_BIRTH,YEAR_OF_BIRTH,INCOME\n"    \
  "        [a-z]+ [a-z]+ 1..31       1..12          (-)YYYY       0..2^32-1\n" \
  "        daniel,vondrak,6,7,1979,73000\n"                                    \
  "\n"                                                                         \
  "        The output are stable sorted entries matching the query.\n"         \
  "\n"                                                                         \
  "OPTIONS:\n"                                                                 \
  "        -n\n"                                                               \
  "                filter by first name\n"                                     \
  "        -s\n"                                                               \
  "                filter by surname\n"                                        \
  "        -d\n"                                                               \
  "                filter by day of birth\n"                                   \
  "        -m\n"                                                               \
  "                filter by month of birth\n"                                 \
  "        -y\n"                                                               \
  "                filter by year of birth\n"                                  \
  "        -i\n"                                                               \
  "                filter by income\n"                                         \
  "        -r\n"                                                               \
  "                reverse order of output\n"                                  \
  "        -o\n"                                                               \
  "                use a half open interval instead of exact match\n"          \
  "        -u\n"                                                               \
  "                together with -o uses one of the filter options as the\n"   \
  "                upper bound of the half open interval instead of the\n"     \
  "                lower bound which is the default\n"                         \
  "        -h\n"                                                               \
  "                show this help message\n"                                   \
  "\n"                                                                         \
  "EXAMPLES:\n"                                                                \
  "        to get all Ivos: cat db.csv | dbfilter -n ivo\n"                    \
  "        to get all Ivos born before 1970:\n"                                \
  "            cat db.csv | dbfilter -n ivo | dbfilter -ouy 1970\n"
