#ifndef _LIBBCH_H_
#define _LIBBCH_H_

#include <stdint.h>
#include "bch.h"

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define CONST_M (CONFIG_BCH_CONST_M)
#define CONST_T (CONFIG_BCH_CONST_T)
#define ECC_CAP (CONST_T)

struct libbch {
	struct bch_control *ctrl;
	uint8_t *data;
	uint8_t *ecc;
	int err_cnt;
	unsigned int err_loc[ECC_CAP];
};

static inline void checked_free(void *ptr)
{
	if (ptr)
		free(ptr);
}

static inline unsigned int libbch_ecc_cap(struct libbch *bch)
{
	return CONST_T;
}

static inline unsigned int libbch_ecc_bits(struct libbch *bch)
{
	return bch->ctrl->ecc_bits;
}

static inline unsigned int libbch_ecc_size(struct libbch *bch)
{
	return bch->ctrl->ecc_bytes;
}

static inline const unsigned int libbch_word_bits(struct libbch *bch)
{
	return ((1 << (CONST_M)) - 1);
}

static inline const unsigned int libbch_data_size(struct libbch *bch)
{
	return DATA_BYTES;
}

void libbch_dump_data(struct libbch *bch);
void libbch_dump_ecc(struct libbch *bch);
void libbch_dump_err_loc(struct libbch *bch);

void libbch_show_info(struct libbch *bch);
void libbch_decode_result(struct libbch *bch);

struct libbch *libbch_init(void);
void libbch_set_buf(struct libbch *bch, uint8_t *data, uint8_t *ecc);
void libbch_encode(struct libbch *bch);
void libbch_decode(struct libbch *bch);
void libbch_broke_data_rand(struct libbch *bch);
void libbch_correct_data(struct libbch *bch);
void libbch_free(struct libbch *bch);

#endif /* _LIBBCH_H_ */
