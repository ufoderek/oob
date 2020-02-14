GCC=gcc
CFLAGS=-O2 -march=native
INC_COMPAT=-Icompat/include
BCH_M=13
BCH_T=32
BCH_CONST_PARAMS=-DCONFIG_BCH_CONST_PARAMS -DCONFIG_BCH_CONST_M=$(BCH_M) -DCONFIG_BCH_CONST_T=$(BCH_T)

bch.o: bch.c
	$(GCC) $(CFLAGS) $(BCH_CONST_PARAMS) $(INC_COMPAT) $< -c


.PHONY: clean
clean:
	rm -f bch.o
