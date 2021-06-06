#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "oob.h"

#define pr_debug printf

int main(int argc, const char *argv[])
{
	struct oob *oob;

	oob = oob_init();
	oob_reinit(oob);
	oob_info(oob);

	memset(oob->data, 1, sizeof(oob->data));
	oob_encode(oob);

	oob_dump_ecc(oob);

	oob_flip_data(oob, 3);
	oob_flip_ecc(oob, 6);

	oob_decode(oob);
	if (oob->errcnt == -EBADMSG) {
		fprintf(stderr, "Decode failed\n");
		return 0;
	} else if (oob->errcnt == -EINVAL) {
		fprintf(stderr, "Decode with invalid parameters\n");
		return 0;
	}
	pr_debug("errcnt = %d\n", oob->errcnt);

	oob_correct(oob);
	oob_free(oob);
	return 0;
}
