#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "oob.h"

int main(int argc, const char *argv[])
{
	size_t read_bytes;
	size_t write_bytes;
	uint8_t data[DATA_BYTES];
	uint8_t ecc[ECC_BYTES];

	FILE *fdata;
	FILE *foob;

	fpos_t epos;
	fpos_t dpos;

	srand(time(NULL));

	fdata = fopen(argv[1], "rb+");
	foob = fopen(argv[2], "rb+");

	read_bytes = fread(ecc, 1, sizeof(struct oob_header), foob);  

	while (1) {
		int i;

		read_bytes = fread(ecc, 1, ECC_BYTES, foob);  
		fseek(foob, -read_bytes, SEEK_CUR);
		//printf("read ecc %d bytes\n", read_bytes);
		if (!read_bytes)
			break;

		read_bytes = fread(data, 1, DATA_BYTES, fdata);  
		fseek(fdata, -read_bytes, SEEK_CUR);
		//printf("read data %d bytes\n", read_bytes);
		if (!read_bytes)
			break;

		for (i = 0; i < (ECC_CAP / 8); i++) {
			int x = rand() % (DATA_BYTES + ECC_BYTES);
			
			if (x < DATA_BYTES)
				data[x] = ~data[x];
			else
				ecc[x - DATA_BYTES] = ~ecc[x - DATA_BYTES];
		}
		write_bytes = fwrite(ecc, 1, ECC_BYTES, foob);  
		write_bytes = fwrite(data, 1, read_bytes, fdata);  
	}

	fclose(fdata);
	fclose(foob);
	return 0;
}
