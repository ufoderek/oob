#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "oob.h"

#define pr_debug printf

struct oob_header
{
	char magic[4];
	uint8_t ver;
	uint8_t bch_m;
	uint16_t ecc_cap;
	uint16_t data_bytes;
	uint16_t ecc_bytes;
};

static const struct oob_header oob_header =
{
	.magic = "OOB",
	.ver = 1,
	.bch_m = BCH_M,
	.ecc_cap = ECC_CAP,
	.data_bytes = DATA_BYTES,
	.ecc_bytes = ECC_BYTES,
};

static int file_exist(char *file_name, int status)
{
	return !access(file_name, status);
}

static int generate_oob(void)
{
	size_t read_bytes;
	size_t left_bytes;
	size_t write_bytes;

	struct oob *oob;

	char *fdata_name = "/home/ufoderek/wk/test.tar.xz";
	char *foob_name = "/home/ufoderek/wk/test.tar.xz.oob";
	FILE *fdata;
	FILE *foob;

	if (!file_exist(fdata_name, R_OK)) {
		fprintf(stderr, "%s not exist\n", fdata_name);
		exit(EXIT_FAILURE);
	}

	if (file_exist(foob_name, F_OK)) {
		fprintf(stderr, "%s exist\n", foob_name);
		exit(EXIT_FAILURE);
	}

	fdata = fopen(fdata_name, "rb");
	if (!fdata) {
		fprintf(stderr, "Error opening %s: %s\n", fdata_name, strerror(errno));
		exit(EXIT_FAILURE);
	}

	foob = fopen(foob_name, "wb");
	if (!foob) {
		fprintf(stderr, "Error opening %s: %s\n", foob_name, strerror(errno));
		exit(EXIT_FAILURE);
	}

	oob = oob_init();
	oob_info(oob);

	write_bytes = fwrite(&oob_header, 1, sizeof(oob_header), foob);
	if (write_bytes != sizeof(oob_header)) {
		fprintf(stderr, "Error writing OOB header to %s\n", foob_name);
		exit(EXIT_FAILURE);
	}

	while (!feof(fdata)) {

		oob_reinit(oob);

		read_bytes = fread(oob->data, 1, DATA_BYTES, fdata);
		left_bytes = DATA_BYTES - read_bytes;
		if (left_bytes)
			memset(oob->data + read_bytes, 0, left_bytes);

		oob_encode(oob);

		write_bytes = fwrite(oob->ecc, 1, ECC_BYTES, foob);
		if (write_bytes != ECC_BYTES) {
			fprintf(stderr, "Error writing %s\n", foob_name);
			exit(EXIT_FAILURE);
		}
	}
	fclose(fdata);
	fclose(foob);
	oob_free(oob);
	return 0;
}

int main(int argc, const char *argv[])
{
	int i;
	int ret;
	struct oob *oob;

	ret = generate_oob();
	return ret;

	/*
	oob = oob_init();
	oob_reinit(oob);
	oob_info(oob);

	memset(oob->data, 1, sizeof(oob->data));
	oob_encode(oob);

	//oob_dump_ecc(oob);

	for (i=0;i<(128/8);i++) {
		oob_flip_data(oob, 3+i);
	}

	oob_flip_data(oob, 0);
	//oob_flip_ecc(oob, 6);

	oob_decode(oob);
	if (oob->errcnt == -EBADMSG) {
		fprintf(stderr, "Decode failed\n");
		return -EBADMSG;
	} else if (oob->errcnt == -EINVAL) {
		fprintf(stderr, "Decode with invalid parameters\n");
		return -EINVAL;
	}
	pr_debug("errcnt = %d\n", oob->errcnt);

	oob_correct(oob);
	oob_free(oob);
	*/
	return 0;
}
