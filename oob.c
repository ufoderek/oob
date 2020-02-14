#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "bch.h"

#define CONST_M CONFIG_BCH_CONST_M
#define CONST_T CONFIG_BCH_CONST_T

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

static void show_bch_info(struct bch_control *bch)
{
	const uint32_t word_bits = (1 << CONST_M) - 1;
	uint32_t data_bits = word_bits - bch->ecc_bits;

	printf("max_data_bytes: %u\n", DIV_ROUND_UP(data_bits, 8));
	printf("ecc_bytes: %u\n", bch->ecc_bytes);
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
	printf("Data:\n");
	dump_buf(data, DATA_BYTES);
}

static void dump_ecc(struct bch_control *bch)
{
	printf("ECC:\n");
	dump_buf((uint8_t*)bch->ecc_buf, bch->ecc_bytes);
}

static void test_encode(struct bch_control *bch)
{
	const unsigned int buf_size = DATA_BYTES;
	uint8_t *data;

	data = malloc(buf_size);
	if (!data)
		goto has_error;

	memset(data, 0xFF, buf_size);

	encode_bch(bch, data, buf_size, NULL);
	dump_data(data);
	dump_ecc(bch);

has_error:
	if (data)
		free(data);
	if (bch)
		free(bch);
}


int main(const int argc, const char *argv[])
{
	struct bch_control *bch;

	bch = init_bch(CONST_M, CONST_T, 0);
	show_bch_info(bch);


	test_encode(bch);

	return 0;
}
