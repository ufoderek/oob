GCC=gcc
CFLAGS=-O2 -march=native
INC_COMPAT=-Icompat/include

bch.o: bch.c
	$(GCC) $(CFLAGS) $(INC_COMPAT) $< -c


.PHONY: clean
clean:
	rm -f bch.o
