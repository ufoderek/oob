#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "bch.h"

#define CONST_M (CONFIG_BCH_CONST_M)
#define CONST_T (CONFIG_BCH_CONST_T)

#define ECC_CAP (CONST_T)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

static void checked_free(void *ptr)
{
	if (ptr)
		free(ptr);
}

static void show_bch_info(struct bch_control *bch)
{
	const unsigned int word_bits = (1 << CONST_M) - 1;
	const unsigned int data_size = DATA_BYTES;
	unsigned int data_bits = word_bits - bch->ecc_bits;

	printf("BCH(%u, %u)\n", word_bits, data_bits);
	printf("Data bytes: %u (max: %u)\n", data_size, DIV_ROUND_UP(data_bits, 8));
	printf("ECC bytes: %u\n", bch->ecc_bytes);
	printf("ECC capability: %u\n", ECC_CAP);
}

static void dump_buf(uint8_t *buf, unsigned int size)
{
	int i;
	int i_mod_8;

	for (i = 0; i < size; i++) {
		i_mod_8 = i % 8;
		if (i && (i_mod_8 == 0))
			printf("\n");
		if (i_mod_8 == 0)
			printf("%02d: ", i / 8 + 1);
		printf("0x%02X ", buf[i]);
	}
	if (i_mod_8)
		printf("\n");
}

static void dump_data(uint8_t *data)
{
	const unsigned int data_size = DATA_BYTES;

	printf("Data dump:\n");
	dump_buf(data, data_size);
}

static void dump_ecc(struct bch_control *bch)
{
	printf("ECC dump:\n");
	dump_buf((uint8_t*)bch->ecc_buf, bch->ecc_bytes);
}

static void dump_err_loc(unsigned int *err_loc)
{
	int i;
	int i_mod_8;

	printf("Error locations:\n");
	for (i = 0; i < ECC_CAP; i++) {
		i_mod_8 = i % 8;
		if (i && (i_mod_8 == 0))
			printf("\n");
		if (i_mod_8 == 0)
			printf("%2d: ", i / 8 + 1);
		printf("%6d ", err_loc[i]);
	}
	if (i_mod_8)
		printf("\n");
}

static void broke_data(uint8_t *data)
{
	//const unsigned int data_size = DATA_BYTES;

	data[0] = 0;
}

static void test_encode(struct bch_control *bch)
{
	const unsigned int data_size = DATA_BYTES;
	uint8_t *data;
	uint8_t *ecc;
	unsigned int *err_loc;
	int err_cnt;

	if (!bch)
		goto has_error;

	data = malloc(data_size);
	if (!data)
		goto has_error;

	ecc = calloc(1, bch->ecc_bytes);
	if (!ecc)
		goto has_error;

	err_loc = calloc(1, ECC_CAP * sizeof(*err_loc));
	if (!err_loc)
		goto has_error;

	memset(data, 0xFF, data_size);

	encode_bch(bch, data, data_size, ecc);
	//dump_data(data);
	dump_ecc(bch);

	broke_data(data);

	err_cnt = decode_bch(bch, data, data_size, ecc, NULL, NULL, err_loc);

	if (err_cnt == -EBADMSG) {
		printf("decoding fail\n");
	} else if (err_cnt == -EINVAL) {
		printf("wrong paremeters\n");
	} else if (err_cnt >= 0) {
		printf("error count: %d\n", err_cnt);
		dump_err_loc(err_loc);
	} else {
		printf("unknown errors\n");
	}

has_error:
	checked_free(data);
	checked_free(ecc);
	checked_free(err_loc);
	checked_free(bch);
}


int main(const int argc, const char *argv[])
{
	struct bch_control *bch;

	bch = init_bch(CONST_M, CONST_T, 0);
	show_bch_info(bch);


	test_encode(bch);

	return 0;
}
