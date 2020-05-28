#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>

#include "oob.h"

static int set_default_oob_args(struct oob *oob)
{
	oob->cpus = 1;
}

static int parse_oob_args(int argc, char *const argv[], struct oob *oob)
{
	int c;
	int opt_index;
	int parsed = 0;

	struct option long_options[] = {
		{ "create",		no_argument,		NULL, 'c' },
		{ "verify",		no_argument,		NULL, 'v' },
		{ "repair",		no_argument,		NULL, 'r' },
		{ "destroy",		no_argument,		NULL, 'd' },
		{ "input",		required_argument,	NULL, 'i' },
		{ "cpus",		required_argument,	NULL, 'j' },
		{ "version",		no_argument,		NULL, 'V' },
		{ "",			0,			NULL, '\0'}
	};

	while (1) {
		c = getopt_long(argc, argv, "cvrdi:o:j:V", long_options,
				&opt_index);

		if (c == -1)
			return parsed;
		else if (c == 'c') {
			oob->mode = CREATE;
			//printf("oob: create\n");
		} else if (c == 'v') {
			oob->mode = VERIFY;
			//printf("oob: verify\n");
		} else if (c == 'r') {
			oob->mode = REPAIR;
			//printf("oob: repair\n");
		} else if (c == 'd') {
			oob->mode = DESTROY;
			//printf("oob: destroy\n");
		} else if (c == 'i') {
			int len = strlen(optarg);
			int oob_len = len + strlen(FILE_EXT_STR);

			oob->fin.name = malloc(len);
			oob->fin_oob.name = malloc(oob_len);

			strcpy(oob->fin.name, optarg);
			strcpy(oob->fin_oob.name, optarg);
			strcat(oob->fin_oob.name, FILE_EXT_STR);
			//printf("oob: data file: %s\n", oob->fin.name);
			//printf("oob: oob file: %s\n", oob->fin_oob.name);
		} else if (c == 'j') {
			oob->cpus = strtol(optarg, NULL, 10);
			//printf("oob: cpus: %lu\n", oob->cpus);
		} else if (c == 'V') {
			printf("oob: version\n");
			break;
		} else {
			fprintf(stderr, "oob: unknown option %c\n", c);
			return -EINVAL;
		}
		parsed++;
	}

	return parsed;
}

int main(int argc, char *const argv[])
{
	int ret;
	struct oob oob = { 0 };

	srand(time(NULL));

	/* Init program arguments */
	set_default_oob_args(&oob);
	if (parse_oob_args(argc, argv, &oob) <= 0) {
		fprintf(stderr, "oob: invalid arguments\n");
		return -EINVAL;
	}

	/* init bch */
	oob.bch = bch_init();
	if (!oob.bch)
		return -ENOMEM;
	bch_show_info(oob.bch);

	if (oob.mode == CREATE)
		ret = oob_create(&oob);
	else if (oob.mode == VERIFY)
		ret = oob_verify(&oob);
	else if (oob.mode == REPAIR)
		ret = oob_repair(&oob);
	else if (oob.mode == DESTROY)
		ret = oob_destroy(&oob);

	file_close_all(&oob);

	return ret;
}
