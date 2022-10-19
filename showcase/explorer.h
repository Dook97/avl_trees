#include "avl.h"

int namecmp(const void *i1, const void *i2);
int surnamecmp(const void *i1, const void *i2);
int daycmp(const void *i1, const void *i2);
int monthcmp(const void *i1, const void *i2);
int yearcmp(const void *i1, const void *i2);
int incomecmp(const void *i1, const void *i2);

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

typedef struct args {
	bool reverse;
	bool specify_upper_bound;
	bool half_open;
} args_t;

#define USAGEMSG \
	"USAGE:\n" \
	"        dbfilter [OPTION] [PARAMETER]\n"

#define HELPMSG \
	USAGEMSG\
	"\n" \
	"DESCRIPTION:\n" \
	"        dbfilter reads from stdin a .csv file with entries in the following format:\n" \
	"\n" \
	"        FIRST_NAME,SURNAME,DAY_OF_BIRTH,MONTH_OF_BIRTH,YEAR_OF_BIRTH,INCOME\n" \
	"        [a-z]+     [a-z]+  1..31        1..12          (-)YYYY       0..2^32-1\n" \
	"        daniel,vondrak,6,7,1979,73000\n" \
	"\n" \
	"        ...and filters and sorts them based on the provided option (singular).\n" \
	"\n" \
	"        Each option can be invoked either as a literal:\n" \
	"                dbfilter -d 10\n" \
	"\n" \
	"        Or as a half open range:\n" \
	"                dbfilter -d 10.\n" \
	"                dbfilter -d .10\n" \
	"\n" \
	"OPTIONS:\n" \
	"        -f\n" \
	"                filter by first name\n" \
	"        -s\n" \
	"                filter by surname\n" \
	"        -d\n" \
	"                filter by day of birth\n" \
	"        -m\n" \
	"                filter by month of birth\n" \
	"        -y\n" \
	"                filter by year of birth\n" \
	"        -i\n" \
	"                filter by income\n" \
	"        -r\n" \
	"                reverse order of output \n" \
	"        -h\n" \
	"                show this help message"
