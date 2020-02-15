#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bch.h"

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

void bch_dump_data(struct bch *bch)
{
	printf("bch: dump data:\n");
	dump_buf(bch->data, bch_data_size(bch));
}

void bch_dump_ecc(struct bch *bch)
{
	printf("bch: dump ecc:\n");
	dump_buf(bch->ecc, bch_ecc_size(bch));
}

void bch_dump_err_loc(struct bch *bch)
{
	int i;
	int i_mod_8;
	int byte_n;
	int bit_n;

	printf("bch: error locations:\n");
	for (i = 0; i < bch_ecc_cap(bch); i++) {
		i_mod_8 = i % 8;
		if (i && (i_mod_8 == 0))
			printf("\n");
		if (i_mod_8 == 0)
			printf("%2d: ", i / 8 + 1);
		printf("[%3d][%2d]  ", bch->err_loc[i] / 8, bch->err_loc[i] % 8);
	}
	if (i_mod_8)
		printf("\n");
}

void bch_show_info(struct bch *bch)
{
	unsigned int word_bits = bch_word_bits(bch);
	unsigned int data_bits = word_bits - bch_ecc_bits(bch);

	printf("BCH(%u, %u)\n", word_bits, data_bits);
	printf("Data bytes: %u (max: %u)\n", bch_data_size(bch),
	       DIV_ROUND_UP(data_bits, 8));
	printf("ECC bytes: %u\n", bch_ecc_size(bch));
	printf("ECC capability: %u\n", bch_ecc_cap(bch));
}

void bch_decode_result(struct bch *bch)
{
	int err_cnt = bch->err_cnt;

	if (err_cnt == -EBADMSG)
		printf("bch: decoding fail\n");
	else if (err_cnt == -EINVAL)
		printf("bch: wrong paremeters\n");
	else if (err_cnt >= 0)
		printf("bch: bit flips detected: %d\n", err_cnt);
	else
		printf("bch: unknown errors\n");
}

struct bch *bch_init(void)
{
	struct bch *bch;

	bch = malloc(sizeof(*bch));
	if (!bch)
		goto has_error;

	bch->ctrl = init_bch(CONST_M, CONST_T, 0);
	if (!bch->ctrl)
		goto has_error;

	return bch;

has_error:
	bch_free(bch);
	return NULL;
}

void bch_set_buf(struct bch *bch, uint8_t *data, uint8_t *ecc)
{
	bch->data = data;
	bch->ecc = ecc;
}

void bch_encode(struct bch *bch)
{
	memset(bch->ecc, 0, bch_ecc_size(bch));
	encode_bch(bch->ctrl, bch->data, bch_data_size(bch), bch->ecc);
}

void bch_decode(struct bch *bch)
{
	memset(bch->err_loc, 0, sizeof(bch->err_loc));
	bch->err_cnt = decode_bch(bch->ctrl, bch->data, bch_data_size(bch),
				  bch->ecc, NULL, NULL, bch->err_loc);
}

void bch_broke_data_rand(struct bch *bch)
{
	int i;
	unsigned int err_loc;
	int byte_n;
	int bit_n;
	unsigned int data_size = bch_data_size(bch);
	unsigned int total_bits = data_size * 8 + bch_ecc_bits(bch);
	unsigned int ecc_pos = 8 * data_size;
	int err_cnt = rand() % (bch_ecc_cap(bch) + 1);

	for (i = 0; i < err_cnt; i++) {
		err_loc = rand() % (total_bits + 1);
		byte_n = err_loc / 8;
		bit_n = err_loc % 8;

		if (err_loc < ecc_pos)
			bch->data[byte_n] ^= 1 << bit_n;
		else
			bch->ecc[byte_n - data_size] ^= 1 << bit_n;

		//printf("flip: word[%03d][%02d]\n", byte_n, bit_n);
	}

	printf("bch: %d bits flipped\n", err_cnt);
}

void bch_correct_data(struct bch *bch)
{
	int i;
	int byte_n;
	int bit_n;
	unsigned int data_size = bch_data_size(bch);
	unsigned int ecc_pos = 8 * data_size;
	unsigned int *err_loc = bch->err_loc;

	printf("bch: correct bits:\n");
	for (i = 0; i < bch->err_cnt; i++) {
		byte_n = err_loc[i] / 8;
		bit_n = err_loc[i] % 8;

		if (err_loc[i] < ecc_pos)
			bch->data[byte_n] ^= 1 << bit_n;
		else
			bch->ecc[byte_n - data_size] ^= 1 << bit_n;

		if (i && (!(i % 8)))
			printf("\n");
		if (!(i % 8))
			printf("%2d: ", i / 8 + 1);
		printf("[%03d][%02d]  ", byte_n, bit_n);
	}
	if (i % 8)
		printf("\n");
}

void bch_free(struct bch *bch)
{
	checked_free(bch->ecc);
	checked_free(bch->data);
	if (bch->ctrl)
		free_bch(bch->ctrl);
	checked_free(bch);
}
