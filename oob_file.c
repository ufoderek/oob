#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "oob.h"

int file_prepare(struct file *file, uint64_t unit_size, int read_out,
		 int write_back, int inplace)
{
	int read;
	uint64_t remain;

	file->inplace = inplace;

	/* open source file */
	if (inplace && read_out && write_back)
		file->fp = fopen(file->name, "rb+"); /* read/write src */
	else if (inplace && write_back)
		file->fp = fopen(file->name, "wbx"); /* write src */
	else if (read_out)
		file->fp = fopen(file->name, "rb"); /* read src */

	if (!file->fp) {
		fprintf(stderr, "oob: error open %s\n", file->name);
		return -errno;
	}

	/* open new source file */
	if (!inplace && write_back) {
		file->fp_new = fopen(file->name_new, "wbx"); /* write new */
		if (!file->fp_new) {
			fprintf(stderr, "oob: error open %s\n", file->name_new);
			return -errno;
		}
	}

	if (read_out && !file->size)
		file->size = (uint64_t)lseek(fileno(file->fp), 0, SEEK_END);

	if (file->size <= 0) {
		fprintf(stderr, "oob: cannot determine file size %s\n", file->name);
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
	memset(file->buf + file->size, 0, remain);
	memset(file->buf, 0, file->size + remain);

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
	FILE *fp;
	char *name;

	if (file->inplace) {
		fp = file->fp;
		name = file->name;
	} else {
		fp = file->fp_new;
		name = file->name_new;
	}

	lseek(fileno(fp), 0, SEEK_SET);
	if (fwrite(file->buf, file->size, 1, fp) != 1) {
		fprintf(stderr, "oob: error writing file %s\n", name);
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
	checked_fclose(oob->file_data.fp_new);
	checked_fclose(oob->file_oob.fp_new);
}
