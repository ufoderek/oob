#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bch.h"

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


int main(const int argc, const char *argv[])
{
	srand(time(NULL));
	test();

	return 0;
}
