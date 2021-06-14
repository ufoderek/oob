#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <argp.h>
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

static const struct oob_header OOB_HDR =
{
	.magic = "OOB",
	.ver = 1,
	.bch_m = BCH_M,
	.ecc_cap = ECC_CAP,
	.data_bytes = DATA_BYTES,
	.ecc_bytes = ECC_BYTES,
};

static int verify_oob_header(const struct oob_header *gold, const struct oob_header *target)
{
	const struct oob_header *g = gold;
	const struct oob_header *t = target;

	if (strcmp(g->magic, t->magic)) {
		fprintf(stderr, "Invalid magic: %s\n", t->magic);
		return 0;
	}

	if (g->ver != t->ver) {
		fprintf(stderr, "Invalid version: %u\n", t->ver);
		return 0;
	}

	if (g->bch_m != t->bch_m) {
		fprintf(stderr, "Invalid BCH_M: %u\n", t->bch_m);
		return 0;
	}

	if (g->ecc_cap != t->ecc_cap) {
		fprintf(stderr, "Invalid ECC capability: %u\n", t->ecc_cap);
		return 0;
	}

	if (g->data_bytes != t->data_bytes) {
		fprintf(stderr, "Invalid data bytes: %u\n", t->data_bytes);
		return 0;
	}

	if (g->ecc_bytes != t->ecc_bytes) {
		fprintf(stderr, "Invalid ECC bytes: %u\n", t->ecc_bytes);
		return 0;
	}

	return 1;
}

static int file_exist(const char *file_name, int status)
{
	return !access(file_name, status);
}

static int generate_oob(const char *fdata_name, const char *foob_name)
{
	size_t read_bytes;
	size_t left_bytes;
	size_t write_bytes;

	struct oob *oob;

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

	write_bytes = fwrite(&OOB_HDR, 1, sizeof(OOB_HDR), foob);
	if (write_bytes != sizeof(OOB_HDR)) {
		fprintf(stderr, "Error writing OOB header: \n", foob_name);
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

static int verify_oob(const char *fdata_name, const char *foob_name)
{
	size_t read_bytes;
	size_t left_bytes;
	size_t write_bytes;

	struct oob *oob;
	struct oob_header oob_hdr;

	FILE *fdata;
	FILE *foob;

	if (!file_exist(fdata_name, R_OK)) {
		fprintf(stderr, "%s not exist\n", fdata_name);
		exit(EXIT_FAILURE);
	}

	if (!file_exist(foob_name, R_OK)) {
		fprintf(stderr, "%s exist\n", foob_name);
		exit(EXIT_FAILURE);
	}

	fdata = fopen(fdata_name, "rb");
	if (!fdata) {
		fprintf(stderr, "Error opening %s: %s\n", fdata_name, strerror(errno));
		exit(EXIT_FAILURE);
	}

	foob = fopen(foob_name, "rb");
	if (!foob) {
		fprintf(stderr, "Error opening %s: %s\n", foob_name, strerror(errno));
		exit(EXIT_FAILURE);
	}

	oob = oob_init();
	oob_info(oob);

	read_bytes = fread(&oob_hdr, 1, sizeof(oob_hdr), foob);
	if (read_bytes != sizeof(oob_hdr)) {
		fprintf(stderr, "Error reading OOB header: %s\n", foob_name);
		exit(EXIT_FAILURE);
	}

	if (!verify_oob_header(&OOB_HDR, &oob_hdr)) {
		fprintf(stderr, "Invalid OOB header: %s\n", foob_name);
		exit(EXIT_FAILURE);
	}

	while (!feof(fdata)) {

		oob_reinit(oob);

		read_bytes = fread(oob->data, 1, DATA_BYTES, fdata);
		left_bytes = DATA_BYTES - read_bytes;
		if (left_bytes)
			memset(oob->data + read_bytes, 0, left_bytes);


		read_bytes = fread(oob->ecc, 1, ECC_BYTES, foob);
		left_bytes = ECC_BYTES - read_bytes;
		if (left_bytes)
			memset(oob->ecc + read_bytes, 0, left_bytes);

		oob_decode(oob);
		if (oob->errcnt == -EBADMSG) {
			fprintf(stderr, "Decode failed\n");
			exit(EXIT_FAILURE);
		} else if (oob->errcnt == -EINVAL) {
			fprintf(stderr, "Decode with invalid parameters\n");
			exit(EXIT_FAILURE);
		}
		//pr_debug("errcnt = %d\n", oob->errcnt);
	}
	fclose(fdata);
	fclose(foob);
	oob_free(oob);
	return 0;
}
static int correct_oob(const char *fdata_name, const char *foob_name)
{
	return 0;
}

int main(int argc, const char *argv[])
{
	int i;
	int ret;
	struct oob *oob;

	if (argc != 4) {
		fprintf(stderr, "Invalid arguments\n");
		exit(EXIT_FAILURE);
	}

	if (!strcmp(argv[1], "generate")) {
		ret = generate_oob(argv[2], argv[3]);
	} else if (!strcmp(argv[1], "verify")) {
		ret = verify_oob(argv[2], argv[3]);
	} else if (!strcmp(argv[1], "corret")) {
		ret = correct_oob(argv[2], argv[3]);
	} else {
		fprintf(stderr, "Invalid arguments: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

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
