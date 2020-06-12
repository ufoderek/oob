#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "oob.h"

int file_prepare(struct file *file, uint64_t expected_size, uint64_t unit_size, int read_out, int write_back)
{
	int read;
	uint64_t remain;

	if (read_out && write_back)
		file->fp = fopen(file->name, "rb+");
	else if (write_back)
		file->fp = fopen(file->name, "wbx");
	else
		file->fp = fopen(file->name, "rb");

	if (!file->fp) {
		fprintf(stderr, "oob: error open %s\n", file->name);
		return -errno;
	}

	if (read_out)
		file->size = (uint64_t)lseek(fileno(file->fp), 0, SEEK_END);
	else
		file->size = expected_size;

	if (file->size <= 0) {
		fprintf(stderr, "oob: cannot determine file size %s\n", file->name);
		return errno;
	} else if (expected_size && (file->size != expected_size)) {
		fprintf(stderr, "oob: unexpected file size %s\n", file->name);
		return errno;
	}

	remain = file->size % unit_size;
	if (remain)
		remain = unit_size - remain;

	file->buf = malloc(file->size + remain);
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
		if (remain)
			memset(file->buf + file->size, 0, remain);
	}

	return 0;
}

static void checked_fclose(FILE *fp)
{
	if (fp)
		fclose(fp);
}

int file_write_close(struct file *file)
{
	lseek(fileno(file->fp), 0, SEEK_SET);
	if (fwrite(file->buf, file->size, 1, file->fp) != 1) {
		fprintf(stderr, "oob: error writing file %s\n", file->name);
		checked_fclose(file->fp);
		return errno;
	}
	checked_fclose(file->fp);
	return 0;
}
