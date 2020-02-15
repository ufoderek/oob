#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>

#include "oob.h"

static void test(void)
{
	struct bch *bch;
	uint8_t *data;
	uint8_t *ecc;

	bch = bch_init();
	if (!bch)
		goto has_error;

	data = malloc(bch_data_size(bch));
	if (!data)
		goto has_error;

	ecc = malloc(bch_ecc_size(bch));
	if (!ecc)
		goto has_error;

	memset(data, 0xFF, bch_data_size(bch));

	bch_set_buf(bch, data, ecc);

	bch_show_info(bch);

	bch_encode(bch);
	//bch_dump_data(data);
	//bch_dump_ecc(bch);

	bch_broke_data_rand(bch);

	bch_decode(bch);
	bch_decode_result(bch);
	bch_dump_err_loc(bch);

	if (bch->err_cnt > 0)
		bch_correct_data(bch);

	bch_decode(bch);
	bch_decode_result(bch);
	bch_dump_err_loc(bch);

has_error:
	bch_free(bch);
}

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
		{ "break",		no_argument,		NULL, 'b' },
		{ "data",		required_argument,	NULL, 'd' },
		{ "oob",		required_argument,	NULL, 'o' },
		{ "repaired-data",	required_argument,	NULL, 'D' },
		{ "repaired-oob",	required_argument,	NULL, 'O' },
		{ "cpus",		required_argument,	NULL, 'j' },
		{ "version",		no_argument,		NULL, 'V' },
		{ "",			0,			NULL, '\0'}
	};

	while (1) {
		c = getopt_long(argc, argv, "cvrbd:o:D:O:j:V", long_options,
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
		} else if (c == 'b') {
			oob->mode = BREAK;
			//printf("oob: break\n");
		} else if (c == 'd') {
			oob->file_data.name = optarg;
			//printf("oob: data file: %s\n", oob->data_name);
		} else if (c == 'o') {
			oob->file_oob.name = optarg;
			//printf("oob: oob file: %s\n", oob->oob_name);
		} else if (c == 'D') {
			oob->file_data_r.name = optarg;
			//printf("oob: fixed data file: %s\n", oob->rdata_name);
		} else if (c == 'O') {
			oob->file_oob_r.name = optarg;
			//printf("oob: fixed oob file: %s\n", oob->roob_name);
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
		return oob_create(&oob);
	else if (oob.mode == VERIFY)
		return oob_verify(&oob);
	else if (oob.mode == REPAIR)
		return oob_repair(&oob);
	else if (oob.mode == BREAK)
		return oob_break(&oob);

	file_close_all(&oob);
	//test();

	return 0;
}
