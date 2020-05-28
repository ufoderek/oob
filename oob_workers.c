#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "oob.h"

void *thread_oob_create(void *arg)
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

void *thread_oob_verify(void *arg)
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

void *thread_oob_repair(void *arg)
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

void *thread_oob_break(void *arg)
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

uint64_t calc_sectors(struct bch *bch, uint64_t all_data_size)
{
	return (all_data_size + bch_data_size(bch) - 1) / bch_data_size(bch);
}

uint64_t calc_all_oob_size(struct bch *bch, uint64_t all_data_size)
{
	return bch_ecc_size(bch) * calc_sectors(bch, all_data_size);
}

int create_pthreads(struct oob *oob, void *(wk_thread(void *wd)))
{
	int ret;
	int i;
	struct bch *bch = oob->bch;
	pthread_t *th;
	struct worker_data *wd;
	uint64_t sectors = calc_sectors(bch, oob->fin.size);

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
		wd[i].sect_cnt = sectors / oob->cpus;
		wd[i].partial_data = oob->fin.buf +
				     i * wd[i].sect_cnt * bch_data_size(bch);
		wd[i].partial_oob = oob->fin_oob.buf +
				    i * wd[i].sect_cnt * bch_ecc_size(bch);
	}
	/* Last CPU should do the remaining sectors */
	wd[oob->cpus - 1].sect_cnt += sectors % oob->cpus;

	/* Debug */
	/*
	printf("sectors: %llu\n", sectors);
	for (i = 0; i < oob->cpus; i++) {
		printf("thread_%d: sect_cnt: %llu dbuf: %p-%p obuf: %p-%p\n",
			i,
			wd[i].sect_cnt,
			wd[i].partial_data,
			wd[i].partial_data + wd[i].sect_cnt * bch_data_size(bch),
			wd[i].partial_oob,
			wd[i].partial_oob + wd[i].sect_cnt * bch_ecc_size(bch));
	}
	*/

	/* Let threads working */
	for (i = (oob->cpus - 1) ; i >= 0; i--)
		pthread_create(&th[i], NULL, wk_thread, &wd[i]);

	for (i = 0; i < oob->cpus; i++)
		pthread_join(th[i], NULL);

	for (i = 0; i < oob->cpus; i++)
		oob->bitflips += wd[i].bitflips;

	for (i = 0; i < oob->cpus; i++)
		if (wd[i].ret)
			return ret;

	return 0;
}

int oob_create(struct oob *oob)
{
	int ret;
	struct bch *bch = oob->bch;

	/* Prepare data file */
	ret = file_prepare(&oob->fin, bch_data_size(bch), 1, 0);
	if (ret)
		return ret;

	/* Prepare oob file */
	oob->fin_oob.size = calc_all_oob_size(bch, oob->fin.size);
	ret = file_prepare(&oob->fin_oob, bch_ecc_size(bch), 0, 1);
	if (ret)
		return ret;

	ret = create_pthreads(oob, thread_oob_create);
	if (ret)
		return ret;

	/* Write oob file */
	ret = file_write(&oob->fin_oob);
	if (ret)
		return ret;
}

int oob_verify(struct oob *oob)
{
	int ret;
	struct bch *bch = oob->bch;

	/* Prepare data file */
	ret = file_prepare(&oob->fin, bch_data_size(bch), 1, 0);
	if (ret)
		return ret;

	/* Prepare oob file */
	oob->fin_oob.size = calc_all_oob_size(bch, oob->fin.size);
	ret = file_prepare(&oob->fin_oob, bch_ecc_size(bch), 1, 0);
	if (ret)
		return ret;

	ret = create_pthreads(oob, thread_oob_verify);

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
	struct bch *bch = oob->bch;

	/* Prepare data file */
	ret = file_prepare(&oob->fin, bch_data_size(bch), 1, 1);
	if (ret)
		return ret;

	/* Prepare oob file */
	oob->fin_oob.size = calc_all_oob_size(bch, oob->fin.size);
	ret = file_prepare(&oob->fin_oob, bch_ecc_size(bch), 1, 1);
	if (ret)
		return ret;

	ret = create_pthreads(oob, thread_oob_repair);
	if (ret == -EBADMSG)
		printf("oob: some data is uncorrectable\n");

	/* Write files */
	ret = file_write(&oob->fin);
	if (ret)
		return ret;

	ret = file_write(&oob->fin_oob);
	if (ret)
		return ret;

	printf("oob: %llu bitflips repaired\n", oob->bitflips);
	return 0;
}

int oob_break(struct oob *oob)
{
	int ret;
	struct bch *bch = oob->bch;

	/* Prepare data file */
	ret = file_prepare(&oob->fin, bch_data_size(bch), 1, 1);
	if (ret)
		return ret;

	/* Prepare oob file */
	oob->fin_oob.size = calc_all_oob_size(bch, oob->fin.size);
	ret = file_prepare(&oob->fin_oob, bch_ecc_size(bch), 1, 1);
	if (ret)
		return ret;

	ret = create_pthreads(oob, thread_oob_break);
	if (ret)
		return ret;

	/* Write files */
	ret = file_write(&oob->fin);
	if (ret)
		return ret;

	ret = file_write(&oob->fin_oob);
	if (ret)
		return ret;

	printf("oob: %llu bitflips generated\n", oob->bitflips);
	return 0;
}
