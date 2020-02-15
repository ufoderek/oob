#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>
#include "bch.h"

enum oob_mode {
	CREATE,
	VERIFY,
	REPAIR,
};

struct oob_data {
	enum oob_mode mode;

	char *data_name;
	char *oob_name;
	char *rdata_name;
	char *roob_name;

	FILE *fp_data;
	FILE *fp_oob;
	FILE *fp_rdata;
	FILE *fp_roob;

	uint64_t data_size;
	uint64_t oob_size;

	uint8_t *data;
	uint8_t *oob;
	uint8_t *rdata;
	uint8_t *roob;
};

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

static int parse_oob_args(int argc, char * const argv[], struct oob_data *oob)
{
	int c;
	int opt_index;
	int parsed = 0;

	struct option long_options[] = {
		{ "create",		no_argument,		NULL, 'c' },
		{ "verify",		no_argument,		NULL, 'v' },
		{ "repair",		no_argument,		NULL, 'r' },
		{ "data",		required_argument,	NULL, 'd' },
		{ "oob",		required_argument,	NULL, 'o' },
		{ "repaired-data",	required_argument,	NULL, 'D' },
		{ "repaired-oob",	required_argument,	NULL, 'O' },
		{ "version",		no_argument,		NULL, 'V' },
		{ "",			0,			NULL, '\0'}
	};

	while (1) {
		c = getopt_long(argc, argv, "cvfd:o:D:O:V", long_options,
				&opt_index);

		if (c == -1) {
			return parsed;
		} else if (c == 'c') {
			oob->mode = CREATE;
			//printf("oob: create\n");
		} else if (c == 'v') {
			oob->mode = VERIFY;
			//printf("oob: verify\n");
		} else if (c == 'r') {
			oob->mode = REPAIR;
			//printf("oob: repair\n");
		} else if (c == 'd') {
			oob->data_name = optarg;
			//printf("oob: data file: %s\n", oob->data_name);
		} else if (c == 'o') {
			oob->oob_name = optarg;
			//printf("oob: oob file: %s\n", oob->oob_name);
		} else if (c == 'D') {
			oob->rdata_name = optarg;
			//printf("oob: fixed data file: %s\n", oob->rdata_name);
		} else if (c == 'O') {
			oob->roob_name = optarg;
			//printf("oob: fixed oob file: %s\n", oob->roob_name);
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

static int prepare_orig_files(struct oob_data *oob, int write_oob)
{
	oob->fp_data = fopen(oob->data_name, "rb");
	if (!oob->fp_data) {
		fprintf(stderr, "oob: error open %s\n", oob->data_name);
		return -errno;
	}

	if (write_oob)
		oob->fp_oob = fopen(oob->oob_name, "wb");
	else
		oob->fp_oob = fopen(oob->oob_name, "rb");

	if (!oob->fp_oob) {
		fprintf(stderr, "oob: error open %s\n", oob->oob_name);
		return -errno;
	}

	oob->data_size = (uint64_t)lseek(fileno(oob->fp_data), 0, SEEK_END);
	oob->oob_size = (uint64_t)lseek(fileno(oob->fp_oob), 0, SEEK_END);

	return 0;
}

static int prepare_repaired_files(struct oob_data *oob)
{
	oob->fp_rdata = fopen(oob->rdata_name, "wb");
	if (!oob->fp_rdata) {
		fprintf(stderr, "oob: error open %s\n", oob->rdata_name);
		return -errno;
	}

	oob->fp_roob = fopen(oob->roob_name, "wb");
	if (!oob->fp_roob) {
		fprintf(stderr, "oob: error open %s\n", oob->roob_name);
		return -errno;
	}

	return 0;
}

int oob_create(struct oob_data *oob)
{
	int ret;

	ret = prepare_orig_files(oob, 1);
	if (ret)
		return ret;

	ret = prepare_repaired_files(oob);
	if (ret)
		return ret;
}

int main(int argc, char * const argv[])
{
	struct oob_data oob = { 0 };

	srand(time(NULL));

	if (parse_oob_args(argc, argv, &oob) <= 0)
	{
		fprintf(stderr, "oob: invalid arguments\n");
		return -EINVAL;
	}

	if (oob.mode == CREATE) {
		return oob_create(&oob);
	} else if (oob.mode == VERIFY) {
	} else if (oob.mode == REPAIR) {
	}

	test();

	return 0;
}
