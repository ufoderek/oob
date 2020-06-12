#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "oob.h"

static void *worker_oob_create(void *arg)
{
	int i;
	struct bch *bch;
	struct worker_data *wd = arg;

	bch = bch_init();
	if (!bch) {
		wd->ret = -ENOMEM;
		return 0;
	}

	for (i = 0; i < wd->sect_cnt; i++) {
		bch_set_buf(bch, wd->partial_data, wd->partial_oob);
		bch_encode(bch);
		wd->partial_data += bch_data_size(bch);
		wd->partial_oob += bch_ecc_size(bch);
	}

	wd->ret = 0;
	return 0;
}

static void *worker_oob_verify(void *arg)
{
	int i;
	struct bch *bch;
	struct worker_data *wd = arg;

	bch = bch_init();
	if (!bch) {
		wd->ret = -ENOMEM;
		return 0;
	}

	for (i = 0; i < wd->sect_cnt; i++) {
		bch_set_buf(bch, wd->partial_data, wd->partial_oob);
		bch_decode(bch);
		if (bch->err_cnt == -EBADMSG)
			wd->ret = -EBADMSG;
		else if (bch->err_cnt > 0)
			wd->bitflips += bch->err_cnt;
		wd->partial_data += bch_data_size(bch);
		wd->partial_oob += bch_ecc_size(bch);
	}

	wd->ret = 0;
	return 0;
}

static void *worker_oob_repair(void *arg)
{
	int i;
	struct bch *bch;
	struct worker_data *wd = arg;

	bch = bch_init();
	if (!bch) {
		wd->ret = -ENOMEM;
		return 0;
	}

	for (i = 0; i < wd->sect_cnt; i++) {
		bch_set_buf(bch, wd->partial_data, wd->partial_oob);
		bch_decode(bch);
		if (bch->err_cnt == -EBADMSG)
			wd->ret = -EBADMSG;
		else if (bch->err_cnt > 0) {
			wd->bitflips += bch->err_cnt;
			bch_correct_data(bch);
		}
		wd->partial_data += bch_data_size(bch);
		wd->partial_oob += bch_ecc_size(bch);
	}

	wd->ret = 0;
	return 0;
}

static void *worker_oob_destroy(void *arg)
{
	int i;
	struct bch *bch;
	struct worker_data *wd = arg;

	bch = bch_init();
	if (!bch) {
		wd->ret = -ENOMEM;
		return 0;
	}

	for (i = 0; i < wd->sect_cnt; i++) {
		bch_set_buf(bch, wd->partial_data, wd->partial_oob);
		bch_broke_data_rand(bch);
		wd->bitflips += bch->err_cnt;
		wd->partial_data += bch_data_size(bch);
		wd->partial_oob += bch_ecc_size(bch);
	}

	wd->ret = 0;
	return 0;
}

static int create_workers(struct oob *oob, void *(wk_thread(void *wd)))
{
	int i;
	pthread_t *th;
	struct worker_data *wd;
	uint64_t nr_subpages = oob->nr_subpages;

	th = malloc(sizeof(*th) * oob->cpus);
	if (!th)
		return -ENOMEM;

	wd = malloc(sizeof(*wd) * oob->cpus);
	if (!wd)
		return -ENOMEM;

	/* Prepare worker data */
	for (i = 0; i < oob->cpus; i++) {
		wd[i].bitflips = 0;
		wd[i].ret = 0;
		wd[i].sect_cnt = nr_subpages / oob->cpus;
		wd[i].partial_data = oob->file.buf +
				     i * wd[i].sect_cnt * oob->subpage_size;
		wd[i].partial_oob = oob->file_oob.buf +
				    i * wd[i].sect_cnt * oob->suboob_size;
	}
	/* Last CPU should do the remaining nr_subpages */
	wd[oob->cpus - 1].sect_cnt += nr_subpages % oob->cpus;

	/* Debug */
	/*
	printf("nr_subpages: %llu\n", nr_subpages);
	for (i = 0; i < oob->cpus; i++) {
		printf("thread_%d: sect_cnt: %llu dbuf: %p-%p obuf: %p-%p\n",
			i,
			wd[i].sect_cnt,
			wd[i].partial_data,
			wd[i].partial_data + wd[i].sect_cnt * oob->subpage_size,
			wd[i].partial_oob,
			wd[i].partial_oob + wd[i].sect_cnt * oob->suboob_size);
	}
	*/

	/* Wakeup all worker threads */
	for (i = 0; i < oob->cpus; i++)
		pthread_create(&th[i], NULL, wk_thread, &wd[i]);

	for (i = 0; i < oob->cpus; i++) {
		pthread_join(th[i], NULL);
		oob->bitflips += wd[i].bitflips;
	}

	for (i = 0; i < oob->cpus; i++) {
		if (wd[i].ret)
			return wd[i].ret;
	}

	return 0;
}

static uint64_t calc_nr_subpages(struct oob *oob)
{
	return (oob->file.size + oob->subpage_size - 1) / oob->subpage_size;
}

int oob_create(struct oob *oob)
{
	int ret;
	uint64_t oob_size;

	/* Prepare data file */
	ret = file_prepare(&oob->file, 0, oob->subpage_size, 1, 0);
	if (ret)
		return ret;

	/* Prepare oob file */
	oob->nr_subpages = calc_nr_subpages(oob);
	oob_size = oob->suboob_size * oob->nr_subpages;
	ret = file_prepare(&oob->file_oob, oob_size, oob->suboob_size, 0, 1);
	if (ret)
		return ret;

	/* Do the work */
	ret = create_workers(oob, worker_oob_create);
	if (ret)
		return ret;

	/* Write oob file */
	ret = file_write_close(&oob->file_oob);
	if (ret)
		return ret;
}

int oob_verify(struct oob *oob)
{
	int ret;
	uint64_t oob_size;

	/* Prepare data file */
	ret = file_prepare(&oob->file, 0, oob->subpage_size, 1, 0);
	if (ret)
		return ret;

	/* Prepare oob file */
	oob->nr_subpages = calc_nr_subpages(oob);
	oob_size = oob->suboob_size * oob->nr_subpages;
	ret = file_prepare(&oob->file_oob, oob_size, oob->suboob_size, 1, 0);
	if (ret)
		return ret;

	/* Do the work */
	ret = create_workers(oob, worker_oob_verify);

	if (ret == -EBADMSG)
		printf("oob: some data is uncorrectable\n");
	if (oob->bitflips)
		printf("oob: %llu correctable bitflips detected\n", oob->bitflips);
	else
		printf("oob: no bitflips detected\n");

	return ret;
}

int oob_repair(struct oob *oob)
{
	int ret;
	uint64_t oob_size;

	/* Prepare data file */
	ret = file_prepare(&oob->file, 0, oob->subpage_size, 1, 1);
	if (ret)
		return ret;

	/* Prepare oob file */
	oob->nr_subpages = calc_nr_subpages(oob);
	oob_size = oob->suboob_size * oob->nr_subpages;
	ret = file_prepare(&oob->file_oob, oob_size, oob->suboob_size, 1, 1);
	if (ret)
		return ret;

	/* Do the work */
	ret = create_workers(oob, worker_oob_repair);
	if (ret == -EBADMSG)
		printf("oob: some data is uncorrectable\n");

	/* Write files */
	ret = file_write_close(&oob->file);
	ret = file_write_close(&oob->file_oob);

	printf("oob: %llu bitflips repaired\n", oob->bitflips);
	return ret;
}

int oob_destroy(struct oob *oob)
{
	int ret;
	uint64_t oob_size;

	/* Prepare data file */
	ret = file_prepare(&oob->file, 0, oob->subpage_size, 1, 1);
	if (ret)
		return ret;

	/* Prepare oob file */
	oob->nr_subpages = calc_nr_subpages(oob);
	oob_size = oob->suboob_size * oob->nr_subpages;
	ret = file_prepare(&oob->file_oob, oob_size, oob->suboob_size, 1, 1);
	if (ret)
		return ret;

	/* Do the work */
	ret = create_workers(oob, worker_oob_destroy);
	if (ret)
		return ret;

	/* Write files */
	ret = file_write_close(&oob->file);
	ret = file_write_close(&oob->file_oob);

	printf("oob: %llu bitflips generated\n", oob->bitflips);
	return ret;
}
