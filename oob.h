#ifndef __OOB_H__
#define __OOB_H__

#include <stdint.h>
#include <linux/kernel.h>

#define BCH_M		CONFIG_BCH_CONST_M
#define ECC_CAP		CONFIG_BCH_CONST_T
#define DATA_BYTES	(CONFIG_BCH_CONST_K / 8)
#define ECC_BYTES	DIV_ROUND_UP(BCH_M * ECC_CAP, 8)

struct oob
{
	uint8_t data[DATA_BYTES];
	uint8_t ecc[ECC_BYTES];
	unsigned int errloc[ECC_CAP];
	int errcnt;
	struct bch_control *bch;
};

struct oob *oob_init(void);
void oob_reinit(struct oob *oob);
void oob_free(struct oob *oob);
void oob_info(struct oob *oob);
void oob_dump_ecc(struct oob *oob);
void oob_encode(struct oob *oob);
void oob_decode(struct oob *oob);
void oob_correct(struct oob *oob);
void oob_flip_data(struct oob *oob, int i);
void oob_flip_ecc(struct oob *oob, int i);

#endif /* __OOB_H__ */
