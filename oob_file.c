#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "oob.h"

int file_prepare(struct file *file, int read_out, int write_back)
{
	int read;

	if (read_out && write_back)
		file->fp = fopen(file->name, "rwb+");
	else if (write_back)
		file->fp = fopen(file->name, "wb");
	else
		file->fp = fopen(file->name, "rb");

	if (!file->fp) {
		fprintf(stderr, "oob: error open %s\n", file->name);
		return -errno;
	}

	if (read_out && !file->size) {
		file->size = (uint64_t)lseek(fileno(file->fp), 0, SEEK_END);
	}

	if (file->size <= 0) {
		fprintf(stderr, "oob: cannot determine file size %s\n", file->name);
		return errno;
	}

	file->buf = malloc(file->size);
	if (!file->buf) {
		fprintf(stderr, "oob: insufficient memory\n");
		return errno;
	}

	if (read_out) {
		lseek(fileno(file->fp), 0, SEEK_SET);
		if (fread(file->buf, file->size, 1, file->fp) != 1) {
			fprintf(stderr, "oob: error reading file %s\n", file->name);
			return errno;
		}
	}

	return 0;
}

int file_write(struct file *file)
{
	lseek(fileno(file->fp), 0, SEEK_SET);
	if (fwrite(file->buf, file->size, 1, file->fp) != 1) {
		fprintf(stderr, "oob: error writing file %s\n", file->name);
		return errno;
	}
	return 0;
}

static void checked_fclose(FILE *fp)
{
	if (fp)
		fclose(fp);
}

int file_close_all(struct oob *oob)
{
	checked_fclose(oob->file_data.fp);
	checked_fclose(oob->file_oob.fp);
	checked_fclose(oob->file_data_r.fp);
	checked_fclose(oob->file_oob_r.fp);
}
