#ifndef _USER_BCH_H_
#define _USER_BCH_H_

#include <stdint.h>
#include <linux/bch.h>

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define CONST_M (CONFIG_BCH_CONST_M)
#define CONST_T (CONFIG_BCH_CONST_T)
#define ECC_CAP (CONST_T)

struct bch {
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

static inline unsigned int bch_ecc_cap(struct bch *bch)
{
	return CONST_T;
}

static inline unsigned int bch_ecc_bits(struct bch *bch)
{
	return bch->ctrl->ecc_bits;
}

static inline unsigned int bch_ecc_size(struct bch *bch)
{
	return bch->ctrl->ecc_bytes;
}

static inline const unsigned int bch_word_bits(struct bch *bch)
{
	return ((1 << (CONST_M)) - 1);
}

static inline const unsigned int bch_data_size(struct bch *bch)
{
	return DATA_BYTES;
}

void bch_dump_data(struct bch *bch);
void bch_dump_ecc(struct bch *bch);
void bch_dump_err_loc(struct bch *bch);

void bch_show_info(struct bch *bch);
void bch_decode_result(struct bch *bch);

struct bch *bch_init(void);
void bch_set_buf(struct bch *bch, uint8_t *data, uint8_t *ecc);
void bch_encode(struct bch *bch);
void bch_decode(struct bch *bch);
void bch_broke_data_rand(struct bch *bch);
void bch_correct_data(struct bch *bch);
void bch_free(struct bch *bch);

#endif /* _USER_BCH_H_ */
