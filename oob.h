#ifndef _OOB_H_
#define _OOB_H_

#include <stdio.h>
#include "bch.h"

#define FILE_EXT_STR ".oob"

enum oob_mode {
	CREATE,
	VERIFY,
	REPAIR,
	BREAK,
};

struct file {
	char *name;
	FILE *fp;
	uint8_t *buf;
	uint64_t size;
};

struct oob_header {
	char prefix[4];		// 4
	uint8_t ecc_cap;	// 1
	uint8_t ecc_size;	// 1
	uint16_t data_size;	// 2
};

struct oob {
	struct bch *bch;
	enum oob_mode mode;
	struct oob_header header;
	unsigned long cpus;
	uint64_t bitflips;

	struct file fin;
	struct file fin_oob;
};

struct worker_data {
	uint64_t sect_cnt;
	uint8_t *partial_data;
	uint8_t *partial_oob;
	uint64_t bitflips;
	int ret;
};

int file_prepare(struct file *file, uint64_t unit_size, int read_out, int write_back);
int file_write(struct file *file);
int file_close_all(struct oob *oob);

int oob_create(struct oob *oob);
int oob_verify(struct oob *oob);
int oob_repair(struct oob *oob);
int oob_break(struct oob *oob);

#endif /* _OOB_H_ */
