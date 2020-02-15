#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "libbch.h"

static void test(void)
{
	struct libbch *bch;
	uint8_t *data;
	uint8_t *ecc;

	bch = libbch_init();
	if (!bch)
		goto has_error;

	data = malloc(libbch_data_size(bch));
	if (!data)
		goto has_error;

	ecc = malloc(libbch_ecc_size(bch));
	if (!ecc)
		goto has_error;

	memset(data, 0xFF, libbch_data_size(bch));

	libbch_set_buf(bch, data, ecc);

	libbch_show_info(bch);

	libbch_encode(bch);
	//libbch_dump_data(data);
	//libbch_dump_ecc(bch);

	libbch_broke_data_rand(bch);

	libbch_decode(bch);
	libbch_decode_result(bch);
	libbch_dump_err_loc(bch);

	if (bch->err_cnt > 0)
		libbch_correct_data(bch);

	libbch_decode(bch);
	libbch_decode_result(bch);
	libbch_dump_err_loc(bch);

has_error:
	libbch_free(bch);
}


int main(const int argc, const char *argv[])
{
	srand(time(NULL));
	test();

	return 0;
}
