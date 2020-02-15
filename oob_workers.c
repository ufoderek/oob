#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "oob.h"

void *worker_create(struct worker_data *wd)
{
	int i;
	struct bch *bch;

	bch = bch_init();
	if (!bch)
		return 0;

	for (i = 0;i < wd->sect_cnt; i++)
	{
		bch_set_buf(bch, wd->partial_data, wd->partial_oob);
		bch_encode(bch);
		wd->partial_data += bch_data_size(bch);
		wd->partial_oob += bch_ecc_size(bch);
	}

	return 0;
}

uint64_t calc_sectors(struct bch *bch, uint64_t all_data_size)
{
	return (all_data_size + bch_data_size(bch) - 1) / bch_data_size(bch);
}

uint64_t calc_all_oob_size(struct bch *bch, uint64_t all_data_size)
{
	return bch_ecc_size(bch) * calc_sectors(bch, all_data_size);
}

int oob_create(struct oob_data *oob)
{
	int ret;
	struct bch *bch;
	struct worker_data wd;
	uint64_t all_data_size;
	uint64_t all_oob_size;
	uint64_t sectors;

	bch = bch_init();
	if (!bch)
		return -ENOMEM;

	bch_show_info(bch);

	ret = file_prepare(&oob->file_data, 0, 0);
	if (ret)
		return ret;


	all_data_size = oob->file_data.size;
	all_oob_size = calc_all_oob_size(bch, all_data_size);
	sectors = calc_sectors(bch, all_data_size);

	printf("all_data_size: %llu all_oob_size: %llu\n",
		all_data_size, all_oob_size);

	ret = file_prepare(&oob->file_oob, all_oob_size, 1);
	if (ret)
		return ret;

	wd.sect_cnt = sectors;
	wd.partial_data = oob->file_data.buf;
	wd.partial_oob = oob->file_oob.buf;
	worker_create(&wd);

	ret = file_flush(&oob->file_oob);
	if (ret)
		return ret;
}
