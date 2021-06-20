#include <stdio.h>
#include <string.h>
#include <linux/bch.h>

#include "oob.h"

#ifdef DEBUG
#define oob_dbg printf
#else
#define oob_dbg(...) do { } while (0)
#endif

struct oob *oob_init(void)
{
	struct oob *oob = calloc(1, sizeof(*oob));

	if (!oob) {
		fprintf(stderr, "oob_init: no sufficient memory...\n");
		return NULL;
	}

	oob->data = oob->_data;
	oob->ecc = oob->_ecc;
	oob->bch = bch_init(BCH_M, ECC_CAP, 0, 0);
	return oob;
}

void oob_reinit(struct oob *oob)
{
	struct bch_control *bch = oob->bch;

	memset(oob, 0, sizeof(*oob));
	oob->data = oob->_data;
	oob->ecc = oob->_ecc;
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

	oob_dbg("m = %u n = %u t = %u\n", bch->m, bch->n, bch->t);
	oob_dbg("ecc_bits = %u ecc_bytes = %u\n", bch->ecc_bits, ECC_BYTES);
	oob_dbg("data_bytes = %u\n", DATA_BYTES);
}

void oob_dump_ecc(struct oob *oob)
{
	int i;
	uint8_t *ecc = oob->ecc;

	for (i = 0; i < ECC_BYTES; i+=8) {
		oob_dbg("%u:\t0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
			i, ecc[i], ecc[i+1], ecc[i+2], ecc[i+3],
			ecc[i+4], ecc[i+5], ecc[i+6], ecc[i+7]);
	}
}

void oob_encode(struct oob *oob)
{
	bch_encode(oob->bch, oob->data, sizeof(oob->_data), oob->ecc);
}

void oob_decode(struct oob *oob)
{
	oob->errcnt = bch_decode(oob->bch, oob->data, sizeof(oob->_data), oob->ecc, 0, 0, oob->errloc);
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
			//oob_dbg("correct: data[%04d][%d]=0x%02X\n", byte_n, bit_n, oob->data[byte_n]);
		} else {
			byte_n -= DATA_BYTES;
			oob->ecc[byte_n] ^= 1 << bit_n;
			//oob_dbg("correct:  ecc[%04d][%d]=0x%02X\n", byte_n, bit_n, oob->ecc[byte_n]);
		}
	}
}

void oob_flip_data(struct oob *oob, int i)
{
	uint8_t orig_val = oob->data[i];
	uint8_t *data = &oob->data[i];

	*data = ~orig_val;
	oob_dbg("flip: data[%04d]: 0x%02X --> 0x%02X\n", i, orig_val, *data);
}

void oob_flip_ecc(struct oob *oob, int i)
{
	uint8_t orig_val = oob->ecc[i];
	uint8_t *ecc = &oob->ecc[i];

	*ecc = ~orig_val;
	oob_dbg("flip:  ecc[%04d]: 0x%02X --> 0x%02X\n", i, orig_val, *ecc);
}

void oob_data(struct oob *oob, int i)
{
	oob_dbg("data[%04d]: 0x%02X\n", i, oob->data[i]);
}

void oob_ecc(struct oob *oob, int i)
{
	oob_dbg("ecc[%04d]:  0x%02X\n", i, oob->ecc[i]);
}
