#ifndef _OOB_H_
#define _OOB_H_

#include <stdio.h>
#include "bch.h"

enum oob_mode {
	CREATE,
	VERIFY,
	REPAIR,
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

struct oob_data {
	enum oob_mode mode;
	struct oob_header header;

	struct file file_data;
	struct file file_oob;
	struct file file_data_r;
	struct file file_oob_r;
};

struct worker_data {
	//int id;
	//int workers;
	//uint64_t start_sect;
	uint64_t sect_cnt;
	//struct oob_data oob;
	uint8_t *partial_data;
	uint8_t *partial_oob;
};

int file_prepare(struct file *file, uint64_t size, int write);
int file_flush(struct file *file);
int file_close_all(struct oob_data *oob);

int oob_create(struct oob_data *oob);

#endif /* _OOB_H_ */
