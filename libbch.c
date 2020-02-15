#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libbch.h"

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

void libbch_dump_data(struct libbch *bch)
{
	printf("libbch: dump data:\n");
	dump_buf(bch->data, libbch_data_size(bch));
}

void libbch_dump_ecc(struct libbch *bch)
{
	printf("libbch: dump ecc:\n");
	dump_buf(bch->ecc, libbch_ecc_size(bch));
}

void libbch_dump_err_loc(struct libbch *bch)
{
	int i;
	int i_mod_8;
	int byte_n;
	int bit_n;

	printf("libbch: error locations:\n");
	for (i = 0; i < libbch_ecc_cap(bch); i++) {
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

void libbch_show_info(struct libbch *bch)
{
	unsigned int word_bits = libbch_word_bits(bch);
	unsigned int data_bits = word_bits - libbch_ecc_bits(bch);

	printf("BCH(%u, %u)\n", word_bits, data_bits);
	printf("Data bytes: %u (max: %u)\n", libbch_data_size(bch),
	       DIV_ROUND_UP(data_bits, 8));
	printf("ECC bytes: %u\n", libbch_ecc_size(bch));
	printf("ECC capability: %u\n", libbch_ecc_cap(bch));
}

void libbch_decode_result(struct libbch *bch)
{
	int err_cnt = bch->err_cnt;

	if (err_cnt == -EBADMSG)
		printf("libbch: decoding fail\n");
	else if (err_cnt == -EINVAL)
		printf("libbch: wrong paremeters\n");
	else if (err_cnt >= 0)
		printf("libbch: bit flips detected: %d\n", err_cnt);
	else
		printf("libbch: unknown errors\n");
}

struct libbch *libbch_init(void)
{
	struct libbch *bch;

	bch = malloc(sizeof(*bch));
	if (!bch)
		goto has_error;

	bch->ctrl = init_bch(CONST_M, CONST_T, 0);
	if (!bch->ctrl)
		goto has_error;

	return bch;

has_error:
	libbch_free(bch);
	return NULL;
}

void libbch_set_buf(struct libbch *bch, uint8_t *data, uint8_t *ecc)
{
	bch->data = data;
	bch->ecc = ecc;
}

void libbch_encode(struct libbch *bch)
{
	memset(bch->ecc, 0, libbch_ecc_size(bch));
	encode_bch(bch->ctrl, bch->data, libbch_data_size(bch), bch->ecc);
}

void libbch_decode(struct libbch *bch)
{
	memset(bch->err_loc, 0, sizeof(bch->err_loc));
	bch->err_cnt = decode_bch(bch->ctrl, bch->data, libbch_data_size(bch),
				  bch->ecc, NULL, NULL, bch->err_loc);
}

void libbch_broke_data_rand(struct libbch *bch)
{
	int i;
	unsigned int err_loc;
	int byte_n;
	int bit_n;
	unsigned int data_size = libbch_data_size(bch);
	unsigned int total_bits = data_size * 8 + libbch_ecc_bits(bch);
	unsigned int ecc_pos = 8 * data_size;
	int err_cnt = rand() % (libbch_ecc_cap(bch) + 1);

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

	printf("libbch: %d bits flipped\n", err_cnt);
}

void libbch_correct_data(struct libbch *bch)
{
	int i;
	int byte_n;
	int bit_n;
	unsigned int data_size = libbch_data_size(bch);
	unsigned int ecc_pos = 8 * data_size;
	unsigned int *err_loc = bch->err_loc;

	printf("libbch: correct bits:\n");
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

void libbch_free(struct libbch *bch)
{
	checked_free(bch->ecc);
	checked_free(bch->data);
	if (bch->ctrl)
		free_bch(bch->ctrl);
	checked_free(bch);
}
