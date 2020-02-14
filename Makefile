GCC=gcc
CFLAGS="-march=native -O2"

bch.o: bch.c
	$(GCC) $< -c
