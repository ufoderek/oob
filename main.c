#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "oob.h"

#define pr_debug printf

int main(int argc, const char *argv[])
{
	int i;
	struct oob *oob;

	oob = oob_init();
	oob_reinit(oob);
	oob_info(oob);

	memset(oob->data, 1, sizeof(oob->data));
	oob_encode(oob);

	//oob_dump_ecc(oob);

	for (i=0;i<(128/8);i++) {
		oob_flip_data(oob, 3+i);
	}

	oob_flip_data(oob, 0);
	//oob_flip_ecc(oob, 6);

	oob_decode(oob);
	if (oob->errcnt == -EBADMSG) {
		fprintf(stderr, "Decode failed\n");
		return -EBADMSG;
	} else if (oob->errcnt == -EINVAL) {
		fprintf(stderr, "Decode with invalid parameters\n");
		return -EINVAL;
	}
	pr_debug("errcnt = %d\n", oob->errcnt);

	oob_correct(oob);
	oob_free(oob);
	return 0;
}
