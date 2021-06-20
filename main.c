#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <argp.h>
#include "oob.h"

#ifdef DEBUG
#define oob_dbg printf
#else
#define oob_dbg(...) do { } while (0)
#endif

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

static FILE *fopen_read(const char *file_name)
{
	FILE *fp;

	if (!file_exist(file_name, R_OK)) {
		fprintf(stderr, "%s not exist\n", file_name);
		return NULL;
	}

	fp = fopen(file_name, "rb");
	if (!fp) {
		fprintf(stderr, "Error opening %s: %s\n", file_name, strerror(errno));
	}
	
	return fp;
}

static FILE *fopen_write(const char *file_name)
{
	FILE *fp;

	if (file_exist(file_name, F_OK)) {
		fprintf(stderr, "%s exist\n", file_name);
		return NULL;
	}

	fp = fopen(file_name, "wb");
	if (!fp) {
		fprintf(stderr, "Error opening %s: %s\n", file_name, strerror(errno));
	}
	
	return fp;
}

static int generate_oob(const char *fdata_name, const char *foob_name)
{
	size_t read_bytes;
	size_t left_bytes;
	size_t write_bytes;

	struct oob *oob;

	FILE *fdata;
	FILE *foob;

	fdata = fopen_read(fdata_name);
	if (!fdata)
		exit(EXIT_FAILURE);

	foob = fopen_write(foob_name);
	if (!foob)
		exit(EXIT_FAILURE);

	oob = oob_init();
	oob_info(oob);

	write_bytes = fwrite(&OOB_HDR, 1, sizeof(OOB_HDR), foob);
	if (write_bytes != sizeof(OOB_HDR)) {
		fprintf(stderr, "Error writing OOB header: \n", foob_name);
		exit(EXIT_FAILURE);
	}

	/* generate_oob */
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

static char *str_append(const char *src, const char *append)
{
	char *new_str;
	int len = strlen(src) + strlen(append);

	if (!len)
		return NULL;

	new_str = malloc(len);
	if (!new_str)
		return NULL;

	strcpy(new_str, src);
	strcat(new_str, append);
	return new_str;
}

static int verify_oob(const char *fdata_name, const char *foob_name, int correct)
{
	size_t read_bytes;
	size_t left_bytes;
	size_t write_bytes;

	struct oob *oob;
	struct oob_header oob_hdr;

	char *fdata2_name;
	char *foob2_name;

	FILE *fdata;
	FILE *foob;
	FILE *fdata2;
	FILE *foob2;

	fdata = fopen_read(fdata_name);
	if (!fdata)
		exit(EXIT_FAILURE);

	foob = fopen_read(foob_name);
	if (!foob)
		exit(EXIT_FAILURE);

	if (correct) {
		fdata2_name = str_append(fdata_name, ".fix");
		foob2_name = str_append(foob_name, ".fix");

		fdata2 = fopen_write(fdata2_name);
		if (!fdata2)
			exit(EXIT_FAILURE);

		foob2 = fopen_write(foob2_name);
		if (!foob2)
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
		fprintf(stderr, "Try continuing...\n");
	}
	
	if (correct) {
		write_bytes = fwrite(&OOB_HDR, 1, sizeof(OOB_HDR), foob2);
		if (write_bytes != sizeof(OOB_HDR)) {
			fprintf(stderr, "Error writing OOB header: \n", foob2_name);
			exit(EXIT_FAILURE);
		}
	}

	/* verify_oob */
	while (!feof(fdata)) {
		oob_reinit(oob);

		read_bytes = fread(oob->ecc, 1, ECC_BYTES, foob);
		left_bytes = ECC_BYTES - read_bytes;
		if (left_bytes)
			memset(oob->ecc + read_bytes, 0, left_bytes);

		read_bytes = fread(oob->data, 1, DATA_BYTES, fdata);
		left_bytes = DATA_BYTES - read_bytes;
		if (left_bytes)
			memset(oob->data + read_bytes, 0, left_bytes);

		oob_decode(oob);
		if (oob->errcnt == -EBADMSG) {
			fprintf(stderr, "Decode failed\n");
			exit(EXIT_FAILURE);
		} else if (oob->errcnt == -EINVAL) {
			fprintf(stderr, "Decode with invalid parameters\n");
			exit(EXIT_FAILURE);
		} else if (oob->errcnt > 0) {
			printf("Correctable error count: %d\n", oob->errcnt);
		}
		
		if (correct) {
			oob_correct(oob);

			write_bytes = fwrite(oob->data, 1, read_bytes, fdata2);
			if (write_bytes != read_bytes) {
				fprintf(stderr, "Error writing %s\n", fdata2_name);
				exit(EXIT_FAILURE);
			}

			write_bytes = fwrite(oob->ecc, 1, ECC_BYTES, foob2);
			if (write_bytes != ECC_BYTES) {
				fprintf(stderr, "Error writing %s\n", foob2_name);
				exit(EXIT_FAILURE);
			}
		}
	}
	fclose(fdata);
	fclose(foob);
	oob_free(oob);
	if (correct) {
		fclose(fdata2);
		fclose(foob2);
	}
	return 0;
}

int main(int argc, const char *argv[])
{
	int i;
	int ret;
	struct oob *oob;

	if ((argc == 4) && !strcmp(argv[1], "generate")) {
		ret = generate_oob(argv[2], argv[3]);
	} else if ((argc == 4) && !strcmp(argv[1], "verify")) {
		ret = verify_oob(argv[2], argv[3], 0);
	} else if ((argc == 4) && !strcmp(argv[1], "correct")) {
		ret = verify_oob(argv[2], argv[3], 1);
	} else {
		fprintf(stderr, "Invalid arguments: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	return ret;
	return 0;
}
