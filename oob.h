#ifndef _OOB_H_
#define _OOB_H_

#include <stdio.h>
#include "bch.h"

#define MAX_PATH_LENGTH 4096
#define OOB_FILE_EXT ".oob"
#define FIXED_FILE_EXT ".fixed"

enum oob_mode {
	CREATE,
	VERIFY,
	REPAIR,
	DESTROY,
};

struct file {
	char name[MAX_PATH_LENGTH];
	char name_wb[MAX_PATH_LENGTH];
	FILE *fp;
	FILE *fp_wb;
	uint8_t *buf;
	uint64_t size;
};

struct oob {
	struct bch *bch;
	enum oob_mode mode;
	unsigned long cpus;
	unsigned int subpage_size;
	unsigned int suboob_size;
	uint64_t nr_subpages;
	uint64_t bitflips;

	struct file file;
	struct file file_oob;
};

struct worker_data {
	uint64_t sect_cnt;
	uint8_t *partial_data;
	uint8_t *partial_oob;
	uint64_t bitflips;
	int ret;
};

int file_prepare(struct file *file, uint64_t expected_size, uint64_t unit_size, int read_out, int write_back);
int file_write(struct file *file);
int file_close_all(struct oob *oob);

int oob_create(struct oob *oob);
int oob_verify(struct oob *oob);
int oob_repair(struct oob *oob);
int oob_destroy(struct oob *oob);

#endif /* _OOB_H_ */
