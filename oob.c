#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <linux/bch.h>
#include <linux/kernel.h>

#define BCH_M CONFIG_BCH_CONST_M
#define ECC_CAP CONFIG_BCH_CONST_T
#define DATA_BYTES (CONFIG_BCH_CONST_K / 8)
#define ECC_BYTES DIV_ROUND_UP(BCH_M * ECC_CAP, 8)

#define pr_debug printf

struct oob
{
	uint8_t data[DATA_BYTES];
	uint8_t ecc[ECC_BYTES];
	unsigned int errloc[ECC_CAP];
	int errcnt;
	struct bch_control *bch;
};

struct oob *oob_init(void)
{
	struct oob *oob = calloc(1, sizeof(*oob));

	if (!oob) {
		fprintf(stderr, "No sufficient memory...\n");
		return NULL;
	}

	oob->bch = bch_init(BCH_M, ECC_CAP, 0, 0);
	return oob;
}

void oob_reinit(struct oob *oob)
{
	struct bch_control *bch = oob->bch;
	memset(oob, 0, sizeof(*oob));
	oob->bch = bch;
}

void oob_free(struct oob *oob)
{
	if (oob) {
		bch_free(oob->bch);
		free(oob);
	}
}

void oob_info(struct oob *oob)
{
	struct bch_control *bch = oob->bch;

	pr_debug("m = %u n = %u t = %u\n", bch->m, bch->n, bch->t);
	pr_debug("ecc_bits = %u ecc_bytes = %u\n", bch->ecc_bits, ECC_BYTES);
	pr_debug("data_bytes = %u\n", DATA_BYTES);
}

void oob_dump_ecc(struct oob *oob)
{
	int i;
	uint8_t *ecc = oob->ecc;

	for (i = 0; i < ECC_BYTES; i+=8) {
		printf("%u:\t0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
			i, ecc[i], ecc[i+1], ecc[i+2], ecc[i+3],
			ecc[i+4], ecc[i+5], ecc[i+6], ecc[i+7]);
	}
}

void oob_encode(struct oob *oob)
{
	bch_encode(oob->bch, oob->data, sizeof(oob->data), oob->ecc);
}

void oob_decode(struct oob *oob)
{
	oob->errcnt = bch_decode(oob->bch, oob->data, sizeof(oob->data), oob->ecc, 0, 0, oob->errloc);
}

void oob_correct(struct oob *oob)
{
	int i;
	
	for (i = 0; i < oob->errcnt; i++) {
		int *errloc = oob->errloc;
		int byte_n = errloc[i] / 8;
		int bit_n = errloc[i] % 8;
		if (errloc[i] < (DATA_BYTES * 8)) {
			oob->data[byte_n] ^= 1 << bit_n;
			pr_debug("correct: data[%04d][%d]=0x%02X\n", byte_n, bit_n, oob->data[byte_n]);
		} else {
			byte_n -= DATA_BYTES;
			oob->ecc[byte_n] ^= 1 << bit_n;
			pr_debug("correct:  ecc[%04d][%d]=0x%02X\n", byte_n, bit_n, oob->ecc[byte_n]);
		}
	}
}

void oob_flip_data(struct oob *oob, int i)
{
	uint8_t orig_val = oob->data[i];
	uint8_t *data = &oob->data[i];

	*data = ~orig_val;
	pr_debug("flip: data[%04d]: 0x%02X --> 0x%02X\n", i, orig_val, *data);
}

void oob_flip_ecc(struct oob *oob, int i)
{
	uint8_t orig_val = oob->ecc[i];
	uint8_t *ecc = &oob->ecc[i];

	*ecc = ~orig_val;
	pr_debug("flip:  ecc[%04d]: 0x%02X --> 0x%02X\n", i, orig_val, *ecc);
}

int main(int argc, const char *argv[])
{
	struct oob *oob;

	oob = oob_init();
	oob_reinit(oob);
	oob_info(oob);

	memset(oob->data, 1, sizeof(oob->data));
	oob_encode(oob);

	oob_dump_ecc(oob);

	oob_flip_data(oob, 3);
	oob_flip_ecc(oob, 6);

	oob_decode(oob);
	if (oob->errcnt == -EBADMSG) {
		fprintf(stderr, "Decode failed\n");
		return 0;
	} else if (oob->errcnt == -EINVAL) {
		fprintf(stderr, "Decode with invalid parameters\n");
		return 0;
	}
	pr_debug("errcnt = %d\n", oob->errcnt);

	oob_correct(oob);
	oob_free(oob);
	return 0;
}
